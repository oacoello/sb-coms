#pragma once

#include <QObject>
#include <QAudioFormat>
#include <QByteArray>
#include <sb_coms/codec/OpusFrameCodec.hpp>
#include <memory>

class QAudioSink;
class QAudioSource;
class QIODevice;

namespace sb_coms::audio {

class AudioLoopback final : public QObject
{
    Q_OBJECT

public:
    explicit AudioLoopback(QObject* parent = nullptr);
    ~AudioLoopback() override;

    void start();
    void stop();

    [[nodiscard]] bool isRunning() const;

signals:
    void errorOccurred(const QString& message);

private:
    QAudioFormat format_;
    std::unique_ptr<QAudioSource> source_;
    std::unique_ptr<QAudioSink> sink_;
    QIODevice* input_ = nullptr;
    QIODevice* output_ = nullptr;
    QByteArray pendingPcm_;
    sb_coms::codec::OpusFrameCodec codec_;
    bool running_ = false;

    void configureFormat();
    void pumpAudio();
};

} // namespace sb_coms::audio
