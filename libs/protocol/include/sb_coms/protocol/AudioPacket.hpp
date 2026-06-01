#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sb_coms::protocol {

enum class PacketType : std::uint8_t
{
    Hello = 1,
    Audio = 2,
    Presence = 3,
    Heartbeat = 4,
    Leave = 5,
};

struct AudioPacket
{
    PacketType type = PacketType::Audio;
    std::uint32_t streamId = 0;
    std::uint32_t sequence = 0;
    std::uint64_t timestampMs = 0;
    std::string channel;
    std::string senderName;
    std::vector<std::string> participants;
    std::vector<std::string> activeSpeakers;
    std::vector<std::uint8_t> payload;
};

} // namespace sb_coms::protocol
