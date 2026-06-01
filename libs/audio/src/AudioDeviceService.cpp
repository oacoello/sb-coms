#include "sb_coms/audio/AudioDeviceService.hpp"

namespace sb_coms::audio {

std::vector<AudioDevice> AudioDeviceService::inputDevices() const
{
    return {};
}

std::vector<AudioDevice> AudioDeviceService::outputDevices() const
{
    return {};
}

} // namespace sb_coms::audio
