#include "MainWindow.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QHostAddress>
#include <utility>

MainWindow::MainWindow(QString instanceName, QWidget* parent)
    : QMainWindow(parent),
      instanceName_(std::move(instanceName)),
      statusLabel_(new QLabel(instanceName_ + " idle")),
      relayHostInput_(new QLineEdit("127.0.0.1")),
      relayPortInput_(new QLineEdit("50000")),
      channelInput_(new QLineEdit("dispatch")),
      connectButton_(new QPushButton("Connect")),
      pushToTalkButton_(new QPushButton("Hold to Talk")),
      participantsList_(new QListWidget())
{
    setWindowTitle("sb-coms - " + instanceName_);
    resize(360, 180);

    auto* root = new QWidget(this);
    auto* layout = new QVBoxLayout(root);

    statusLabel_->setAlignment(Qt::AlignCenter);
    pushToTalkButton_->setMinimumHeight(64);
    participantsList_->setMinimumHeight(80);

    auto* relayLayout = new QHBoxLayout();
    relayLayout->addWidget(relayHostInput_);
    relayLayout->addWidget(relayPortInput_);
    relayLayout->addWidget(channelInput_);
    relayLayout->addWidget(connectButton_);

    layout->addLayout(relayLayout);
    layout->addWidget(statusLabel_);
    layout->addWidget(pushToTalkButton_);
    layout->addWidget(participantsList_);

    setCentralWidget(root);

    connect(connectButton_, &QPushButton::clicked, this, &MainWindow::connectToRelay);

    connect(pushToTalkButton_, &QPushButton::pressed, this, [this] {
        setTransmitting(true);
    });

    connect(pushToTalkButton_, &QPushButton::released, this, [this] {
        setTransmitting(false);
    });

    connect(&audioClient_, &sb_coms::audio::NetworkAudioClient::errorOccurred, this, [this](const QString& message) {
        statusLabel_->setText(message);
    });

    connect(&audioClient_, &sb_coms::audio::NetworkAudioClient::statusChanged, this, [this](const QString& message) {
        statusLabel_->setText(message);
    });

    connect(&audioClient_, &sb_coms::audio::NetworkAudioClient::participantsChanged, this, [this](const QStringList& participants) {
        participantsList_->clear();
        participantsList_->addItems(participants);
    });
}

void MainWindow::connectToRelay()
{
    QHostAddress host;
    if (!host.setAddress(relayHostInput_->text())) {
        statusLabel_->setText(instanceName_ + ": invalid relay host.");
        return;
    }

    bool portOk = false;
    const int port = relayPortInput_->text().toInt(&portOk);
    if (!portOk || port <= 0 || port > 65535) {
        statusLabel_->setText(instanceName_ + ": invalid relay port.");
        return;
    }

    audioClient_.setRelayEndpoint(host, static_cast<quint16>(port));
    audioClient_.setChannel(channelInput_->text());
    audioClient_.setDisplayName(instanceName_);
    audioClient_.start();

    relayHostInput_->setEnabled(false);
    relayPortInput_->setEnabled(false);
    channelInput_->setEnabled(false);
    connectButton_->setEnabled(false);
}

void MainWindow::setTransmitting(bool enabled)
{
    if (enabled) {
        audioClient_.startTransmit();
        statusLabel_->setText(audioClient_.isTransmitting() ? instanceName_ + " transmitting..." : "Audio unavailable");
        return;
    }

    audioClient_.stopTransmit();
    statusLabel_->setText(instanceName_ + " idle");
}
