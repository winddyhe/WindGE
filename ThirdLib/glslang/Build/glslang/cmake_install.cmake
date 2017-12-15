# Install script for directory: D:/Vulkan/1.0.65.1/glslang/glslang

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
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/glslang/Debug/glslangd.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/glslang/Release/glslang.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/glslang/MinSizeRel/glslang.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Vulkan/1.0.65.1/glslang/Build/glslang/RelWithDebInfo/glslang.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Public" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Public/ShaderLang.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/arrays.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/BaseTypes.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/Common.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/ConstantUnion.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/InfoSink.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/InitializeGlobals.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/intermediate.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/PoolAlloc.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/ResourceLimits.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/revision.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/ShHandle.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/Include" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/Include/Types.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/glslang_tab.cpp.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/gl_types.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/Initialize.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/iomapper.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/LiveTraverser.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/localintermediate.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/ParseHelper.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/reflection.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/RemoveTree.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/Scan.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/ScanContext.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/SymbolTable.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/Versions.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/parseVersions.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/propagateNoContraction.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent/preprocessor" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/preprocessor/PpContext.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glslang/MachineIndependent/preprocessor" TYPE FILE FILES "D:/Vulkan/1.0.65.1/glslang/glslang/MachineIndependent/preprocessor/PpTokens.h")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Vulkan/1.0.65.1/glslang/Build/glslang/OSDependent/Windows/cmake_install.cmake")

endif()

