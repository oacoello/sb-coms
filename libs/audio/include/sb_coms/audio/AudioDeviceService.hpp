#pragma once

#include <string>
#include <vector>

namespace sb_coms::audio {

struct AudioDevice
{
    std::string id;
    std::string name;
};

class AudioDeviceService
{
public:
    [[nodiscard]] std::vector<AudioDevice> inputDevices() const;
    [[nodiscard]] std::vector<AudioDevice> outputDevices() const;
};

} // namespace sb_coms::audio
