#pragma once

namespace sb_coms::ptt {

class PushToTalkState
{
public:
    void press();
    void release();

    [[nodiscard]] bool isPressed() const;

private:
    bool pressed_ = false;
};

} // namespace sb_coms::ptt
