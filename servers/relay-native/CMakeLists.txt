qt_add_executable(sb-coms-relay
    src/main.cpp
)

target_link_libraries(sb-coms-relay
    PRIVATE
        Qt6::Core
        Qt6::Network
        sb_coms_relay
)
