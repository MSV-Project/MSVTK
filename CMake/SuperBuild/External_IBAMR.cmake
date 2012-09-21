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
# IBAMR
#

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED IBAMR_DIR AND NOT EXISTS ${IBAMR_DIR})
  message(FATAL_ERROR "IBAMR_DIR variable is defined but corresponds to non-existing directory")
endif()

#set(IBAMR_enabling_variable IBAMR_LIBRARIES)

set(additional_vtk_cmakevars )
if(MINGW)
  list(APPEND additional_vtk_cmakevars -DCMAKE_USE_PTHREADS:BOOL=OFF)
endif()

set(IBAMR_DEPENDENCIES "")

# Include dependent projects if any
msvMacroCheckExternalProjectDependency(IBAMR)
set(proj IBAMR)

if(NOT DEFINED IBAMR_DIR)

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

#     message(STATUS "Adding project:${proj}")

  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build
    PREFIX ${proj}${ep_suffix}
    GIT_REPOSITORY ${git_protocol}@bitbucket.org:ricortiz/ibamr.git
    GIT_TAG "master"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DBUILD_TESTING:BOOL=OFF
      ${additional_vtk_cmakevars}
      -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
      -DLIBRARY_OUTPUT_PATH:STRING=${MSVTK_BINARY_DIR}/MSVTK-build/bin
      -DEXECUTABLE_OUTPUT_PATH:STRING=${MSVTK_BINARY_DIR}/MSVTK-build/bin
      -DBUILD_TESTING:BOO:=OFF
    DEPENDS
      ${IBAMR_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  
else()
  msvMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()

list(APPEND MSVTK_SUPERBUILD_EP_ARGS -DIBAMR_DIR:PATH=${IBAMR_DIR})


