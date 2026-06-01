#include "sb_coms/audio/AudioLoopback.hpp"

#include <QAudioDevice>
#include <QAudioSink>
#include <QAudioSource>
#include <QMediaDevices>

#include <exception>

namespace sb_coms::audio {

AudioLoopback::AudioLoopback(QObject* parent)
    : QObject(parent)
{
    configureFormat();
}

AudioLoopback::~AudioLoopback()
{
    stop();
}

void AudioLoopback::start()
{
    if (running_) {
        return;
    }

    const QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    const QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();

    if (inputDevice.isNull()) {
        emit errorOccurred("No input audio device found.");
        return;
    }

    if (outputDevice.isNull()) {
        emit errorOccurred("No output audio device found.");
        return;
    }

    if (!inputDevice.isFormatSupported(format_)) {
        format_ = inputDevice.preferredFormat();
    }

    source_ = std::make_unique<QAudioSource>(inputDevice, format_);
    sink_ = std::make_unique<QAudioSink>(outputDevice, format_);

    output_ = sink_->start();
    input_ = source_->start();

    if (input_ == nullptr || output_ == nullptr) {
        stop();
        emit errorOccurred("Could not start audio loopback.");
        return;
    }

    connect(input_, &QIODevice::readyRead, this, &AudioLoopback::pumpAudio);
    running_ = true;
}

void AudioLoopback::stop()
{
    if (source_) {
        source_->stop();
    }

    if (sink_) {
        sink_->stop();
    }

    input_ = nullptr;
    output_ = nullptr;
    source_.reset();
    sink_.reset();
    running_ = false;
}

bool AudioLoopback::isRunning() const
{
    return running_;
}

void AudioLoopback::configureFormat()
{
    format_.setSampleRate(48000);
    format_.setChannelCount(1);
    format_.setSampleFormat(QAudioFormat::Int16);
}

void AudioLoopback::pumpAudio()
{
    if (input_ == nullptr || output_ == nullptr) {
        return;
    }

    const QByteArray audio = input_->readAll();
    if (audio.isEmpty()) {
        return;
    }

    pendingPcm_.append(audio);

    while (pendingPcm_.size() >= sb_coms::codec::OpusFrameCodec::PcmFrameBytes) {
        const QByteArray pcmFrame = pendingPcm_.left(sb_coms::codec::OpusFrameCodec::PcmFrameBytes);
        pendingPcm_.remove(0, sb_coms::codec::OpusFrameCodec::PcmFrameBytes);

        try {
            const QByteArray packet = codec_.encodePcmFrame(pcmFrame);
            const QByteArray decodedPcm = codec_.decodePacket(packet);
            output_->write(decodedPcm);
        } catch (const std::exception& error) {
            stop();
            emit errorOccurred(QString::fromUtf8(error.what()));
            return;
        }
    }
}

} // namespace sb_coms::audio
