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
# PETSc
#

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED PETSC_DIR AND NOT EXISTS ${PETSC_DIR})
  message(FATAL_ERROR "PETSC_DIR variable is defined but corresponds to non-existing directory")
endif()

#set(PETSC_enabling_variable PETSC_LIBRARIES)

set(PETSC_DEPENDENCIES "HYPRE;OPENMPI")

# Include dependent projects if any
msvMacroCheckExternalProjectDependency(PETSC)
set(proj PETSC)

if(NOT DEFINED PETSC_DIR)

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

#     message(STATUS "Adding project:${proj}")

  # Set PETSc specific environment variables
  set(ENV{PETSC_DIR} ${CMAKE_BINARY_DIR}/${proj})
  set(ENV{PETSC_ARCH} build)
  
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}
    PREFIX ${proj}${ep_suffix}
    URL http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.1-p8.tar.gz
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND $ENV{PETSC_DIR}/configure
      --prefix=${ep_install_dir}
      --COPTFLAGS=-O3
      --CXXOPTFLAGS=-O3
      --FOPTFLAGS=-O3
      --with-default-arch=0 
      --PETSC_ARCH=$ENV{PETSC_ARCH}
      --PETSC_DIR=$ENV{PETSC_DIR}
      --LDFLAGS="-L${ep_install_dir}/lib -Wl,-rpath,${ep_install_dir}/lib"
      --with-debugging=0 
      --with-c++-support
      --with-hypre=1
      --with-hypre-dir=${ep_install_dir}
      --with-mpi=1
      --with-mpi-dir=${ep_install_dir}
      --with-x=0
    BUILD_COMMAND make PETSC_DIR=$ENV{PETSC_DIR} PETSC_ARCH=$ENV{PETSC_ARCH} all
    DEPENDS
      ${PETSC_DEPENDENCIES}
    )
    set(${proj}_DIR $ENV{PETSC_DIR}/$ENV{PETSC_ARCH})

else()
  msvMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()

list(APPEND MSVTK_SUPERBUILD_EP_ARGS -DPETSC_DIR:PATH=${PETSC_DIR})

