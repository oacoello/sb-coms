#include "sb_coms/codec/OpusFrameCodec.hpp"

#include <opus.h>

#include <stdexcept>
#include <string>

namespace sb_coms::codec {

namespace {
constexpr int MaxPacketBytes = 4000;
constexpr int VoiceBitrate = 24000;

void ensureOpusOk(int code, const char* operation)
{
    if (code < OPUS_OK) {
        throw std::runtime_error(std::string(operation) + " failed: " + opus_strerror(code));
    }
}
} // namespace

OpusFrameCodec::OpusFrameCodec()
{
    int error = OPUS_OK;

    encoder_.reset(opus_encoder_create(SampleRate, ChannelCount, OPUS_APPLICATION_VOIP, &error));
    ensureOpusOk(error, "opus_encoder_create");

    decoder_.reset(opus_decoder_create(SampleRate, ChannelCount, &error));
    ensureOpusOk(error, "opus_decoder_create");

    ensureOpusOk(opus_encoder_ctl(encoder_.get(), OPUS_SET_BITRATE(VoiceBitrate)), "OPUS_SET_BITRATE");
    ensureOpusOk(opus_encoder_ctl(encoder_.get(), OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE)), "OPUS_SET_SIGNAL");
}

OpusFrameCodec::~OpusFrameCodec() = default;

QByteArray OpusFrameCodec::encodePcmFrame(const QByteArray& pcmFrame) const
{
    if (pcmFrame.size() != PcmFrameBytes) {
        throw std::runtime_error("PCM frame size must be exactly one Opus frame.");
    }

    QByteArray packet(MaxPacketBytes, Qt::Uninitialized);

    const auto* pcm = reinterpret_cast<const opus_int16*>(pcmFrame.constData());
    const int encodedBytes = opus_encode(
        encoder_.get(),
        pcm,
        FrameSamples,
        reinterpret_cast<unsigned char*>(packet.data()),
        packet.size()
    );

    ensureOpusOk(encodedBytes, "opus_encode");
    packet.resize(encodedBytes);

    return packet;
}

QByteArray OpusFrameCodec::decodePacket(const QByteArray& packet) const
{
    QByteArray pcm(PcmFrameBytes, Qt::Uninitialized);

    const int decodedSamples = opus_decode(
        decoder_.get(),
        reinterpret_cast<const unsigned char*>(packet.constData()),
        packet.size(),
        reinterpret_cast<opus_int16*>(pcm.data()),
        FrameSamples,
        0
    );

    ensureOpusOk(decodedSamples, "opus_decode");
    pcm.resize(decodedSamples * ChannelCount * BytesPerSample);

    return pcm;
}

void OpusFrameCodec::EncoderDeleter::operator()(OpusEncoder* encoder) const
{
    opus_encoder_destroy(encoder);
}

void OpusFrameCodec::DecoderDeleter::operator()(OpusDecoder* decoder) const
{
    opus_decoder_destroy(decoder);
}

} // namespace sb_coms::codec
