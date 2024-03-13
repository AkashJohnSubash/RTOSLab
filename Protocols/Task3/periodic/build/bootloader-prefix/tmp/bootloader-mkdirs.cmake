# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/akashjohnsubash/esp/esp-idf/components/bootloader/subproject"
  "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader"
  "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix"
  "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix/tmp"
  "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix/src"
  "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/akashjohnsubash/Uni/RTOSlab_WS2022/Protocols/Task3/periodic/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
