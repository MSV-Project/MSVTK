###########################################################################
#
#  Library:   MSVTK
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
###########################################################################
#
#  Program:   Visualization Toolkit
#  Module:    vtkGenerateVTKConfig.cmake
#
#  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
#
#  All rights reserved.
#  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#     PURPOSE.  See the above copyright notice for more information.
#
###########################################################################

#
# Generate the MSVTKConfig.cmake file in the build tree. Also configure
# one for installation.  The file tells external projects how to use MSVTK.
#

#INCLUDE(MSVTKFunctionGeneratePluginUseFile)

# Construct version numbers for MSVTKConfigVersion.cmake.
SET(_MSVTK_VERSION_MAJOR ${MSVTK_MAJOR_VERSION})
SET(_MSVTK_VERSION_MINOR ${MSVTK_MINOR_VERSION})
SET(_MSVTK_VERSION_PATCH ${MSVTK_BUILD_VERSION})
# We use odd minor numbers for development versions.
# Use a date for the development patch level.
# IF("${_MSVTK_VERSION_MINOR}" MATCHES "[13579]$")
#   INCLUDE(${MSVTK_SOURCE_DIR}/Utilities/kwsys/kwsysDateStamp.cmake)
#   SET(_MSVTK_VERSION_PATCH
#     "${KWSYS_DATE_STAMP_YEAR}${KWSYS_DATE_STAMP_MONTH}${KWSYS_DATE_STAMP_DAY}"
#     )
# ENDIF()

#-----------------------------------------------------------------------------
# Settings shared between the build tree and install tree.


#-----------------------------------------------------------------------------
# Settings specific to the build tree.

# The install-only section is empty for the build tree.
SET(MSVTK_CONFIG_INSTALL_ONLY)

# The "use" file.
SET(MSVTK_USE_FILE ${MSVTK_SUPERBUILD_BINARY_DIR}/UseMSVTK.cmake)

# Generate list of target to exports
SET(MSVTK_TARGETS_TO_EXPORT ${MSVTK_LIBRARIES} ${MSVTK_PLUGIN_LIBRARIES})

# Append MSVTK PythonQt static libraries
IF(NOT MSVTK_BUILD_SHARED_LIBS)
  FOREACH(lib ${MSVTK_WRAPPED_LIBRARIES_PYTHONQT})
    LIST(APPEND MSVTK_TARGETS_TO_EXPORT ${lib}PythonQt)
  ENDFOREACH()
ENDIF()

# Export targets so they can be imported by a project using MSVTK
# as an external library
EXPORT(TARGETS ${MSVTK_TARGETS_TO_EXPORT} FILE ${MSVTK_SUPERBUILD_BINARY_DIR}/MSVTKExports.cmake)

# Generate a file containing plugin specific variables
#SET(MSVTK_PLUGIN_USE_FILE "${MSVTK_SUPERBUILD_BINARY_DIR}/MSVTKPluginUseFile.cmake")
#MSVTKFunctionGeneratePluginUseFile(${MSVTK_PLUGIN_USE_FILE})

