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
#include <algorithm>
#include <cmath>
#include <cstdint>
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
        emit errorOccurred("No se puede cambiar el repetidor mientras está conectado.");
        return;
    }

    relayHost_ = host;
    relayPort_ = port;
}

void NetworkAudioClient::setChannel(QString channel)
{
    if (started_) {
        emit errorOccurred("No se puede cambiar el canal mientras está conectado.");
        return;
    }

    channel = channel.trimmed();
    if (channel.isEmpty()) {
        emit errorOccurred("El canal no puede estar vacío.");
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

void NetworkAudioClient::setAudioDevices(QByteArray inputDeviceId, QByteArray outputDeviceId)
{
    if (started_) {
        emit errorOccurred("No se pueden cambiar los dispositivos mientras está conectado.");
        return;
    }

    inputDeviceId_ = std::move(inputDeviceId);
    outputDeviceId_ = std::move(outputDeviceId);
}

void NetworkAudioClient::start()
{
    if (started_) {
        return;
    }

    if (!socket_.bind(QHostAddress::AnyIPv4, 0)) {
        emit errorOccurred("No se pudo abrir el socket UDP del cliente.");
        return;
    }

    startPlayback();
    sendHello();
    heartbeatTimer_.start();
    started_ = true;
    emit statusChanged("Conectado a " + relayEndpointLabel() + " / " + channel_);
}

void NetworkAudioClient::disconnectFromRelay()
{
    sendLeave();
    stopTransmit();
    heartbeatTimer_.stop();
    socket_.close();
    sink_.reset();
    output_ = nullptr;
    started_ = false;
    emit participantsChanged({});
    emit statusChanged("Desconectado");
}

void NetworkAudioClient::startTransmit()
{
    start();

    if (transmitting_) {
        return;
    }

    const QAudioDevice inputDevice = selectedInputDevice();
    if (inputDevice.isNull()) {
        emit errorOccurred("No se encontró micrófono.");
        return;
    }

    if (!inputDevice.isFormatSupported(format_)) {
        format_ = inputDevice.preferredFormat();
    }

    source_ = std::make_unique<QAudioSource>(inputDevice, format_);
    input_ = source_->start();

    if (input_ == nullptr) {
        stopTransmit();
        emit errorOccurred("No se pudo iniciar la captura del micrófono.");
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
    emit inputLevelChanged(0);
}

void NetworkAudioClient::setDeafened(bool deafened)
{
    deafened_ = deafened;
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

QAudioDevice NetworkAudioClient::selectedInputDevice() const
{
    for (const QAudioDevice& device : QMediaDevices::audioInputs()) {
        if (!inputDeviceId_.isEmpty() && device.id() == inputDeviceId_) {
            return device;
        }
    }

    return QMediaDevices::defaultAudioInput();
}

QAudioDevice NetworkAudioClient::selectedOutputDevice() const
{
    for (const QAudioDevice& device : QMediaDevices::audioOutputs()) {
        if (!outputDeviceId_.isEmpty() && device.id() == outputDeviceId_) {
            return device;
        }
    }

    return QMediaDevices::defaultAudioOutput();
}

void NetworkAudioClient::startPlayback()
{
    if (sink_) {
        return;
    }

    const QAudioDevice outputDevice = selectedOutputDevice();
    if (outputDevice.isNull()) {
        emit errorOccurred("No se encontró dispositivo de salida.");
        return;
    }

    sink_ = std::make_unique<QAudioSink>(outputDevice, format_);
    output_ = sink_->start();

    if (output_ == nullptr) {
        sink_.reset();
        emit errorOccurred("No se pudo iniciar la reproducción de audio.");
    }
}

void NetworkAudioClient::sendHello()
{
    sendControlPacket(sb_coms::protocol::PacketType::Hello);
    emit statusChanged("Registrado en " + relayEndpointLabel() + " / " + channel_);
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

    emitInputLevel(audio);
    emit capturedPcm(audio);
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

void NetworkAudioClient::emitInputLevel(const QByteArray& pcm)
{
    if (pcm.size() < static_cast<int>(sizeof(std::int16_t))) {
        emit inputLevelChanged(0);
        return;
    }

    const auto* samples = reinterpret_cast<const std::int16_t*>(pcm.constData());
    const int sampleCount = pcm.size() / static_cast<int>(sizeof(std::int16_t));
    double sumSquares = 0.0;

    for (int index = 0; index < sampleCount; ++index) {
        const double normalized = static_cast<double>(samples[index]) / 32768.0;
        sumSquares += normalized * normalized;
    }

    const double rms = std::sqrt(sumSquares / static_cast<double>(sampleCount));
    const int percent = std::clamp(static_cast<int>(rms * 300.0), 0, 100);
    emit inputLevelChanged(percent);
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
            emit receivedPcm(decodedPcm);

            if (output_ != nullptr && !deafened_) {
                output_->write(decodedPcm);
            }
        } catch (const std::exception& error) {
            emit errorOccurred(QString::fromUtf8(error.what()));
        }
    }
}

} // namespace sb_coms::audio
