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

set(IBAMR_DEPENDENCIES "OPENMPI;SAMRAI;HDF5;SILO;BLITZ;PETSC;HYPRE")

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
#     SVN_REPOSITORY https://ibamr.googlecode.com/svn/branches/ibamr-dev
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/${proj}/configure 
      "CFLAGS=${ep_common_c_flags}"
      "CXXFLAGS=${ep_common_cxx_flags}"
      "FCFLAGS=${CMAKE_F_FLAGS}"
      "FFLAGS=${CMAKE_F_FLAGS}"
      CPPFLAGS=-DOMPI_SKIP_MPICXX
      CC=${ep_install_dir}/bin/mpicc
      CXX=${ep_install_dir}/bin/mpicxx
      F77=${ep_install_dir}/bin/mpif90 
      FC=${ep_install_dir}/bin/mpif90 
      MPICC=${ep_install_dir}/bin/mpicc 
      MPICXX=${ep_install_dir}/bin/mpicxx 
      --with-samrai=${ep_install_dir}
      --with-hdf5=${ep_install_dir}
      --with-petsc=${PETSC_DIR}
      --with-petsc-arch=${PETSC_ARCH}
      --with-hypre=${ep_install_dir}
      --with-blitz=${BLITZ_DIR}
      --with-silo=${ep_install_dir}
    TEST_BEFORE_INSTALL 1
    LOG_CONFIGURE 1
#     LOG_BUILD 1
    LOG_INSTALL 1
    TEST_COMMAND make check
    BUILD_COMMAND make lib
    DEPENDS
      ${IBAMR_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  
else()
  msvMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()

list(APPEND MSVTK_SUPERBUILD_EP_ARGS -DIBAMR_DIR:PATH=${IBAMR_DIR})


