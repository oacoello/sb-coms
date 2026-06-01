#pragma once

#include <QMainWindow>

#include <sb_coms/audio/AudioLoopback.hpp>

class QLabel;
class QPushButton;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QLabel* statusLabel_;
    QPushButton* pushToTalkButton_;
    sb_coms::audio::AudioLoopback audioLoopback_;

    void setTransmitting(bool enabled);
};
