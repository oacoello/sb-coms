#include "sb_coms/relay/UdpRelay.hpp"

#include <sb_coms/protocol/AudioPacketCodec.hpp>

#include <QDateTime>
#include <QNetworkDatagram>
#include <QSet>

namespace sb_coms::relay {

namespace {
constexpr qint64 ClientTimeoutMs = 15000;
constexpr qint64 SpeakingTimeoutMs = 1000;
} // namespace

UdpRelay::UdpRelay(QObject* parent)
    : QObject(parent)
{
    connect(&socket_, &QUdpSocket::readyRead, this, &UdpRelay::processPendingDatagrams);
    connect(&cleanupTimer_, &QTimer::timeout, this, &UdpRelay::cleanupClients);
    cleanupTimer_.setInterval(500);
}

UdpRelay::~UdpRelay()
{
    stop();
}

bool UdpRelay::start(quint16 port)
{
    if (isRunning()) {
        return true;
    }

    if (!socket_.bind(QHostAddress::AnyIPv4, port)) {
        emit errorOccurred("No se pudo iniciar el repetidor UDP en el puerto " + QString::number(port) + ".");
        return false;
    }

    port_ = port;
    cleanupTimer_.start();
    emit statusChanged("Repetidor local escuchando UDP :" + QString::number(port_));
    return true;
}

void UdpRelay::stop()
{
    cleanupTimer_.stop();
    socket_.close();
    clients_.clear();

    if (port_ != 0) {
        emit statusChanged("Repetidor local detenido");
    }

    port_ = 0;
}

bool UdpRelay::isRunning() const
{
    return socket_.state() == QAbstractSocket::BoundState;
}

quint16 UdpRelay::port() const
{
    return port_;
}

void UdpRelay::processPendingDatagrams()
{
    while (socket_.hasPendingDatagrams()) {
        const QNetworkDatagram datagram = socket_.receiveDatagram();
        const QString senderKey = clientKey(datagram.senderAddress(), static_cast<quint16>(datagram.senderPort()));
        const qint64 now = QDateTime::currentMSecsSinceEpoch();

        sb_coms::protocol::AudioPacket packet;
        QString channel = "default";
        QString name = senderKey;
        if (sb_coms::protocol::AudioPacketCodec::decode(datagram.data(), packet) && !packet.channel.empty()) {
            channel = QString::fromStdString(packet.channel);
            if (!packet.senderName.empty()) {
                name = QString::fromStdString(packet.senderName);
            }
        }

        if (packet.type == sb_coms::protocol::PacketType::Leave) {
            auto existing = clients_.find(senderKey);
            if (existing != clients_.end()) {
                removeClientAndBroadcast(existing);
            }
            continue;
        }

        clients_.insert(senderKey, Client{
            .address = datagram.senderAddress(),
            .port = static_cast<quint16>(datagram.senderPort()),
            .channel = channel,
            .name = name,
            .lastSeenMs = now,
            .lastAudioMs = packet.type == sb_coms::protocol::PacketType::Audio ? now : clients_.value(senderKey).lastAudioMs,
        });

        sendPresence(channel);

        if (packet.type == sb_coms::protocol::PacketType::Hello || packet.type == sb_coms::protocol::PacketType::Heartbeat) {
            continue;
        }

        for (auto it = clients_.begin(); it != clients_.end();) {
            if (it.key() == senderKey) {
                ++it;
                continue;
            }

            if (it->channel != channel) {
                ++it;
                continue;
            }

            if (now - it->lastSeenMs > ClientTimeoutMs) {
                removeClientAndBroadcast(it);
                continue;
            }

            socket_.writeDatagram(datagram.data(), it->address, it->port);
            ++it;
        }
    }
}

void UdpRelay::cleanupClients()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QSet<QString> dirtyChannels;

    for (auto it = clients_.begin(); it != clients_.end();) {
        if (now - it->lastSeenMs > ClientTimeoutMs) {
            dirtyChannels.insert(it->channel);
            removeClientAndBroadcast(it);
            continue;
        }

        if (it->lastAudioMs > 0 && now - it->lastAudioMs > SpeakingTimeoutMs) {
            dirtyChannels.insert(it->channel);
            it->lastAudioMs = 0;
        }

        ++it;
    }

    for (const QString& channel : dirtyChannels) {
        sendPresence(channel);
    }
}

void UdpRelay::sendPresence(const QString& channel)
{
    sb_coms::protocol::AudioPacket presence;
    presence.type = sb_coms::protocol::PacketType::Presence;
    presence.channel = channel.toStdString();
    presence.timestampMs = static_cast<std::uint64_t>(QDateTime::currentMSecsSinceEpoch());

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (const auto& client : clients_) {
        if (client.channel != channel) {
            continue;
        }

        presence.participants.push_back(client.name.toStdString());
        if (now - client.lastAudioMs <= SpeakingTimeoutMs) {
            presence.activeSpeakers.push_back(client.name.toStdString());
        }
    }

    const QByteArray datagram = sb_coms::protocol::AudioPacketCodec::encode(presence);
    for (const auto& client : clients_) {
        if (client.channel == channel) {
            socket_.writeDatagram(datagram, client.address, client.port);
        }
    }
}

void UdpRelay::removeClientAndBroadcast(QHash<QString, Client>::iterator& it)
{
    const QString channel = it->channel;
    it = clients_.erase(it);
    sendPresence(channel);
}

QString UdpRelay::clientKey(const QHostAddress& address, quint16 port)
{
    return address.toString() + ":" + QString::number(port);
}

} // namespace sb_coms::relay
