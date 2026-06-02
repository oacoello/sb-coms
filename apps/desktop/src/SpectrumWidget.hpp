#pragma once

#include <QVector>
#include <QWidget>

class SpectrumWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumWidget(QWidget* parent = nullptr);

    void updateFromPcm(const QByteArray& pcm, bool transmitting);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<float> waveform_;

    static float computeLevel(const QByteArray& pcm);
};
