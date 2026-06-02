#include "MainWindow.hpp"

#include <QApplication>
#include <QAudioDevice>
#include <QComboBox>
#include <QCoreApplication>
#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMediaDevices>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QHostAddress>
#include <QIcon>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <algorithm>
#include <utility>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#endif

MainWindow::MainWindow(QString instanceName, QWidget* parent)
    : QMainWindow(parent),
      instanceName_(std::move(instanceName)),
      statusLabel_(new QLabel(instanceName_ + " en espera")),
      activeChannelLabel_(new QLabel("Operaciones")),
      localRelayStatusLabel_(new QLabel("Repetidor local detenido")),
      relayHostInput_(new QLineEdit("127.0.0.1")),
      relayPortInput_(new QLineEdit("50000")),
      channelInput_(new QLineEdit("Operaciones")),
      channelsList_(new QListWidget()),
      inputDeviceSelect_(new QComboBox()),
      outputDeviceSelect_(new QComboBox()),
      connectButton_(new QPushButton("Conectar")),
      disconnectButton_(new QPushButton("Desconectar")),
      startLocalRelayButton_(new QPushButton("Iniciar repetidor")),
      stopLocalRelayButton_(new QPushButton("Detener repetidor")),
      addChannelButton_(new QPushButton("Agregar")),
      removeChannelButton_(new QPushButton("Eliminar")),
      deafenButton_(new QPushButton("Ensordecerse")),
      micLockButton_(new QPushButton("Bloquear micrófono")),
      pushToTalkButton_(new QPushButton("Mantener\npara hablar\n(o pulse ESPACIO)")),
      recordButton_(new QPushButton("Iniciar grabación")),
      renameRecordingButton_(new QPushButton("Cambiar nombre")),
      deleteRecordingButton_(new QPushButton("Eliminar")),
      exportRecordingButton_(new QPushButton("Exportar selección")),
      inputLevelBar_(new QProgressBar()),
      spectrumWidget_(new SpectrumWidget()),
      participantsList_(new QListWidget()),
      recordingsList_(new QListWidget())
{
    setWindowTitle("SB Comms - " + instanceName_);
    setWindowIcon(QIcon(":/icons/icon_comms.png"));
    resize(840, 520);
    loadMaterialSymbols();
    populateChannels();
    populateAudioDevices();
    loadSettings();
    syncChannelSelection();
    applyDarkTheme();
    applyDarkTitleBar();

    auto* root = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(root);

    activeChannelLabel_->setStyleSheet("font-weight: 700; font-size: 18px;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    localRelayStatusLabel_->setAlignment(Qt::AlignCenter);
    pushToTalkButton_->setMinimumHeight(72);
    recordButton_->setMinimumHeight(72);
    pushToTalkButton_->setObjectName("pttButton");
    recordButton_->setObjectName("recordButton");
    deafenButton_->setCheckable(true);
    micLockButton_->setCheckable(true);
    deafenButton_->setObjectName("stateButton");
    micLockButton_->setObjectName("stateButton");
    inputLevelBar_->setRange(0, 100);
    inputLevelBar_->setValue(0);
    inputLevelBar_->setTextVisible(false);
    participantsList_->setMinimumHeight(160);
    channelsList_->setMinimumWidth(180);
    channelsList_->setIconSize(QSize(28, 28));

    auto* relayLayout = new QHBoxLayout();
    relayLayout->addWidget(relayHostInput_);
    relayLayout->addWidget(relayPortInput_);
    relayLayout->addWidget(channelInput_);
    relayLayout->addWidget(connectButton_);
    relayLayout->addWidget(disconnectButton_);

    auto* deviceLayout = new QHBoxLayout();
    deviceLayout->addWidget(inputDeviceSelect_);
    deviceLayout->addWidget(outputDeviceSelect_);

    auto* localRelayLayout = new QHBoxLayout();
    localRelayLayout->addWidget(startLocalRelayButton_);
    localRelayLayout->addWidget(stopLocalRelayButton_);

    auto* leftPanel = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftPanel);
    auto* channelsTitle = new QLabel("CANALES");
    channelsTitle->setStyleSheet("font-weight: 700;");
    leftLayout->addWidget(channelsTitle);
    leftLayout->addWidget(channelsList_);
    auto* channelButtonsLayout = new QHBoxLayout();
    channelButtonsLayout->addWidget(addChannelButton_);
    channelButtonsLayout->addWidget(removeChannelButton_);
    leftLayout->addLayout(channelButtonsLayout);

    auto* participantsGroup = new QGroupBox("Participantes");
    auto* participantsLayout = new QVBoxLayout(participantsGroup);
    participantsLayout->addWidget(participantsList_);

    auto* controlsGroup = new QGroupBox("Controles de radio");
    auto* controlsLayout = new QVBoxLayout(controlsGroup);
    auto* actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->addWidget(pushToTalkButton_);
    actionButtonsLayout->addWidget(recordButton_);
    controlsLayout->addLayout(actionButtonsLayout);
    auto* radioStateButtonsLayout = new QHBoxLayout();
    radioStateButtonsLayout->addWidget(deafenButton_);
    radioStateButtonsLayout->addWidget(micLockButton_);
    controlsLayout->addLayout(radioStateButtonsLayout);
    controlsLayout->addWidget(inputLevelBar_);

    auto* settingsGroup = new QGroupBox("Conexión / Dispositivos");
    auto* settingsLayout = new QVBoxLayout(settingsGroup);
    settingsLayout->addLayout(relayLayout);
    settingsLayout->addLayout(deviceLayout);
    settingsLayout->addWidget(localRelayStatusLabel_);
    settingsLayout->addLayout(localRelayLayout);

    auto* radioPage = new QWidget();
    auto* radioLayout = new QVBoxLayout(radioPage);
    radioLayout->addWidget(spectrumWidget_);
    radioLayout->addWidget(participantsGroup, 1);
    radioLayout->addWidget(controlsGroup);

    auto* aboutGroup = new QGroupBox("Acerca de");
    auto* aboutLayout = new QVBoxLayout(aboutGroup);
    auto* aboutLabel = new QLabel(
        "Copyright © 2026 Universidad de Defensa de Honduras UDH\n"
        "Desarrollo Santa Barbara - V.I.M.M\n\n"
        "Versión " + QCoreApplication::applicationVersion()
    );
    aboutLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    aboutLayout->addWidget(aboutLabel);

    auto* settingsPage = new QWidget();
    auto* settingsPageLayout = new QVBoxLayout(settingsPage);
    settingsPageLayout->addWidget(settingsGroup);
    settingsPageLayout->addStretch();
    settingsPageLayout->addWidget(aboutGroup);

    auto* tabs = new QTabWidget();
    tabs->setIconSize(QSize(30, 30));
    tabs->addTab(radioPage, materialIcon("radio"), "Radio");
    auto* recordingsPage = new QWidget();
    auto* recordingsLayout = new QVBoxLayout(recordingsPage);
    recordingsLayout->addWidget(recordingsList_);
    auto* recordingButtonsLayout = new QHBoxLayout();
    recordingButtonsLayout->addWidget(renameRecordingButton_);
    recordingButtonsLayout->addWidget(deleteRecordingButton_);
    recordingButtonsLayout->addWidget(exportRecordingButton_);
    recordingsLayout->addLayout(recordingButtonsLayout);
    tabs->addTab(recordingsPage, materialIcon("folder"), "Grabaciones");
    tabs->addTab(settingsPage, materialIcon("settings"), "Configuración");

    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(activeChannelLabel_);
    rightLayout->addWidget(statusLabel_);
    rightLayout->addWidget(tabs, 1);

    auto* splitter = new QSplitter();
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    rootLayout->addWidget(splitter);

    setCentralWidget(root);
    disconnectButton_->setEnabled(false);
    stopLocalRelayButton_->setEnabled(false);
    applyButtonIcons();
    applyDarkTheme();

    connect(connectButton_, &QPushButton::clicked, this, &MainWindow::connectToRelay);
    connect(disconnectButton_, &QPushButton::clicked, this, &MainWindow::disconnectFromRelay);
    connect(startLocalRelayButton_, &QPushButton::clicked, this, &MainWindow::startLocalRelay);
    connect(stopLocalRelayButton_, &QPushButton::clicked, this, &MainWindow::stopLocalRelay);
    connect(addChannelButton_, &QPushButton::clicked, this, &MainWindow::addChannel);
    connect(removeChannelButton_, &QPushButton::clicked, this, &MainWindow::removeSelectedChannel);
    connect(deafenButton_, &QPushButton::clicked, this, &MainWindow::toggleDeafen);
    connect(micLockButton_, &QPushButton::clicked, this, &MainWindow::toggleMicLock);
    connect(recordButton_, &QPushButton::clicked, this, &MainWindow::toggleRecording);
    connect(renameRecordingButton_, &QPushButton::clicked, this, &MainWindow::renameSelectedRecording);
    connect(deleteRecordingButton_, &QPushButton::clicked, this, &MainWindow::deleteSelectedRecording);
    connect(exportRecordingButton_, &QPushButton::clicked, this, &MainWindow::exportSelectedRecording);
    connect(&recordingTimer_, &QTimer::timeout, this, &MainWindow::updateRecordingElapsed);
    recordingTimer_.setInterval(1000);
    connect(channelsList_, &QListWidget::currentTextChanged, this, [this](const QString& channel) {
        if (channelInput_->isEnabled() && !channel.isEmpty()) {
            channelInput_->setText(channel);
            activeChannelLabel_->setText(channel);
        }
    });
    connect(channelInput_, &QLineEdit::textChanged, this, [this](const QString& channel) {
        activeChannelLabel_->setText(channel.trimmed().isEmpty() ? "Sin canal" : channel.trimmed());
    });
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

    connect(&audioClient_, &sb_coms::audio::NetworkAudioClient::inputLevelChanged, inputLevelBar_, &QProgressBar::setValue);
    connect(&audioClient_, &sb_coms::audio::NetworkAudioClient::capturedPcm, this, [this](const QByteArray& pcm) {
        spectrumWidget_->updateFromPcm(pcm, true);
        recorder_.writePcm(pcm);
    });
    connect(&audioClient_, &sb_coms::audio::NetworkAudioClient::receivedPcm, this, [this](const QByteArray& pcm) {
        spectrumWidget_->updateFromPcm(pcm, false);
        recorder_.writePcm(pcm);
    });

    connect(&localRelay_, &sb_coms::relay::UdpRelay::statusChanged, this, [this](const QString& message) {
        localRelayStatusLabel_->setText(message);
    });

    connect(&localRelay_, &sb_coms::relay::UdpRelay::errorOccurred, this, [this](const QString& message) {
        localRelayStatusLabel_->setText(message);
    });

    refreshRecordings();
}

