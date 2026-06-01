# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "_deps\\opus-build\\CMakeFiles\\opus_autogen.dir\\AutogenUsed.txt"
  "_deps\\opus-build\\CMakeFiles\\opus_autogen.dir\\ParseCache.txt"
  "_deps\\opus-build\\opus_autogen"
  "apps\\desktop\\CMakeFiles\\sb-coms-desktop_autogen.dir\\AutogenUsed.txt"
  "apps\\desktop\\CMakeFiles\\sb-coms-desktop_autogen.dir\\ParseCache.txt"
  "apps\\desktop\\sb-coms-desktop_autogen"
  "libs\\audio\\CMakeFiles\\sb_coms_audio_autogen.dir\\AutogenUsed.txt"
  "libs\\audio\\CMakeFiles\\sb_coms_audio_autogen.dir\\ParseCache.txt"
  "libs\\audio\\sb_coms_audio_autogen"
  "libs\\codec\\CMakeFiles\\sb_coms_codec_autogen.dir\\AutogenUsed.txt"
  "libs\\codec\\CMakeFiles\\sb_coms_codec_autogen.dir\\ParseCache.txt"
  "libs\\codec\\sb_coms_codec_autogen"
  "libs\\protocol\\CMakeFiles\\sb_coms_protocol_autogen.dir\\AutogenUsed.txt"
  "libs\\protocol\\CMakeFiles\\sb_coms_protocol_autogen.dir\\ParseCache.txt"
  "libs\\protocol\\sb_coms_protocol_autogen"
  "libs\\ptt\\CMakeFiles\\sb_coms_ptt_autogen.dir\\AutogenUsed.txt"
  "libs\\ptt\\CMakeFiles\\sb_coms_ptt_autogen.dir\\ParseCache.txt"
  "libs\\ptt\\sb_coms_ptt_autogen"
  "servers\\relay-native\\CMakeFiles\\sb-coms-relay_autogen.dir\\AutogenUsed.txt"
  "servers\\relay-native\\CMakeFiles\\sb-coms-relay_autogen.dir\\ParseCache.txt"
  "servers\\relay-native\\sb-coms-relay_autogen"
  )
endif()
