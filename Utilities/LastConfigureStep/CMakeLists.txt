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
#  Module:    Utilities/LastConfigureStep/CMakeLists.txt
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

# The commands in this directory are intended to be executed at
# the end of the whole configuration process, as a "last step".
# This directory is typically the last SUBDIRS in the main CMakeLists.txt.
# It enable the above commands to use variables that might have been configured
# in previous SUBDIRS. This is especially important when it comes to
# the CONFIGURE_FILE command, since in IMMEDIATE mode that command will
# use the current values of CMake variables instead of waiting until the
# end of CMakeLists processing, i.e. instead of waiting until some variables
# are configured in SUBDIRS.


SET(MSVTK_target_dependencies)

# Loop over the list of MSVTK* targets and retrieve the associated dependencies
FOREACH(MSVTK_target ${CTEST_PROJECT_SUBPROJECTS})
  LIST(APPEND MSVTK_target_dependencies ${${MSVTK_target}_LIB_DEPENDS})
ENDFOREACH()


SET(MSVTK_EXTERNAL_LIBRARIES)

# Loop over dependencies and append to MSVTK_EXTERNAL_LIBRARIES 
# target dependency which are neither MSVTK or static library
SET(link_type)
FOREACH(MSVTK_target_dependency ${MSVTK_target_dependencies})
  STRING(REGEX MATCH "^(general|optimized|debug)$" is_link_type ${MSVTK_target_dependency})
  IF(is_link_type)
    SET(link_type ${MSVTK_target_dependency})
  ELSE()
    # Sanity checks - link_type shouldn't be empty
    IF(link_type STREQUAL "")
      MESSAGE(SEND_ERROR "link_type shouldn't be empty")
    ENDIF()
    # Make sure MSVTK_target_dependency is not a MSVTK library
    STRING(REGEX MATCH "(^(c|C)(t|T)(k|K)[a-zA-Z0-9]+$)" is_MSVTK_library ${MSVTK_target_dependency})
    IF(NOT is_MSVTK_library)
      # Make sure MSVTK_target_dependency is not a static library
      GET_FILENAME_COMPONENT(MSVTK_target_dependency_ext ${MSVTK_target_dependency} EXT)
      IF(NOT MSVTK_target_dependency_ext STREQUAL CMAKE_STATIC_LIBRARY_SUFFIX)
        LIST(APPEND MSVTK_EXTERNAL_LIBRARIES ${link_type} ${MSVTK_target_dependency})
      ENDIF()
      SET(link_type)
    ENDIF()
  ENDIF()
ENDFOREACH()

# Clean variable
IF(REMOVE_DUPLICATES)
  LIST(REMOVE_DUPLICATES MSVTK_BASE_INCLUDE_DIRS)
ENDIF(REMOVE_DUPLICATES)

#-----------------------------------------------------------------------------
# Create the MSVTKConfig.cmake file containing the MSVTK configuration.
# Since it might generate configuration file depending
INCLUDE(MSVTKGenerateMSVTKConfig.cmake)
