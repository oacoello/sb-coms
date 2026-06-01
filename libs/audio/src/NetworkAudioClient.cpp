#include "sb_coms/audio/NetworkAudioClient.hpp"

#include <sb_coms/protocol/AudioPacketCodec.hpp>

#include <QAudioDevice>
#include <QAudioSink>
#include <QAudioSource>
#include <QDateTime>
#include <QMediaDevices>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QSet>

#include <exception>
#include <utility>

namespace sb_coms::audio {

NetworkAudioClient::NetworkAudioClient(QObject* parent)
    : QObject(parent),
      streamId_(QRandomGenerator::global()->generate())
{
    configureFormat();

    connect(&socket_, &QUdpSocket::readyRead, this, &NetworkAudioClient::receivePackets);
    connect(&heartbeatTimer_, &QTimer::timeout, this, &NetworkAudioClient::sendHeartbeat);
    heartbeatTimer_.setInterval(5000);
}

NetworkAudioClient::~NetworkAudioClient()
{
    sendLeave();
    stopTransmit();
}

void NetworkAudioClient::setRelayEndpoint(const QHostAddress& host, quint16 port)
{
    if (started_) {
        emit errorOccurred("Relay endpoint cannot be changed while connected.");
        return;
    }

    relayHost_ = host;
    relayPort_ = port;
}

void NetworkAudioClient::setChannel(QString channel)
{
    if (started_) {
        emit errorOccurred("Channel cannot be changed while connected.");
        return;
    }

    channel = channel.trimmed();
    if (channel.isEmpty()) {
        emit errorOccurred("Channel cannot be empty.");
        return;
    }

    channel_ = std::move(channel);
}

void NetworkAudioClient::setDisplayName(QString displayName)
{
    displayName = displayName.trimmed();
    if (displayName.isEmpty()) {
        return;
    }

    displayName_ = std::move(displayName);
}

void NetworkAudioClient::start()
{
    if (started_) {
        return;
    }

    if (!socket_.bind(QHostAddress::AnyIPv4, 0)) {
        emit errorOccurred("Could not bind UDP client socket.");
        return;
    }

    startPlayback();
    sendHello();
    heartbeatTimer_.start();
    started_ = true;
    emit statusChanged("Connected to " + relayEndpointLabel() + " / " + channel_);
}

void NetworkAudioClient::startTransmit()
{
    start();

    if (transmitting_) {
        return;
    }

    const QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull()) {
        emit errorOccurred("No input audio device found.");
        return;
    }

    if (!inputDevice.isFormatSupported(format_)) {
        format_ = inputDevice.preferredFormat();
    }

    source_ = std::make_unique<QAudioSource>(inputDevice, format_);
    input_ = source_->start();

    if (input_ == nullptr) {
        stopTransmit();
        emit errorOccurred("Could not start microphone capture.");
        return;
    }

    connect(input_, &QIODevice::readyRead, this, &NetworkAudioClient::pumpMicrophone);
    transmitting_ = true;
}

void NetworkAudioClient::stopTransmit()
{
    if (source_) {
        source_->stop();
    }

    input_ = nullptr;
    source_.reset();
    pendingPcm_.clear();
    transmitting_ = false;
}

bool NetworkAudioClient::isTransmitting() const
{
    return transmitting_;
}

QString NetworkAudioClient::relayEndpointLabel() const
{
    return relayHost_.toString() + ":" + QString::number(relayPort_);
}

void NetworkAudioClient::configureFormat()
{
    format_.setSampleRate(sb_coms::codec::OpusFrameCodec::SampleRate);
    format_.setChannelCount(sb_coms::codec::OpusFrameCodec::ChannelCount);
    format_.setSampleFormat(QAudioFormat::Int16);
}

void NetworkAudioClient::startPlayback()
{
    if (sink_) {
        return;
    }

    const QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (outputDevice.isNull()) {
        emit errorOccurred("No output audio device found.");
        return;
    }

    sink_ = std::make_unique<QAudioSink>(outputDevice, format_);
    output_ = sink_->start();

    if (output_ == nullptr) {
        sink_.reset();
        emit errorOccurred("Could not start audio playback.");
    }
}

