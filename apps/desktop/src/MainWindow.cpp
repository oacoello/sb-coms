#include "MainWindow.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      statusLabel_(new QLabel("Idle")),
      pushToTalkButton_(new QPushButton("Hold to Talk"))
{
    setWindowTitle("sb-coms");
    resize(360, 180);

    auto* root = new QWidget(this);
    auto* layout = new QVBoxLayout(root);

    statusLabel_->setAlignment(Qt::AlignCenter);
    pushToTalkButton_->setMinimumHeight(64);

    layout->addWidget(statusLabel_);
    layout->addWidget(pushToTalkButton_);

    setCentralWidget(root);

    connect(pushToTalkButton_, &QPushButton::pressed, this, [this] {
        setTransmitting(true);
    });

    connect(pushToTalkButton_, &QPushButton::released, this, [this] {
        setTransmitting(false);
    });

    connect(&audioLoopback_, &sb_coms::audio::AudioLoopback::errorOccurred, this, [this](const QString& message) {
        statusLabel_->setText(message);
    });
}

void MainWindow::setTransmitting(bool enabled)
{
    if (enabled) {
        audioLoopback_.start();
        statusLabel_->setText(audioLoopback_.isRunning() ? "Transmitting locally..." : "Audio unavailable");
        return;
    }

    audioLoopback_.stop();
    statusLabel_->setText("Idle");
}