void MainWindow::connectToRelay()
{
    QHostAddress host;
    if (!host.setAddress(relayHostInput_->text())) {
        statusLabel_->setText(instanceName_ + ": host del repetidor inválido.");
        return;
    }

    bool portOk = false;
    const int port = relayPortInput_->text().toInt(&portOk);
    if (!portOk || port <= 0 || port > 65535) {
        statusLabel_->setText(instanceName_ + ": puerto del repetidor inválido.");
        return;
    }

    saveSettings();

    audioClient_.setRelayEndpoint(host, static_cast<quint16>(port));
    audioClient_.setChannel(channelInput_->text());
    audioClient_.setDisplayName(instanceName_);
    audioClient_.setAudioDevices(
        inputDeviceSelect_->currentData().toByteArray(),
        outputDeviceSelect_->currentData().toByteArray()
    );
    audioClient_.start();

    activeChannelLabel_->setText(channelInput_->text().trimmed());
    setConnectionInputsEnabled(false);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("connection");
    relayHostInput_->setText(settings.value("relayHost", relayHostInput_->text()).toString());
    relayPortInput_->setText(settings.value("relayPort", relayPortInput_->text()).toString());
    channelInput_->setText(settings.value("channel", channelInput_->text()).toString());
    setSelectedComboValue(inputDeviceSelect_, settings.value("inputDeviceId").toByteArray());
    setSelectedComboValue(outputDeviceSelect_, settings.value("outputDeviceId").toByteArray());
    settings.endGroup();
}

