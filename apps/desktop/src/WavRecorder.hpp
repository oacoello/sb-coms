#pragma once

#include <QFile>
#include <QString>

class WavRecorder final
{
public:
    bool start(const QString& filePath, int sampleRate = 48000, int channelCount = 1);
    void writePcm(const QByteArray& pcm);
    void stop();

    [[nodiscard]] bool isRecording() const;
    [[nodiscard]] QString filePath() const;

private:
    QFile file_;
    QString filePath_;
    int sampleRate_ = 48000;
    int channelCount_ = 1;
    quint32 dataBytes_ = 0;

    void writeHeader(quint32 dataBytes);
};
