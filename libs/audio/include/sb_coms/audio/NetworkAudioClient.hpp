#pragma once

#include <QObject>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>
#include <QUdpSocket>

#include <memory>

#include <sb_coms/codec/OpusFrameCodec.hpp>
#include <sb_coms/protocol/AudioPacket.hpp>

class QAudioSink;
class QAudioSource;
class QIODevice;

namespace sb_coms::audio {

class NetworkAudioClient final : public QObject
{
    Q_OBJECT

public:
    explicit NetworkAudioClient(QObject* parent = nullptr);
    ~NetworkAudioClient() override;

    void setRelayEndpoint(const QHostAddress& host, quint16 port);
    void setChannel(QString channel);
    void setDisplayName(QString displayName);
    void setAudioDevices(QByteArray inputDeviceId, QByteArray outputDeviceId);
    void start();
    void disconnectFromRelay();
    void startTransmit();
    void stopTransmit();
    void setDeafened(bool deafened);

    [[nodiscard]] bool isTransmitting() const;
    [[nodiscard]] QString relayEndpointLabel() const;

signals:
    void errorOccurred(const QString& message);
    void statusChanged(const QString& message);
    void participantsChanged(const QStringList& participants);
    void inputLevelChanged(int percent);
    void capturedPcm(const QByteArray& pcm);
    void receivedPcm(const QByteArray& pcm);

private:
    QAudioFormat format_;
    QUdpSocket socket_;
    QTimer heartbeatTimer_;
    QHostAddress relayHost_ = QHostAddress::LocalHost;
    quint16 relayPort_ = 50000;
    QString channel_ = "dispatch";
    QString displayName_ = "Client";
    QByteArray inputDeviceId_;
    QByteArray outputDeviceId_;

    std::unique_ptr<QAudioSource> source_;
    std::unique_ptr<QAudioSink> sink_;
    QIODevice* input_ = nullptr;
    QIODevice* output_ = nullptr;

    QByteArray pendingPcm_;
    sb_coms::codec::OpusFrameCodec codec_;
    std::uint32_t streamId_ = 0;
    std::uint32_t sequence_ = 0;
    bool started_ = false;
    bool transmitting_ = false;
    bool deafened_ = false;

    void configureFormat();
    [[nodiscard]] QAudioDevice selectedInputDevice() const;
    [[nodiscard]] QAudioDevice selectedOutputDevice() const;
    void startPlayback();
    void sendHello();
    void sendHeartbeat();
    void sendLeave();
    void sendControlPacket(sb_coms::protocol::PacketType type);
    void sendAudioPacket(const QByteArray& packet);
    void pumpMicrophone();
    void emitInputLevel(const QByteArray& pcm);
    void receivePackets();
};

} // namespace sb_coms::audio
