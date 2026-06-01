#pragma once

#include <cstdint>
#include <vector>

namespace sb_coms::protocol {

struct AudioPacket
{
    std::uint32_t streamId = 0;
    std::uint32_t sequence = 0;
    std::vector<std::uint8_t> payload;
};

} // namespace sb_coms::protocol
