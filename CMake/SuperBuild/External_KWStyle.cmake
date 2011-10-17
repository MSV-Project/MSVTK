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

#
# KWStyle
#

set(kwstyle_DEPENDS)
if(MSVTK_USE_KWSTYLE)

  # Sanity checks
  if(DEFINED MSVTK_KWSTYLE_EXECUTABLE AND NOT EXISTS ${MSVTK_KWSTYLE_EXECUTABLE})
    message(FATAL_ERROR "MSVTK_KWSTYLE_EXECUTABLE variable is defined but corresponds to non-existing executable")
  endif()

  set(proj KWStyle-CVSHEAD)
  set(proj_DEPENDENCIES)

  set(kwstyle_DEPENDS ${proj})

  if(NOT DEFINED MSVTK_KWSTYLE_EXECUTABLE)
    # Set CMake OSX variable to pass down the external project
    set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
    if(APPLE)
      list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
        -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
    endif()

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      BINARY_DIR ${proj}-build
      PREFIX ${proj}${ep_suffix}
      LIST_SEPARATOR ${sep}
      CVS_REPOSITORY ":pserver:anoncvs:@public.kitware.com:/cvsroot/KWStyle"
      CVS_MODULE "KWStyle"
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
        ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
        -DBUILD_TESTING:BOOL=OFF
      DEPENDS
        ${proj_DEPENDENCIES}
      )
    SET(MSVTK_KWSTYLE_EXECUTABLE ${ep_install_dir}/bin/KWStyle)

    # Since KWStyle is an executable, there is not need to add its corresponding
    # library output directory to MSVTK_EXTERNAL_LIBRARY_DIRS
  ELSE()
    ctkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  ENDIF()

  LIST(APPEND MSVTK_SUPERBUILD_EP_ARGS -DMSVTK_KWSTYLE_EXECUTABLE:FILEPATH=${MSVTK_KWSTYLE_EXECUTABLE})

ENDIF()
