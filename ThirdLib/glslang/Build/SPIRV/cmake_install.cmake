# Install script for directory: D:/Vulkan/1.0.65.1/glslang/SPIRV

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
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/Debug/SPIRVd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/Release/SPIRV.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/MinSizeRel/SPIRV.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/RelWithDebInfo/SPIRV.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/Debug/SPVRemapperd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/Release/SPVRemapper.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/MinSizeRel/SPVRemapper.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/SPIRV/RelWithDebInfo/SPVRemapper.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SPIRV" TYPE FILE FILES
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/bitutils.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/spirv.hpp"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/GLSL.std.450.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/GLSL.ext.KHR.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/GlslangToSpv.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/hex_float.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/Logger.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/SpvBuilder.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/spvIR.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/doc.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/disassemble.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/GLSL.ext.AMD.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/GLSL.ext.NV.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/SPVRemapper.h"
    "D:/Vulkan/1.0.65.1/glslang/SPIRV/doc.h"
    )
endif()

