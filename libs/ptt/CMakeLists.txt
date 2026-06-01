add_library(sb_coms_ptt STATIC
    src/PushToTalkState.cpp
    include/sb_coms/ptt/PushToTalkState.hpp
)

target_include_directories(sb_coms_ptt
    PUBLIC
        include
)
