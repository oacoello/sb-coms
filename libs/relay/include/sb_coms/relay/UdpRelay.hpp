#pragma once

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

namespace sb_coms::relay {

class UdpRelay final : public QObject
{
    Q_OBJECT

public:
    explicit UdpRelay(QObject* parent = nullptr);
    ~UdpRelay() override;

    bool start(quint16 port);
    void stop();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] quint16 port() const;

signals:
    void statusChanged(const QString& message);
    void errorOccurred(const QString& message);

private:
    struct Client
    {
        QHostAddress address;
        quint16 port = 0;
        QString channel;
        QString name;
        qint64 lastSeenMs = 0;
        qint64 lastAudioMs = 0;
    };

    QUdpSocket socket_;
    QTimer cleanupTimer_;
    QHash<QString, Client> clients_;
    quint16 port_ = 0;

    void processPendingDatagrams();
    void cleanupClients();
    void sendPresence(const QString& channel);
    void removeClientAndBroadcast(QHash<QString, Client>::iterator& it);

    [[nodiscard]] static QString clientKey(const QHostAddress& address, quint16 port);
};

} // namespace sb_coms::relay
