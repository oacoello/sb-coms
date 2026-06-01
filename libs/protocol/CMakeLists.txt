add_library(sb_coms_protocol STATIC
    src/AudioPacketCodec.cpp
    include/sb_coms/protocol/AudioPacket.hpp
    include/sb_coms/protocol/AudioPacketCodec.hpp
)

target_include_directories(sb_coms_protocol
    PUBLIC
        include
)

target_link_libraries(sb_coms_protocol
    PUBLIC
        Qt6::Core
)
