#pragma once

#include "sb_coms/protocol/AudioPacket.hpp"

#include <QByteArray>

namespace sb_coms::protocol {

class AudioPacketCodec final
{
public:
    [[nodiscard]] static QByteArray encode(const AudioPacket& packet);
    [[nodiscard]] static bool decode(const QByteArray& datagram, AudioPacket& packet);
};

} // namespace sb_coms::protocol
