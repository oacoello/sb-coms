add_library(sb_coms_audio STATIC
    src/AudioDeviceService.cpp
    src/AudioLoopback.cpp
    src/NetworkAudioClient.cpp
    include/sb_coms/audio/AudioDeviceService.hpp
    include/sb_coms/audio/AudioLoopback.hpp
    include/sb_coms/audio/NetworkAudioClient.hpp
)

target_include_directories(sb_coms_audio
    PUBLIC
        include
)

target_link_libraries(sb_coms_audio
    PUBLIC
        Qt6::Multimedia
        Qt6::Network
        sb_coms_codec
        sb_coms_protocol
)
