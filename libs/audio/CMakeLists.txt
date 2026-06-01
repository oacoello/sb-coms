add_library(sb_coms_audio STATIC
    src/AudioDeviceService.cpp
    src/AudioLoopback.cpp
    include/sb_coms/audio/AudioDeviceService.hpp
    include/sb_coms/audio/AudioLoopback.hpp
)

target_include_directories(sb_coms_audio
    PUBLIC
        include
)

target_link_libraries(sb_coms_audio
    PUBLIC
        Qt6::Multimedia
        sb_coms_codec
)