void MainWindow::saveSettings() const
{
    QSettings settings;
    settings.beginGroup("connection");
    settings.setValue("relayHost", relayHostInput_->text());
    settings.setValue("relayPort", relayPortInput_->text());
    settings.setValue("channel", channelInput_->text());
    settings.setValue("inputDeviceId", inputDeviceSelect_->currentData().toByteArray());
    settings.setValue("outputDeviceId", outputDeviceSelect_->currentData().toByteArray());
    settings.endGroup();
}

void MainWindow::loadMaterialSymbols()
{
    const int fontId = QFontDatabase::addApplicationFont(":/fonts/MaterialSymbolsRounded.ttf");
    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (!families.isEmpty()) {
        materialSymbolsFamily_ = families.first();
    }
}

QIcon MainWindow::materialIcon(const QString& name, int size, const QColor& color) const
{
    if (materialSymbolsFamily_.isEmpty()) {
        return {};
    }

    QChar glyph;
    if (name == "radio") {
        glyph = QChar(0xe03e);
    } else if (name == "folder") {
        glyph = QChar(0xe2c7);
    } else if (name == "settings") {
        glyph = QChar(0xe8b8);
    } else if (name == "login") {
        glyph = QChar(0xea77);
    } else if (name == "logout") {
        glyph = QChar(0xe9ba);
    } else if (name == "settings_remote") {
        glyph = QChar(0xe8c7);
    } else if (name == "stop_circle") {
        glyph = QChar(0xef71);
    } else if (name == "mic") {
        glyph = QChar(0xe029);
    } else if (name == "fiber_manual_record") {
        glyph = QChar(0xe061);
    } else if (name == "file_download") {
        glyph = QChar(0xe2c4);
    } else if (name == "add") {
        glyph = QChar(0xe145);
    } else if (name == "delete") {
        glyph = QChar(0xe872);
    } else if (name == "hearing_disabled") {
        glyph = QChar(0xf104);
    } else if (name == "mic_off") {
        glyph = QChar(0xe02b);
    } else if (name == "edit") {
        glyph = QChar(0xe3c9);
    }

    if (glyph.isNull()) {
        return {};
    }

    const int pixelSize = size * 2;
    QPixmap pixmap(pixelSize, pixelSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setPen(color);

    QFont font(materialSymbolsFamily_);
    font.setPixelSize(size);
    font.setWeight(QFont::Normal);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString(glyph));

    return QIcon(pixmap);
}

