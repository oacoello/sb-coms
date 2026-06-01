#include <QCoreApplication>
#include <QDateTime>
#include <QHash>
#include <QNetworkDatagram>
#include <QSet>
#include <QTimer>
#include <QUdpSocket>

#include <sb_coms/protocol/AudioPacketCodec.hpp>

#include <iostream>

namespace {
constexpr quint16 RelayPort = 50000;
constexpr qint64 ClientTimeoutMs = 15000;
constexpr qint64 SpeakingTimeoutMs = 1000;

struct Client
{
    QHostAddress address;
    quint16 port = 0;
    QString channel;
    QString name;
    qint64 lastSeenMs = 0;
    qint64 lastAudioMs = 0;
};

QString clientKey(const QHostAddress& address, quint16 port)
{
    return address.toString() + ":" + QString::number(port);
}

void sendPresence(QUdpSocket& socket, const QHash<QString, Client>& clients, const QString& channel)
{
    sb_coms::protocol::AudioPacket presence;
    presence.type = sb_coms::protocol::PacketType::Presence;
    presence.channel = channel.toStdString();
    presence.timestampMs = static_cast<std::uint64_t>(QDateTime::currentMSecsSinceEpoch());

    for (const auto& client : clients) {
        if (client.channel == channel) {
            presence.participants.push_back(client.name.toStdString());
            if (QDateTime::currentMSecsSinceEpoch() - client.lastAudioMs <= SpeakingTimeoutMs) {
                presence.activeSpeakers.push_back(client.name.toStdString());
            }
        }
    }

    const QByteArray datagram = sb_coms::protocol::AudioPacketCodec::encode(presence);
    for (const auto& client : clients) {
        if (client.channel == channel) {
            socket.writeDatagram(datagram, client.address, client.port);
        }
    }
}

void removeClientAndBroadcast(QUdpSocket& socket, QHash<QString, Client>& clients, QHash<QString, Client>::iterator& it)
{
    const QString channel = it->channel;
    it = clients.erase(it);
    sendPresence(socket, clients, channel);
}
} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QUdpSocket socket;
    if (!socket.bind(QHostAddress::AnyIPv4, RelayPort)) {
        std::cerr << "Could not bind UDP relay on port " << RelayPort << '\n';
        return 1;
    }

    QHash<QString, Client> clients;
    std::cout << "sb-coms relay listening on udp :" << RelayPort << '\n';

    QObject::connect(&socket, &QUdpSocket::readyRead, [&] {
        while (socket.hasPendingDatagrams()) {
            const QNetworkDatagram datagram = socket.receiveDatagram();
            const QString senderKey = clientKey(datagram.senderAddress(), datagram.senderPort());
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
                auto existing = clients.find(senderKey);
                if (existing != clients.end()) {
                    removeClientAndBroadcast(socket, clients, existing);
                }
                continue;
            }

            clients.insert(senderKey, Client{
                .address = datagram.senderAddress(),
                .port = static_cast<quint16>(datagram.senderPort()),
                .channel = channel,
                .name = name,
                .lastSeenMs = now,
                .lastAudioMs = packet.type == sb_coms::protocol::PacketType::Audio ? now : clients.value(senderKey).lastAudioMs,
            });

            sendPresence(socket, clients, channel);

            if (packet.type == sb_coms::protocol::PacketType::Hello || packet.type == sb_coms::protocol::PacketType::Heartbeat) {
                continue;
            }

            std::cout << "packet from " << senderKey.toStdString()
                      << " channel=" << channel.toStdString()
                      << " bytes=" << datagram.data().size()
                      << " clients=" << clients.size()
                      << '\n';

            for (auto it = clients.begin(); it != clients.end();) {
                if (it.key() == senderKey) {
                    ++it;
                    continue;
                }

                if (it->channel != channel) {
                    ++it;
                    continue;
                }

                if (now - it->lastSeenMs > ClientTimeoutMs) {
                    removeClientAndBroadcast(socket, clients, it);
                    continue;
                }

                socket.writeDatagram(datagram.data(), it->address, it->port);
                ++it;
            }
        }
    });

    QTimer cleanupTimer;
    QObject::connect(&cleanupTimer, &QTimer::timeout, [&] {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        QSet<QString> dirtyChannels;

        for (auto it = clients.begin(); it != clients.end();) {
            if (now - it->lastSeenMs > ClientTimeoutMs) {
                dirtyChannels.insert(it->channel);
                removeClientAndBroadcast(socket, clients, it);
            } else {
                if (it->lastAudioMs > 0 && now - it->lastAudioMs > SpeakingTimeoutMs) {
                    dirtyChannels.insert(it->channel);
                    it->lastAudioMs = 0;
                }
                ++it;
            }
        }

        for (const auto& channel : dirtyChannels) {
            sendPresence(socket, clients, channel);
        }
    });
    cleanupTimer.start(500);

    return app.exec();
}
