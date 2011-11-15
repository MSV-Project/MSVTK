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
# VTK
#

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory")
endif()

#set(VTK_enabling_variable VTK_LIBRARIES)

set(additional_vtk_cmakevars )
if(MINGW)
  list(APPEND additional_vtk_cmakevars -DCMAKE_USE_PTHREADS:BOOL=OFF)
endif()

#if(MSVTK_LIB_Scripting/Python/Core_PYTHONQT_USE_VTK)
#  list(APPEND additional_vtk_cmakevars
#    -DPYTHON_EXECUTABLE:PATH=${PYTHON_EXECUTABLE}
#    -DPYTHON_LIBRARIES:FILEPATH=${PYTHON_LIBRARIES}
#    -DPYTHON_DEBUG_LIBRARIES:FILEPATH=${PYTHON_DEBUG_LIBRARIES}
#    )
#endif()

set(VTK_DEPENDENCIES "")
# Include dependent projects if any
msvMacroCheckExternalProjectDependency(VTK)
set(proj VTK)

if(NOT DEFINED VTK_DIR)

  #set(revision_tag "v5.8.0")
  set(revision_tag fea2d622cf01dfd22f727330dbace97d4af892db)
  if(${proj}_REVISION_TAG)
    set(revision_tag ${${proj}_REVISION_TAG})
  endif()

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
    GIT_REPOSITORY ${git_protocol}://vtk.org/VTK.git
    GIT_TAG ${revision_tag}
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
      -DVTK_WRAP_TCL:BOOL=OFF
      -DVTK_USE_TK:BOOL=OFF
      -DVTK_WRAP_PYTHON:BOOL=${MSVTK_LIB_Scripting/Python/Core_PYTHONQT_USE_VTK}
      -DVTK_WRAP_JAVA:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
      -DDESIRED_QT_VERSION:STRING=4
      -DVTK_USE_GUISUPPORT:BOOL=ON
      -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
      -DVTK_USE_QT:BOOL=ON
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    DEPENDS
      ${VTK_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  # Since the link directories associated with VTK is used, it makes sens to
  # update MSVTK_EXTERNAL_LIBRARY_DIRS with its associated library output directory
  list(APPEND MSVTK_EXTERNAL_LIBRARY_DIRS ${VTK_DIR}/bin)

else()
  msvMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()

list(APPEND MSVTK_SUPERBUILD_EP_ARGS -DVTK_DIR:PATH=${VTK_DIR})