QIcon MainWindow::buttonIcon(const QString& name) const
{
    QIcon icon;
    icon.addPixmap(materialIcon(name, 40, QColor("#131117")).pixmap(48, 48), QIcon::Normal);
    icon.addPixmap(materialIcon(name, 40, QColor("#ffffff")).pixmap(48, 48), QIcon::Disabled);
    icon.addPixmap(materialIcon(name, 40, QColor("#ffffff")).pixmap(48, 48), QIcon::Normal, QIcon::On);
    return icon;
}

void MainWindow::applyButtonIcons()
{
    connectButton_->setIcon(buttonIcon("login"));
    disconnectButton_->setIcon(buttonIcon("logout"));
    startLocalRelayButton_->setIcon(buttonIcon("settings_remote"));
    stopLocalRelayButton_->setIcon(buttonIcon("stop_circle"));
    pushToTalkButton_->setIcon(materialIcon("mic", 54, QColor("#2dd4bf")));
    recordButton_->setIcon(buttonIcon("fiber_manual_record"));
    deafenButton_->setIcon(buttonIcon("hearing_disabled"));
    micLockButton_->setIcon(buttonIcon("mic_off"));
    renameRecordingButton_->setIcon(buttonIcon("edit"));
    deleteRecordingButton_->setIcon(buttonIcon("delete"));
    exportRecordingButton_->setIcon(buttonIcon("file_download"));
    addChannelButton_->setIcon(buttonIcon("add"));
    removeChannelButton_->setIcon(buttonIcon("delete"));

    const QSize regularIconSize(36, 36);
    for (QPushButton* button : {
        connectButton_, disconnectButton_, startLocalRelayButton_, stopLocalRelayButton_,
        recordButton_, exportRecordingButton_, addChannelButton_, removeChannelButton_,
        deafenButton_, micLockButton_, renameRecordingButton_, deleteRecordingButton_
    }) {
        button->setIconSize(regularIconSize);
    }
    pushToTalkButton_->setIconSize(QSize(58, 58));
}

void MainWindow::populateChannels()
{
    channelsList_->clear();

    QSettings settings;
    const QStringList channels = settings.value("channels/list", QStringList({"Operaciones", "Coordinación"})).toStringList();
    for (const QString& channel : channels) {
        auto* item = new QListWidgetItem(materialIcon("radio", 24), channel);
        channelsList_->addItem(item);
    }
}

