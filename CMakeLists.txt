###########################################################################
#
#  Library: MSVTK
#
#  Copyright (c) Kitware Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

cmake_minimum_required(VERSION 2.8.2)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
project(MSVTK)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Library mode: SHARED (default) or STATIC
#
set(MSVTK_LIBRARY_MODE "SHARED")

option(BUILD_SHARED_LIBS "Build MSVTK with shared libraries." ON)
set(MSVTK_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
mark_as_advanced(BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Superbuild Option - Enabled by default
#
option(MSVTK_SUPERBUILD "Build MSVTK and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(MSVTK_SUPERBUILD)

#-----------------------------------------------------------------------------
# Output directories.
#
foreach(type LIBRARY RUNTIME ARCHIVE)
  # Make sure the directory exists
  if(DEFINED MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY
     AND NOT EXISTS ${MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY})
    message(FATAL_ERROR "MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY is set to a non-existing directory [${MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY}]")
  endif()

  if(MSVTK_SUPERBUILD)
    set(output_dir ${MSVTK_BINARY_DIR}/bin)
    if(NOT DEFINED MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY)
      set(MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY ${MSVTK_BINARY_DIR}/MSVTK-build/bin)
    endif()
  else()
    if(NOT DEFINED MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY)
      set(output_dir ${MSVTK_BINARY_DIR}/bin)
    else()
      set(output_dir ${MSVTK_CMAKE_${type}_OUTPUT_DIRECTORY})
    endif()
  endif()
  set(CMAKE_${type}_OUTPUT_DIRECTORY ${output_dir} CACHE INTERNAL "Single output directory for building all libraries.")
endforeach()

#-----------------------------------------------------------------------------
# MSVTK version number.  An even minor number corresponds to releases.
#
set(MSVTK_VERSION_MAJOR 0)
set(MSVTK_VERSION_MINOR 1)
set(MSVTK_VERSION_PATCH 0)
set(MSVTK_VERSION
    "${MSVTK_VERSION_MAJOR}.${MSVTK_VERSION_MINOR}.${MSVTK_VERSION_PATCH}")

# TODO Rename MSVTK_VERSION into MSVTK_VERSION_STR  -  MSVTK_VERSION should be hexadecimal one

# Append the library version information to the library target
# properties.  A parent project may set its own properties and/or may
# block this.
if(NOT MSVTK_NO_LIBRARY_VERSION)
  set(MSVTK_LIBRARY_PROPERTIES ${MSVTK_LIBRARY_PROPERTIES}
    VERSION "${MSVTK_VERSION}"
    SOVERSION "${MSVTK_MAJOR_VERSION}.${MSVTK_MINOR_VERSION}"
    )
endif()

#-----------------------------------------------------------------------------
# Install directories, used for install rules.
#
# TODO Rename with SUBDIR suffix
if(NOT MSVTK_INSTALL_BIN_DIR)
  set(MSVTK_INSTALL_BIN_DIR "bin")
endif()
if(NOT MSVTK_INSTALL_LIB_DIR)
  set(MSVTK_INSTALL_LIB_DIR "lib/MSVTK-${MSVTK_MAJOR_VERSION}.${MSVTK_MINOR_VERSION}")
endif()
if(NOT MSVTK_INSTALL_INCLUDE_DIR)
  set(MSVTK_INSTALL_INCLUDE_DIR "include/MSVTK-${MSVTK_MAJOR_VERSION}.${MSVTK_MINOR_VERSION}")
endif()
if(NOT MSVTK_INSTALL_DOC_DIR)
  set(MSVTK_INSTALL_DOC_DIR "doc")
endif()

#-----------------------------------------------------------------------------
# Update CMake module path
# Note: FindXXX.cmake script specific to utility should be copied into Utilities/CMake
#
set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/CMake"
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  ${CMAKE_MODULE_PATH})

#-----------------------------------------------------------------------------
# Clear MSVTK_BASE_INCLUDE_DIRS, MSVTK_BASE_LIBRARIES and MSVTK_WRAPPED_LIBRARIES_PYTHONQT
#
#set(MSVTK_BASE_LIBRARIES CACHE INTERNAL "MSVTK base libraries" FORCE)
#set(MSVTK_BASE_INCLUDE_DIRS CACHE INTERNAL "MSVTK includes" FORCE)
# TODO Shouldn't be required .. was added to CTK before we rely on exported target

# Variable use in MSVTKConfig.cmake.in
set(MSVTK_LIBRARIES CACHE INTERNAL "MSVTK libraries" FORCE)
set(MSVTK_PLUGIN_LIBRARIES CACHE INTERNAL "MSVTK plugins" FORCE)

# Used by MSVTKGenerateMSVTKConfig.cmake and also used to reference script from other scripts
set(MSVTK_CMAKE_DIR ${MSVTK_SOURCE_DIR}/CMake)
set(MSVTK_CMAKE_UTILITIES_DIR ${MSVTK_SOURCE_DIR}/Utilities/CMake)

#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#
#include(CMake/MSVTKMacroParseArguments.cmake)
#include(CMake/MSVTKMacroSetPaths.cmake)
#include(CMake/MSVTKMacroListFilter.cmake)
#include(CMake/MSVTKMacroBuildLib.cmake)
#include(CMake/MSVTKMacroBuildLibWrapper.cmake)
#include(CMake/MSVTKMacroBuildPlugin.cmake)
#include(CMake/MSVTKMacroBuildApp.cmake)
#include(CMake/MSVTKMacroCompilePythonScript.cmake)
#include(CMake/MSVTKMacroWrapPythonQt.cmake)
#include(CMake/MSVTKMacroSetupQt.cmake)
#include(CMake/MSVTKMacroTargetLibraries.cmake) # Import multiple macros
#include(CMake/MSVTKFunctionExtractOptionNameAndValue.cmake)
#include(CMake/MSVTKMacroValidateBuildOptions.cmake)
#include(CMake/MSVTKMacroAddMSVTKLibraryOptions.cmake)
#include(CMake/MSVTKFunctionGenerateDGraphInput.cmake)
#include(CMake/MSVTKFunctionGenerateProjectXml.cmake)
#include(CMake/MSVTKFunctionGeneratePluginManifest.cmake)
#include(CMake/MSVTKMacroGeneratePluginResourceFile.cmake)
#include(CMake/MSVTKFunctionCheckCompilerFlags.cmake)
#include(CMake/MSVTKFunctionGetIncludeDirs.cmake)
#include(CMake/MSVTKFunctionGetLibraryDirs.cmake)
#include(CMake/MSVTKFunctionGetGccVersion.cmake)

#-----------------------------------------------------------------------------
# Testing
#
option(BUILD_TESTING "Test the project" ON)
if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  set(CPP_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  # DashBoard
  configure_file(
    ${MSVTK_SOURCE_DIR}/CTestConfig.cmake
    ${MSVTK_BINARY_DIR}/CTestConfig.cmake
    COPYONLY)

  # Setup file for setting custom ctest vars
  CONFIGURE_FILE(
    CMake/CTestCustom.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
    @ONLY
    )

  # Configuration for the CMake-generated test driver
  set(CMAKE_TESTDRIVER_EXTRA_INCLUDES "
#include <stdexcept>
#if defined(WIN32)
#  pragma warning(disable : 4996)
#endif")
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "
  try
    {")
  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "
    }
  catch( std::exception & excp )
    {
    fprintf(stderr, \"Test driver caught exception: [%s]\\n\", excp.what());
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    printf(\"Exception caught by the test driver\\n\");
    return EXIT_FAILURE;
    }")
endif()

#-----------------------------------------------------------------------------
# Documentation
#
option(DOCUMENTATION_TARGET_IN_ALL "Include the custom target for building documentation in 'all'" OFF)
mark_as_advanced(DOCUMENTATION_TARGET_IN_ALL)

set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 CACHE PATH "Where documentation archives should be stored")
mark_as_advanced(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY)

#-----------------------------------------------------------------------------
# Attempt to discover Doxygen so that DOXYGEN_EXECUTABLE is set to an appropriate default value
#
FIND_PACKAGE(Doxygen QUIET)

#-----------------------------------------------------------------------------
# Coverage
#
option(WITH_COVERAGE "Enable/Disable coverage" OFF)

#-----------------------------------------------------------------------------
# Set coverage Flags
#
if(WITH_COVERAGE)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(coverage_flags "-g -fprofile-arcs -ftest-coverage -O0 -DNDEBUG")
    set(COVERAGE_CXX_FLAGS ${coverage_flags})
    set(COVERAGE_C_FLAGS ${coverage_flags})
  endif()
endif()

#-----------------------------------------------------------------------------
# Additional CXX/C Flags
#
set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags")
mark_as_advanced(ADDITIONAL_C_FLAGS)
set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags")
mark_as_advanced(ADDITIONAL_CXX_FLAGS)

#-----------------------------------------------------------------------------
# Set symbol visibility Flags
#
# MinGW does not export all symbols automatically, so no need to set flags
#if(CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
#  set(VISIBILITY_CXX_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")
#endif()

#-----------------------------------------------------------------------------
# MSVTK C/CXX Flags
#
set(MSVTK_C_FLAGS "${CMAKE_C_FLAGS_INIT} ${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}")
set(MSVTK_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} ${VISIBILITY_CXX_FLAGS} ${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")

  #MSVTKFunctionCheckCompilerFlags("-fdiagnostics-show-option" cflags)
  #MSVTKFunctionCheckCompilerFlags("-Wl,--no-undefined" cflags)

  #MSVTKFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)
  # With older version of gcc supporting the flag -fstack-protector-all, an extra dependency to libssp.so
  # is introduced. If gcc is smaller than 4.4.0 and the build type is Release let's not include the flag.
  # Doing so should allow to build package made for distribution using older linux distro.
  #if(${GCC_VERSION} VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ${GCC_VERSION} VERSION_LESS "4.4.0"))
  #  MSVTKFunctionCheckCompilerFlags("-fstack-protector-all" cflags)
  #endif()
  if(MINGW)
    # suppress warnings about auto imported symbols
    set(MSVTK_CXX_FLAGS "-Wl,--enable-auto-import ${MSVTK_CXX_FLAGS}")
  endif()

  set(MSVTK_C_FLAGS "${cflags} ${MSVTK_C_FLAGS}")
#   set(MSVTK_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wold-style-cast -Wstrict-null-sentinel -Wsign-promo ${MSVTK_CXX_FLAGS}")
  set(MSVTK_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wstrict-null-sentinel -Wsign-promo ${MSVTK_CXX_FLAGS}")
endif()

if(MSVC)
  set(msvc_suppressed_warnings
    "/wd4290" # C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
  )
  set(MSVTK_CXX_FLAGS "${MSVTK_CXX_FLAGS} ${msvc_suppressed_warnings}")
endif()

add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)

#-----------------------------------------------------------------------------
# QT
#
include(CMake/msvMacroSetupQt.cmake)
msvMacroSetupQt()

# Update MSVTK_BASE_LIBRARIES with QT libraries
#if(QT4_FOUND)
#  set(MSVTK_BASE_LIBRARIES ${MSVTK_BASE_LIBRARIES} ${QT_LIBRARIES} CACHE INTERNAL "MSVTK base libraries" FORCE)
#endif()

#-----------------------------------------------------------------------------
# MSVTK Libraries - Use ON or OFF to indicate if the library should be built by default
#
#set(MSVTK_LIBS
#  Core:ON
#  Widgets:ON
#  )

#-----------------------------------------------------------------------------
# MSVTK Plugins - Use ON or OFF to indicate if the plugin should be built by default
#
#set(MSVTK_PLUGINS
#  )

#-----------------------------------------------------------------------------
# MSVTK Applications - Use ON or OFF to indicate if the application should be built by default
#
#set(MSVTK_APPLICATIONS
#  )

#-----------------------------------------------------------------------------
# To make options show up in both MSVTK-SuperBuild and MSVTK regular build, let's add them
# before the SuperBuild script is included
#

# Let's mark as advanced some default properties
mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(DART_TESTING_TIMEOUT)

# KWStyle
option(MSVTK_USE_KWSTYLE     "Enable sourcecode-based style tests." OFF)
#mark_as_advanced(MSVTK_USE_KWSTYLE)

#---------------------------------------------------------------------------
# Will contain a list of sub-directory without option ON or OFF
#
set(MSVTK_PLUGINS_SUBDIRS )
set(MSVTK_APPLICATIONS_SUBDIRS )

option(MSVTK_APP_ECG "ECG application to demonstrate vtkPipeline architecture and vtkButtons" ON)
option(MSVTK_APP_VTKButtons "vtkButtons application to demonstrate vtkButtons" ON)
option(MSVTK_APP_ButtonClusters "vtkButtons application to demonstrate vtkButtons clustering" ON)

list(APPEND MSVTK_APPLICATIONS_SUBDIRS ECG)
list(APPEND MSVTK_APPLICATIONS_SUBDIRS VTKButtons)
list(APPEND MSVTK_APPLICATIONS_SUBDIRS ButtonClusters)

option(MSVTK_APP_FFS "FFS application to demonstrate fluid flow simulation capabilities in msvtk" OFF)
if(MSVTK_APP_FFS)
  list(APPEND MSVTK_APPLICATIONS_SUBDIRS FFS)
  set(ENABLE_IBAMR ON)
endif()

set(MSVTK_MAIN_PROJECT_APPLICATION_NAME ECG)

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "error: Install Git and try to re-configure")
endif()

#-----------------------------------------------------------------------------
# Superbuild script
#
if(MSVTK_SUPERBUILD)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CMake/SuperBuild")
  return()
endif()

#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------
find_package(VTK REQUIRED)
#if(NOT VTK_USE_QVTK)
#  message(FATAL_ERROR "error: VTK was not configured to use QT, you probably need "
#                      "to recompile it with VTK_USE_GUISUPPORT ON, VTK_USE_QVTK ON, "
#                      "DESIRED_QT_VERSION 4 and QT_QMAKE_EXECUTABLE set appropriatly. "
#                      "Note that Qt >= ${Slicer_REQUIRED_QT_VERSION} is *required*")
#endif()
include(${VTK_USE_FILE})

#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------
find_package(CTK REQUIRED)
include(${CTK_USE_FILE})

#-----------------------------------------------------------------------------
# IBAMR
#-----------------------------------------------------------------------------
if(ENABLE_IBAMR)
  find_package(IBAMR REQUIRED)
  include(${IBAMR_USE_FILE})
endif()  

#-----------------------------------------------------------------------------
# MSVTK_SUPERBUILD_BINARY_DIR

# If MSVTK_SUPERBUILD_BINARY_DIR isn't defined, it means MSVTK is *NOT* build using Superbuild.
# In that specific case, MSVTK_SUPERBUILD_BINARY_DIR should default to MSVTK_BINARY_DIR
if(NOT DEFINED MSVTK_SUPERBUILD_BINARY_DIR)
  set(MSVTK_SUPERBUILD_BINARY_DIR ${MSVTK_BINARY_DIR})
endif()

#-----------------------------------------------------------------------------
# Get operating system, architecture and bitness
#-----------------------------------------------------------------------------
include(CMake/msvMacroGetOperatingSystemArchitectureBitness.cmake)
msvMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX MSVTK)

#-----------------------------------------------------------------------------
# Configure files with settings
#

#CONFIGURE_FILE(${MSVTK_SOURCE_DIR}/UseMSVTK.cmake.in
#              ${MSVTK_SUPERBUILD_BINARY_DIR}/UseMSVTK.cmake COPYONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Set C/CXX Flags
#
set(CMAKE_CXX_FLAGS ${MSVTK_CXX_FLAGS} CACHE STRING "CMake C Flags" FORCE)
set(CMAKE_C_FLAGS ${MSVTK_C_FLAGS} CACHE STRING "CMake CXX Flags" FORCE)

#-----------------------------------------------------------------------------
# Set the header template which defines custom export/import macros
# for shared libraries
#
set(MSVTK_EXPORT_HEADER_TEMPLATE "${MSVTK_SOURCE_DIR}/Libs/msvExport.h.in")

#-----------------------------------------------------------------------------
# Add utility cmake macro to create tests, it sets the environment path before
# executing the tests
#
include(CMake/msvMacroSimpleTest.cmake)

#-----------------------------------------------------------------------------
# Add MSVTK library subdirectories
#
add_subdirectory(Libs)

#-----------------------------------------------------------------------------
# Add MSVTK plugin subdirectories
#
foreach(plugin ${MSVTK_PLUGINS_SUBDIRS})
  if(MSVTK_PLUGIN_${plugin})
    add_subdirectory(Plugins/${plugin})
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Add MSVTK application subdirectories
#
foreach(app ${MSVTK_APPLICATIONS_SUBDIRS})
  if(MSVTK_APP_${app})
    add_subdirectory(Applications/${app})
  endif()            
endforeach()

#-----------------------------------------------------------------------------
# Add MSVTK examples subdirectories
#
add_subdirectory(Examples)

#-----------------------------------------------------------------------------
# Style Checking configuration
#
if(MSVTK_USE_KWSTYLE)
  include(Utilities/KWStyle/KWStyle.cmake)
endif()

#---------------------------------------------------------------------------
# Documentation
#
add_subdirectory( Utilities/Documentation )

#-----------------------------------------------------------------------------
# The commands in this directory are intended to be executed as
# the end of the whole configuration process, as a "last step".
# This directory is typically the last SUBDIRS in the main CMakeLists.txt.
add_subdirectory(Utilities/LastConfigureStep)

#---------------------------------------------------------------------------
# Packaging
#
include( CMake/msvCPack.cmake )
