#pragma once

#include <QMainWindow>
#include <QString>

#include <sb_coms/audio/NetworkAudioClient.hpp>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString instanceName, QWidget* parent = nullptr);

private:
    QString instanceName_;
    QLabel* statusLabel_;
    QLineEdit* relayHostInput_;
    QLineEdit* relayPortInput_;
    QLineEdit* channelInput_;
    QPushButton* connectButton_;
    QPushButton* pushToTalkButton_;
    QListWidget* participantsList_;
    sb_coms::audio::NetworkAudioClient audioClient_;

    void connectToRelay();
    void setTransmitting(bool enabled);
};
