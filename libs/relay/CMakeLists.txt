add_library(sb_coms_relay STATIC
    src/UdpRelay.cpp
    include/sb_coms/relay/UdpRelay.hpp
)

target_include_directories(sb_coms_relay
    PUBLIC
        include
)

target_link_libraries(sb_coms_relay
    PUBLIC
        Qt6::Core
        Qt6::Network
        sb_coms_protocol
)
