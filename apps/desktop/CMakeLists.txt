set(SB_COMS_DESKTOP_PLATFORM_SOURCES)
set(SB_COMS_DESKTOP_PLATFORM_LIBS)

if(WIN32)
    list(APPEND SB_COMS_DESKTOP_PLATFORM_SOURCES resources/app_icon.rc)
    list(APPEND SB_COMS_DESKTOP_PLATFORM_LIBS dwmapi)
endif()

qt_add_executable(sb-coms-desktop
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.hpp
    src/SpectrumWidget.cpp
    src/SpectrumWidget.hpp
    src/WavRecorder.cpp
    src/WavRecorder.hpp
    resources/resources.qrc
    ${SB_COMS_DESKTOP_PLATFORM_SOURCES}
)

target_link_libraries(sb-coms-desktop
    PRIVATE
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Network
        sb_coms_audio
        sb_coms_codec
        sb_coms_ptt
        sb_coms_protocol
        sb_coms_relay
        ${SB_COMS_DESKTOP_PLATFORM_LIBS}
)

set_target_properties(sb-coms-desktop PROPERTIES
    WIN32_EXECUTABLE TRUE
)