void MainWindow::addChannel()
{
    bool accepted = false;
    const QString channel = QInputDialog::getText(
        this,
        "Agregar canal",
        "Nombre del canal:",
        QLineEdit::Normal,
        "",
        &accepted
    ).trimmed();

    if (!accepted || channel.isEmpty()) {
        return;
    }

    if (!channelsList_->findItems(channel, Qt::MatchFixedString).isEmpty()) {
        statusLabel_->setText("Ese canal ya existe.");
        return;
    }

    auto* item = new QListWidgetItem(materialIcon("radio", 24), channel);
    channelsList_->addItem(item);
    channelsList_->setCurrentItem(item);

    QStringList channels;
    for (int i = 0; i < channelsList_->count(); ++i) {
        channels.append(channelsList_->item(i)->text());
    }

    QSettings settings;
    settings.setValue("channels/list", channels);
}

void MainWindow::removeSelectedChannel()
{
    auto* item = channelsList_->currentItem();
    if (item == nullptr || channelsList_->count() <= 1) {
        statusLabel_->setText("Debe quedar al menos un canal.");
        return;
    }

    delete channelsList_->takeItem(channelsList_->row(item));

    QStringList channels;
    for (int i = 0; i < channelsList_->count(); ++i) {
        channels.append(channelsList_->item(i)->text());
    }

    QSettings settings;
    settings.setValue("channels/list", channels);

    if (channelsList_->currentItem() != nullptr) {
        channelInput_->setText(channelsList_->currentItem()->text());
    }
}

void MainWindow::populateAudioDevices()
{
    inputDeviceSelect_->clear();
    outputDeviceSelect_->clear();

    for (const QAudioDevice& device : QMediaDevices::audioInputs()) {
        inputDeviceSelect_->addItem("Micrófono: " + device.description(), device.id());
    }

    for (const QAudioDevice& device : QMediaDevices::audioOutputs()) {
        outputDeviceSelect_->addItem("Salida: " + device.description(), device.id());
    }
}

