#include "sb_coms/ptt/PushToTalkState.hpp"

namespace sb_coms::ptt {

void PushToTalkState::press()
{
    pressed_ = true;
}

void PushToTalkState::release()
{
    pressed_ = false;
}

bool PushToTalkState::isPressed() const
{
    return pressed_;
}

} // namespace sb_coms::ptt
