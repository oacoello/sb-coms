# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-src")
  file(MAKE_DIRECTORY "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-build"
  "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix"
  "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix/tmp"
  "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix/src/opus-populate-stamp"
  "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix/src"
  "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix/src/opus-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix/src/opus-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/oacoe/Documents/sb-coms/build-mingw/_deps/opus-subbuild/opus-populate-prefix/src/opus-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
