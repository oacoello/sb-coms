qt_add_executable(sb-coms-desktop
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.hpp
)

target_link_libraries(sb-coms-desktop
    PRIVATE
        Qt6::Widgets
        Qt6::Multimedia
        sb_coms_audio
        sb_coms_codec
        sb_coms_ptt
        sb_coms_protocol
)

set_target_properties(sb-coms-desktop PROPERTIES
    WIN32_EXECUTABLE TRUE
)
