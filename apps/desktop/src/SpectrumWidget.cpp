#include "SpectrumWidget.hpp"

#include <QPainter>

#include <algorithm>
#include <cmath>

namespace {
constexpr int kHistorySize = 96;
}

SpectrumWidget::SpectrumWidget(QWidget* parent)
    : QWidget(parent),
      waveform_(kHistorySize, 0.0F)
{
    setMinimumHeight(170);
}

void SpectrumWidget::updateFromPcm(const QByteArray& pcm, bool transmitting)
{
    Q_UNUSED(transmitting);

    const float level = computeLevel(pcm);
    for (int i = 0; i < waveform_.size() - 1; ++i) {
        waveform_[i] = waveform_[i + 1] * 0.985F;
    }
    waveform_[waveform_.size() - 1] = level;
    update();
}

void SpectrumWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#05070a"));

    const QRect area = rect().adjusted(18, 18, -18, -18);
    const int centerY = area.center().y();
    const float step = static_cast<float>(area.width()) / static_cast<float>(std::max(1, static_cast<int>(waveform_.size()) - 1));

    painter.setPen(QColor("#30363d"));
    painter.drawLine(area.left(), centerY, area.right(), centerY);

    painter.setPen(QPen(QColor("#f8fafc"), 3, Qt::SolidLine, Qt::RoundCap));
    for (int i = 0; i < waveform_.size(); ++i) {
        const float level = std::clamp(waveform_[i], 0.03F, 1.0F);
        const int height = static_cast<int>(level * area.height() * 0.5F);
        const int x = static_cast<int>(area.left() + step * i);
        painter.drawLine(x, centerY - height, x, centerY + height);
    }

    painter.setPen(QColor("#8b949e"));
    painter.drawText(area.left(), area.top(), "Forma de onda de audio");
}

float SpectrumWidget::computeLevel(const QByteArray& pcm)
{
    if (pcm.size() < 2) {
        return 0.0F;
    }

    const auto* samples = reinterpret_cast<const qint16*>(pcm.constData());
    const int sampleCount = static_cast<int>(pcm.size() / sizeof(qint16));
    double sum = 0.0;

    for (int i = 0; i < sampleCount; ++i) {
        const double sample = static_cast<double>(samples[i]) / 32768.0;
        sum += sample * sample;
    }

    const double rms = std::sqrt(sum / std::max(1, sampleCount));
    return static_cast<float>(std::clamp(rms * 5.0, 0.0, 1.0));
}
