cmake_minimum_required(VERSION 3.24)

project(sb-coms
    VERSION 0.1.0
    DESCRIPTION "Native cross-platform push-to-talk desktop application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia Network)

add_subdirectory(libs/protocol)
add_subdirectory(libs/codec)
add_subdirectory(libs/audio)
add_subdirectory(libs/ptt)
add_subdirectory(apps/desktop)
add_subdirectory(servers/relay-native)