void NetworkAudioClient::sendHello()
{
    sendControlPacket(sb_coms::protocol::PacketType::Hello);
    emit statusChanged("Registered with " + relayEndpointLabel() + " / " + channel_);
}

void NetworkAudioClient::sendHeartbeat()
{
    sendControlPacket(sb_coms::protocol::PacketType::Heartbeat);
}

void NetworkAudioClient::sendLeave()
{
    if (!started_) {
        return;
    }

    heartbeatTimer_.stop();
    sendControlPacket(sb_coms::protocol::PacketType::Leave);
    started_ = false;
}

void NetworkAudioClient::sendControlPacket(sb_coms::protocol::PacketType type)
{
    sb_coms::protocol::AudioPacket packet;
    packet.type = type;
    packet.streamId = streamId_;
    packet.timestampMs = static_cast<std::uint64_t>(QDateTime::currentMSecsSinceEpoch());
    packet.channel = channel_.toStdString();
    packet.senderName = displayName_.toStdString();

    const QByteArray datagram = sb_coms::protocol::AudioPacketCodec::encode(packet);
    socket_.writeDatagram(datagram, relayHost_, relayPort_);
}

void NetworkAudioClient::sendAudioPacket(const QByteArray& packet)
{
    sb_coms::protocol::AudioPacket audioPacket;
    audioPacket.type = sb_coms::protocol::PacketType::Audio;
    audioPacket.streamId = streamId_;
    audioPacket.sequence = sequence_++;
    audioPacket.timestampMs = static_cast<std::uint64_t>(QDateTime::currentMSecsSinceEpoch());
    audioPacket.channel = channel_.toStdString();
    audioPacket.senderName = displayName_.toStdString();
    audioPacket.payload.assign(packet.begin(), packet.end());

    const QByteArray datagram = sb_coms::protocol::AudioPacketCodec::encode(audioPacket);
    socket_.writeDatagram(datagram, relayHost_, relayPort_);
}

void NetworkAudioClient::pumpMicrophone()
{
    if (input_ == nullptr) {
        return;
    }

    const QByteArray audio = input_->readAll();
    if (audio.isEmpty()) {
        return;
    }

    pendingPcm_.append(audio);

    while (pendingPcm_.size() >= sb_coms::codec::OpusFrameCodec::PcmFrameBytes) {
        const QByteArray pcmFrame = pendingPcm_.left(sb_coms::codec::OpusFrameCodec::PcmFrameBytes);
        pendingPcm_.remove(0, sb_coms::codec::OpusFrameCodec::PcmFrameBytes);

        try {
            sendAudioPacket(codec_.encodePcmFrame(pcmFrame));
        } catch (const std::exception& error) {
            stopTransmit();
            emit errorOccurred(QString::fromUtf8(error.what()));
            return;
        }
    }
}

void NetworkAudioClient::receivePackets()
{
    while (socket_.hasPendingDatagrams()) {
        const QNetworkDatagram datagram = socket_.receiveDatagram();

        sb_coms::protocol::AudioPacket packet;
        if (!sb_coms::protocol::AudioPacketCodec::decode(datagram.data(), packet)) {
            continue;
        }

        if (QString::fromStdString(packet.channel) != channel_) {
            continue;
        }

        if (packet.type == sb_coms::protocol::PacketType::Presence) {
            QStringList participants;
            QSet<QString> activeSpeakers;
            for (const auto& activeSpeaker : packet.activeSpeakers) {
                activeSpeakers.insert(QString::fromStdString(activeSpeaker));
            }

            for (const auto& participant : packet.participants) {
                const QString name = QString::fromStdString(participant);
                participants.push_back(activeSpeakers.contains(name) ? name + " 🔊 speaking" : name);
            }
            emit participantsChanged(participants);
            continue;
        }

        if (packet.type != sb_coms::protocol::PacketType::Audio || packet.streamId == streamId_) {
            continue;
        }

        try {
            const QByteArray opusPacket(reinterpret_cast<const char*>(packet.payload.data()), static_cast<qsizetype>(packet.payload.size()));
            const QByteArray decodedPcm = codec_.decodePacket(opusPacket);

            if (output_ != nullptr) {
                output_->write(decodedPcm);
            }
        } catch (const std::exception& error) {
            emit errorOccurred(QString::fromUtf8(error.what()));
        }
    }
}

} // namespace sb_coms::audio
