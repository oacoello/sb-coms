include(FetchContent)

FetchContent_Declare(opus
    URL https://downloads.xiph.org/releases/opus/opus-1.5.2.tar.gz
    URL_HASH SHA256=65c1d2f78b9f2fb20082c38cbe47c951ad5839345876e46941612ee87f9a7ce1
)

set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(opus)

if(TARGET opus)
    set(SB_COMS_OPUS_TARGET opus)
elseif(TARGET Opus::opus)
    set(SB_COMS_OPUS_TARGET Opus::opus)
else()
    message(FATAL_ERROR "Opus target not found after FetchContent_MakeAvailable(opus)")
endif()

add_library(sb_coms_codec STATIC
    src/OpusFrameCodec.cpp
    include/sb_coms/codec/OpusFrameCodec.hpp
)

target_include_directories(sb_coms_codec
    PUBLIC
        include
)

target_link_libraries(sb_coms_codec
    PUBLIC
        Qt6::Core
        ${SB_COMS_OPUS_TARGET}
)
