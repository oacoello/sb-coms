#pragma once

#include <QByteArray>

#include <memory>

struct OpusDecoder;
struct OpusEncoder;

namespace sb_coms::codec {

class OpusFrameCodec final
{
public:
    static constexpr int SampleRate = 48000;
    static constexpr int ChannelCount = 1;
    static constexpr int FrameDurationMs = 20;
    static constexpr int FrameSamples = SampleRate / 1000 * FrameDurationMs;
    static constexpr int BytesPerSample = 2;
    static constexpr int PcmFrameBytes = FrameSamples * ChannelCount * BytesPerSample;

    OpusFrameCodec();
    ~OpusFrameCodec();

    OpusFrameCodec(const OpusFrameCodec&) = delete;
    OpusFrameCodec& operator=(const OpusFrameCodec&) = delete;

    [[nodiscard]] QByteArray encodePcmFrame(const QByteArray& pcmFrame) const;
    [[nodiscard]] QByteArray decodePacket(const QByteArray& packet) const;

private:
    struct EncoderDeleter
    {
        void operator()(OpusEncoder* encoder) const;
    };

    struct DecoderDeleter
    {
        void operator()(OpusDecoder* decoder) const;
    };

    std::unique_ptr<OpusEncoder, EncoderDeleter> encoder_;
    std::unique_ptr<OpusDecoder, DecoderDeleter> decoder_;
};

} // namespace sb_coms::codec