void MainWindow::applyDarkTheme()
{
    qApp->setStyleSheet(R"(
        QWidget { background: #0d1117; color: #e6edf3; }
        QGroupBox { border: 0; border-radius: 10px; margin-top: 10px; background: #161b22; padding: 8px; }
        QFrame, QTabWidget::pane { border: 1px solid #30363d; border-radius: 10px; background: #161b22; }
        QGroupBox::title { color: #8b949e; subcontrol-origin: margin; left: 8px; padding: 0 4px; font-weight: 700; }
        QLabel { color: #e6edf3; }
        QLineEdit, QComboBox, QListWidget { background: #0d1117; color: #e6edf3; border: 1px solid #30363d; border-radius: 8px; padding: 6px; selection-background-color: #2dd4bf; selection-color: #0d1117; }
        QListWidget::item { margin: 3px; padding: 10px; border-radius: 8px; }
        QListWidget::item:selected { background: #2dd4bf; color: #0d1117; border: 1px solid #5eead4; }
        QPushButton { background: #2dd4bf; color: #0d1117; border: 0; border-radius: 8px; padding: 8px 12px; font-weight: 700; }
        QPushButton:hover { background: #5eead4; }
        QPushButton:disabled { background: #30363d; color: #484f58; }
        QPushButton#pttButton { background: #0d1117; color: #2dd4bf; border: 3px solid #2dd4bf; border-radius: 12px; font-size: 18px; }
        QPushButton#pttButton:pressed { background: #2dd4bf; color: #131117; border: 3px solid #2dd4bf; }
        QPushButton#recordButton { background: #ef4444; color: #131117; border: 0; border-radius: 12px; font-size: 16px; }
        QPushButton#recordButton:checked { background: #da3633; color: #131117; }
        QPushButton#stateButton:checked { background: #30363d; color: #ffffff; }
        QProgressBar { background: #0d1117; border: 0; border-radius: 6px; min-height: 8px; }
        QProgressBar::chunk { background: #22c55e; border-radius: 6px; }
        QTabBar::tab { background: #0d1117; color: #8b949e; border: 1px solid #30363d; border-bottom: 0; border-top-left-radius: 8px; border-top-right-radius: 8px; padding: 9px 16px; margin-right: 4px; }
        QTabBar::tab:selected { background: #161b22; color: #2dd4bf; border-color: #2dd4bf; }
        QSplitter::handle { background: #0d1117; }
    )");
}

void MainWindow::applyDarkTitleBar()
{
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    BOOL enabled = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &enabled, sizeof(enabled)); // DWMWA_USE_IMMERSIVE_DARK_MODE
#endif
}

void MainWindow::syncChannelSelection()
{
    const QList<QListWidgetItem*> matches = channelsList_->findItems(channelInput_->text(), Qt::MatchExactly);
    if (!matches.isEmpty()) {
        channelsList_->setCurrentItem(matches.first());
        return;
    }

    if (channelsList_->count() > 0) {
        channelsList_->setCurrentRow(0);
        channelInput_->setText(channelsList_->currentItem()->text());
        activeChannelLabel_->setText(channelInput_->text());
        return;
    }

    activeChannelLabel_->setText(channelInput_->text());
}

void MainWindow::setSelectedComboValue(QComboBox* comboBox, const QByteArray& value)
{
    const int index = comboBox->findData(value);
    if (index >= 0) {
        comboBox->setCurrentIndex(index);
    }
}

void MainWindow::disconnectFromRelay()
{
    audioClient_.disconnectFromRelay();
    setConnectionInputsEnabled(true);
}

void MainWindow::startLocalRelay()
{
    bool portOk = false;
    const int port = relayPortInput_->text().toInt(&portOk);
    if (!portOk || port <= 0 || port > 65535) {
        localRelayStatusLabel_->setText("Puerto del repetidor local inválido.");
        return;
    }

    if (!localRelay_.start(static_cast<quint16>(port))) {
        return;
    }

    startLocalRelayButton_->setEnabled(false);
    stopLocalRelayButton_->setEnabled(true);
}

void MainWindow::stopLocalRelay()
{
    localRelay_.stop();
    startLocalRelayButton_->setEnabled(true);
    stopLocalRelayButton_->setEnabled(false);
}

void MainWindow::toggleDeafen()
{
    deafened_ = !deafened_;
    audioClient_.setDeafened(deafened_);
    deafenButton_->setChecked(deafened_);
    deafenButton_->setText(deafened_ ? "Ensordecido" : "Ensordecerse");
    statusLabel_->setText(deafened_ ? "Audio de salida silenciado." : "Audio de salida habilitado.");
}

void MainWindow::toggleMicLock()
{
    micLocked_ = !micLocked_;
    micLockButton_->setChecked(micLocked_);
    micLockButton_->setText(micLocked_ ? "Micrófono bloqueado" : "Bloquear micrófono");
    if (micLocked_) {
        setTransmitting(false);
    }
}

void MainWindow::toggleRecording()
{
    if (recorder_.isRecording()) {
        recordingTimer_.stop();
        recorder_.stop();
        recordButton_->setText("Iniciar grabación");
        recordButton_->setChecked(false);
        statusLabel_->setText("Grabación guardada");
        refreshRecordings();
        return;
    }

    QDir().mkpath(recordingsDirectory());
    const QString fileName = "session-" + QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss") + ".wav";
    const QString filePath = QDir(recordingsDirectory()).filePath(fileName);

    if (!recorder_.start(filePath)) {
        statusLabel_->setText("No se pudo iniciar la grabación.");
        return;
    }

    recordingStartedAt_ = QDateTime::currentDateTime();
    recordingTimer_.start();
    recordButton_->setChecked(true);
    recordButton_->setText("Detener grabación\n00:00");
    statusLabel_->setText("Grabando...");
}

void MainWindow::updateRecordingElapsed()
{
    if (!recorder_.isRecording() || !recordingStartedAt_.isValid()) {
        return;
    }

    const qint64 elapsedSeconds = recordingStartedAt_.secsTo(QDateTime::currentDateTime());
    const qint64 minutes = elapsedSeconds / 60;
    const qint64 seconds = elapsedSeconds % 60;
    recordButton_->setText(QString("Detener grabación\n%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::refreshRecordings()
{
    recordingsList_->clear();

    const QDir directory(recordingsDirectory());
    for (const QFileInfo& file : directory.entryInfoList({"*.wav"}, QDir::Files, QDir::Time)) {
        auto* item = new QListWidgetItem(file.fileName() + "  ·  " + wavDurationLabel(file));
        item->setData(Qt::UserRole, file.absoluteFilePath());
        recordingsList_->addItem(item);
    }
}

void MainWindow::renameSelectedRecording()
{
    auto* item = recordingsList_->currentItem();
    if (item == nullptr) {
        statusLabel_->setText("Seleccioná una grabación primero.");
        return;
    }

    const QFileInfo sourceInfo(item->data(Qt::UserRole).toString());
    bool accepted = false;
    QString name = QInputDialog::getText(
        this,
        "Cambiar nombre",
        "Nuevo nombre:",
        QLineEdit::Normal,
        sourceInfo.completeBaseName(),
        &accepted
    ).trimmed();

    if (!accepted || name.isEmpty()) {
        return;
    }

    if (!name.endsWith(".wav", Qt::CaseInsensitive)) {
        name += ".wav";
    }

    const QString destination = sourceInfo.dir().filePath(name);
    if (QFile::exists(destination)) {
        statusLabel_->setText("Ya existe una grabación con ese nombre.");
        return;
    }

    if (!QFile::rename(sourceInfo.absoluteFilePath(), destination)) {
        statusLabel_->setText("No se pudo cambiar el nombre.");
        return;
    }

    refreshRecordings();
}

void MainWindow::deleteSelectedRecording()
{
    auto* item = recordingsList_->currentItem();
    if (item == nullptr) {
        statusLabel_->setText("Seleccioná una grabación primero.");
        return;
    }

    const QString path = item->data(Qt::UserRole).toString();
    if (QMessageBox::question(this, "Eliminar grabación", "¿Eliminar esta grabación?") != QMessageBox::Yes) {
        return;
    }

    if (!QFile::remove(path)) {
        statusLabel_->setText("No se pudo eliminar la grabación.");
        return;
    }

    refreshRecordings();
}

void MainWindow::exportSelectedRecording()
{
    auto* item = recordingsList_->currentItem();
    if (item == nullptr) {
        statusLabel_->setText("Seleccioná una grabación primero.");
        return;
    }

    const QString source = item->data(Qt::UserRole).toString();
    const QString destination = QFileDialog::getSaveFileName(this, "Exportar grabación", item->text(), "Audio WAV (*.wav)");
    if (destination.isEmpty()) {
        return;
    }

    if (QFile::exists(destination)) {
        QFile::remove(destination);
    }

    if (!QFile::copy(source, destination)) {
        statusLabel_->setText("No se pudo exportar la grabación.");
        return;
    }

    statusLabel_->setText("Grabación exportada.");
}

QString MainWindow::recordingsDirectory() const
{
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(basePath).filePath("recordings");
}

QString MainWindow::wavDurationLabel(const QFileInfo& file)
{
    constexpr qint64 headerBytes = 44;
    constexpr qint64 bytesPerSecond = 48000 * 1 * 2;
    const qint64 audioBytes = std::max<qint64>(0, file.size() - headerBytes);
    const qint64 totalSeconds = audioBytes / bytesPerSecond;
    return QString("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QChar('0'))
        .arg(totalSeconds % 60, 2, 10, QChar('0'));
}

void MainWindow::setConnectionInputsEnabled(bool enabled)
{
    relayHostInput_->setEnabled(enabled);
    relayPortInput_->setEnabled(enabled);
    channelInput_->setEnabled(enabled);
    channelsList_->setEnabled(enabled);
    addChannelButton_->setEnabled(enabled);
    removeChannelButton_->setEnabled(enabled);
    inputDeviceSelect_->setEnabled(enabled);
    outputDeviceSelect_->setEnabled(enabled);
    connectButton_->setEnabled(enabled);
    disconnectButton_->setEnabled(!enabled);
}

void MainWindow::setTransmitting(bool enabled)
{
    if (enabled && micLocked_) {
        statusLabel_->setText("Micrófono bloqueado.");
        return;
    }

    if (enabled) {
        audioClient_.startTransmit();
        pushToTalkButton_->setIcon(materialIcon("mic_off", 54, QColor("#131117")));
        pushToTalkButton_->setText("Mantener\npara hablar\n(o suelte ESPACIO)");
        statusLabel_->setText(audioClient_.isTransmitting() ? instanceName_ + " transmitiendo..." : "Audio no disponible");
        return;
    }

    audioClient_.stopTransmit();
    pushToTalkButton_->setIcon(materialIcon("mic", 54, QColor("#2dd4bf")));
    pushToTalkButton_->setText("Mantener\npara hablar\n(o pulse ESPACIO)");
    statusLabel_->setText(instanceName_ + " en espera");
}