# Write a set of variables containing library specific include and library directories
SET(MSVTK_LIBRARY_INCLUDE_DIRS_CONFIG)
FOREACH(lib ${MSVTK_LIBRARIES})
  SET(${lib}_INCLUDE_DIRS ${${lib}_SOURCE_DIR} ${${lib}_BINARY_DIR})
  MSVTKFunctionGetIncludeDirs(${lib}_INCLUDE_DIRS ${lib})
  SET(MSVTK_LIBRARY_INCLUDE_DIRS_CONFIG "${MSVTK_LIBRARY_INCLUDE_DIRS_CONFIG}
SET(${lib}_INCLUDE_DIRS \"${${lib}_INCLUDE_DIRS}\")")

  MSVTKFunctionGetLibraryDirs(${lib}_LIBRARY_DIRS ${lib})
  SET(MSVTK_LIBRARY_LIBRARY_DIRS_CONFIG "${MSVTK_LIBRARY_LIBRARY_DIRS_CONFIG}
SET(${lib}_LIBRARY_DIRS \"${${lib}_LIBRARY_DIRS}\")")
ENDFOREACH()

SET(MSVTK_BASE_INCLUDE_DIRS 
  ${msvVTKParallel_INCLUDE_DIRS}
  ${msvVTKWidgets_INCLUDE_DIRS}
  ${msvQtWidgets_INCLUDE_DIRS}
  )

# Determine the include directories needed.
SET(MSVTK_INCLUDE_DIRS_CONFIG
  ${MSVTK_BASE_INCLUDE_DIRS}
)

# Library directory.
SET(MSVTK_LIBRARY_DIRS_CONFIG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Plug-in output directory
IF(WIN32)
  SET(_plugin_output_type "RUNTIME")
ELSE()
  SET(_plugin_output_type "LIBRARY")
ENDIF()
IF(DEFINED MSVTK_PLUGIN_${_plugin_output_type}_OUTPUT_DIRECTORY)
  IF(IS_ABSOLUTE "${MSVTK_PLUGIN_${_plugin_output_type}_OUTPUT_DIRECTORY}")
    SET(MSVTK_PLUGIN_LIBRARIES_DIR_CONFIG "${MSVTK_PLUGIN_${_plugin_output_type}_OUTPUT_DIRECTORY}")
  ELSE()
    SET(MSVTK_PLUGIN_LIBRARIES_DIR_CONFIG "${CMAKE_${_plugin_output_type}_OUTPUT_DIRECTORY}/${MSVTK_PLUGIN_${_plugin_output_type}_OUTPUT_DIRECTORY}")
  ENDIF()
ELSE()
  SET(MSVTK_PLUGIN_LIBRARIES_DIR_CONFIG "${CMAKE_${_plugin_output_type}_OUTPUT_DIRECTORY}/plugins")
ENDIF()

# External project libraries.
SET(MSVTK_EXTERNAL_LIBRARIES_CONFIG ${MSVTK_EXTERNAL_LIBRARIES})

# External project library directory.
SET(MSVTK_EXTERNAL_LIBRARY_DIRS_CONFIG ${MSVTK_EXTERNAL_LIBRARY_DIRS})

# Runtime library directory.
SET(MSVTK_RUNTIME_LIBRARY_DIRS_CONFIG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Binary executable directory.
SET(MSVTK_EXECUTABLE_DIRS_CONFIG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# QtDesigner plugins directory
SET(MSVTK_QTDESIGNERPLUGINS_DIR_CONFIG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# MSVTK external projects variables
STRING(REPLACE "^" ";" MSVTK_SUPERBUILD_EP_VARNAMES "${MSVTK_SUPERBUILD_EP_VARNAMES}")
SET(MSVTK_SUPERBUILD_EP_VARS_CONFIG)
FOREACH(varname ${MSVTK_SUPERBUILD_EP_VARNAMES})
  SET(MSVTK_SUPERBUILD_EP_VARS_CONFIG
   "${MSVTK_SUPERBUILD_EP_VARS_CONFIG}
SET(MSVTK_${varname} \"${${varname}}\")")
ENDFOREACH()

# Executable locations.

# CMake extension module directory.
SET(MSVTK_CMAKE_DIR_CONFIG ${MSVTK_CMAKE_DIR})
SET(MSVTK_CMAKE_UTILITIES_DIR_CONFIG ${MSVTK_CMAKE_UTILITIES_DIR})

# Build configuration information.
SET(MSVTK_CONFIGURATION_TYPES_CONFIG ${CMAKE_CONFIGURATION_TYPES})
SET(MSVTK_BUILD_TYPE_CONFIG ${CMAKE_BUILD_TYPE})

#-----------------------------------------------------------------------------
# Configure MSVTKConfig.cmake for the build tree.
CONFIGURE_FILE(${MSVTK_SOURCE_DIR}/MSVTKConfig.cmake.in
               ${MSVTK_SUPERBUILD_BINARY_DIR}/MSVTKConfig.cmake @ONLY IMMEDIATE)
#CONFIGURE_FILE(${MSVTK_SOURCE_DIR}/MSVTKConfigVersion.cmake.in
#               ${MSVTK_SUPERBUILD_BINARY_DIR}/MSVTKConfigVersion.cmake @ONLY IMMEDIATE)
#CONFIGURE_FILE(${MSVTK_SOURCE_DIR}/MSVTKConfig.h.in
#               ${MSVTK_CONFIG_H_INCLUDE_DIR}/MSVTKConfig.h @ONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Settings specific to the install tree.

#-----------------------------------------------------------------------------
# Configure MSVTKConfig.cmake for the install tree.
