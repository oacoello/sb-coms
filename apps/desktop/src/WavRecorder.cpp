#include "WavRecorder.hpp"

#include <QDataStream>

bool WavRecorder::start(const QString& filePath, int sampleRate, int channelCount)
{
    stop();

    file_.setFileName(filePath);
    if (!file_.open(QIODevice::WriteOnly)) {
        return false;
    }

    filePath_ = filePath;
    sampleRate_ = sampleRate;
    channelCount_ = channelCount;
    dataBytes_ = 0;
    writeHeader(0);
    return true;
}

void WavRecorder::writePcm(const QByteArray& pcm)
{
    if (!isRecording() || pcm.isEmpty()) {
        return;
    }

    const qint64 written = file_.write(pcm);
    if (written > 0) {
        dataBytes_ += static_cast<quint32>(written);
    }
}

void WavRecorder::stop()
{
    if (!file_.isOpen()) {
        return;
    }

    file_.seek(0);
    writeHeader(dataBytes_);
    file_.close();
}

bool WavRecorder::isRecording() const
{
    return file_.isOpen();
}

QString WavRecorder::filePath() const
{
    return filePath_;
}

void WavRecorder::writeHeader(quint32 dataBytes)
{
    constexpr quint16 bitsPerSample = 16;
    const quint32 byteRate = static_cast<quint32>(sampleRate_ * channelCount_ * bitsPerSample / 8);
    const quint16 blockAlign = static_cast<quint16>(channelCount_ * bitsPerSample / 8);
    const quint32 riffSize = 36 + dataBytes;

    QDataStream stream(&file_);
    stream.setByteOrder(QDataStream::LittleEndian);

    file_.write("RIFF", 4);
    stream << riffSize;
    file_.write("WAVE", 4);
    file_.write("fmt ", 4);
    stream << quint32(16);
    stream << quint16(1);
    stream << quint16(channelCount_);
    stream << quint32(sampleRate_);
    stream << byteRate;
    stream << blockAlign;
    stream << bitsPerSample;
    file_.write("data", 4);
    stream << dataBytes;
}
