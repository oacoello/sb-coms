#pragma once

#include <QColor>
#include <QDateTime>
#include <QFileInfo>
#include <QTimer>
#include <QMainWindow>
#include <QIcon>
#include <QString>

#include "WavRecorder.hpp"
#include "SpectrumWidget.hpp"

#include <sb_coms/audio/NetworkAudioClient.hpp>
#include <sb_coms/relay/UdpRelay.hpp>

class QLabel;
class QComboBox;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QPushButton;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString instanceName, QWidget* parent = nullptr);

private:
    QString instanceName_;
    QString materialSymbolsFamily_;
    QLabel* statusLabel_;
    QLabel* activeChannelLabel_;
    QLabel* localRelayStatusLabel_;
    QLineEdit* relayHostInput_;
    QLineEdit* relayPortInput_;
    QLineEdit* channelInput_;
    QListWidget* channelsList_;
    QComboBox* inputDeviceSelect_;
    QComboBox* outputDeviceSelect_;
    QPushButton* connectButton_;
    QPushButton* disconnectButton_;
    QPushButton* startLocalRelayButton_;
    QPushButton* stopLocalRelayButton_;
    QPushButton* addChannelButton_;
    QPushButton* removeChannelButton_;
    QPushButton* deafenButton_;
    QPushButton* micLockButton_;
    QPushButton* pushToTalkButton_;
    QPushButton* recordButton_;
    QPushButton* renameRecordingButton_;
    QPushButton* deleteRecordingButton_;
    QPushButton* exportRecordingButton_;
    QProgressBar* inputLevelBar_;
    SpectrumWidget* spectrumWidget_;
    QListWidget* participantsList_;
    QListWidget* recordingsList_;
    sb_coms::audio::NetworkAudioClient audioClient_;
    sb_coms::relay::UdpRelay localRelay_;
    WavRecorder recorder_;
    QTimer recordingTimer_;
    QDateTime recordingStartedAt_;
    bool deafened_ = false;
    bool micLocked_ = false;

    void loadSettings();
    void saveSettings() const;
    void loadMaterialSymbols();
    QIcon materialIcon(const QString& name, int size = 30, const QColor& color = QColor("#e6edf3")) const;
    QIcon buttonIcon(const QString& name) const;
    void applyButtonIcons();
    void populateChannels();
    void addChannel();
    void removeSelectedChannel();
    void populateAudioDevices();
    void syncChannelSelection();
    void setSelectedComboValue(QComboBox* comboBox, const QByteArray& value);
    void applyDarkTheme();
    void applyDarkTitleBar();
    void connectToRelay();
    void disconnectFromRelay();
    void startLocalRelay();
    void stopLocalRelay();
    void toggleDeafen();
    void toggleMicLock();
    void toggleRecording();
    void updateRecordingElapsed();
    void refreshRecordings();
    void renameSelectedRecording();
    void deleteSelectedRecording();
    void exportSelectedRecording();
    QString recordingsDirectory() const;
    static QString wavDurationLabel(const QFileInfo& file);
    void setConnectionInputsEnabled(bool enabled);
    void setTransmitting(bool enabled);
};
