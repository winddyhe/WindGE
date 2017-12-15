# Install script for directory: D:/Vulkan/1.0.65.1/glslang/External/spirv-tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/glslang")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv-tools" TYPE FILE FILES
    "D:/Vulkan/1.0.65.1/glslang/External/spirv-tools/include/spirv-tools/libspirv.h"
    "D:/Vulkan/1.0.65.1/glslang/External/spirv-tools/include/spirv-tools/libspirv.hpp"
    "D:/Vulkan/1.0.65.1/glslang/External/spirv-tools/include/spirv-tools/optimizer.hpp"
    "D:/Vulkan/1.0.65.1/glslang/External/spirv-tools/include/spirv-tools/linker.hpp"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Vulkan/1.0.65.1/glslang/Build/External/spirv-tools/external/cmake_install.cmake")
  include("D:/Vulkan/1.0.65.1/glslang/Build/External/spirv-tools/source/cmake_install.cmake")
  include("D:/Vulkan/1.0.65.1/glslang/Build/External/spirv-tools/tools/cmake_install.cmake")
  include("D:/Vulkan/1.0.65.1/glslang/Build/External/spirv-tools/test/cmake_install.cmake")
  include("D:/Vulkan/1.0.65.1/glslang/Build/External/spirv-tools/examples/cmake_install.cmake")

endif()

