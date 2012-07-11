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
# SAMRAI
#

# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED SAMRAI_DIR AND NOT EXISTS ${SAMRAI_DIR})
  message(FATAL_ERROR "SAMRAI_DIR variable is defined but corresponds to non-existing directory")
endif()

#set(SAMRAI_enabling_variable SAMRAI_LIBRARIES)

set(SAMRAI_DEPENDENCIES "SILO;OPENMPI;HDF5")

# Include dependent projects if any
msvMacroCheckExternalProjectDependency(SAMRAI)
set(proj SAMRAI)

if(NOT DEFINED SAMRAI_DIR)

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()
  
#   message(STATUS "Downloading SAMRAI-IBAMR patch")
  file(DOWNLOAD http://ibamr.googlecode.com/files/SAMRAI-v2.4.4-patch-111217.gz
	  ${CMAKE_CURRENT_BINARY_DIR}/${proj}${ep_suffix}/src/SAMRAI-v2.4.4-patch-111217.gz 
	  STATUS _out)
  list(GET _out 0 _out_error)
  list(GET _out 1 _out_msg)
  
  if(NOT ${_out_error} EQUAL "0")
    message(FATAL_ERROR "Error downloading SAMRAY-IBAMR patch: ${_out_msg}")
  endif()
  
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build
    PREFIX ${proj}${ep_suffix}
    URL https://computation.llnl.gov/casc/SAMRAI/download/SAMRAI-v2.4.4.tar.gz
    UPDATE_COMMAND ""
    INSTALL_COMMAND make install 
    PATCH_COMMAND gunzip -c 
	${CMAKE_CURRENT_BINARY_DIR}/${proj}${ep_suffix}/src/SAMRAI-v2.4.4-patch-111217.gz 
	| patch -p2 &&
	./source/scripts/includes --link
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/${proj}/configure
      --libdir=${ep_install_dir}/lib
      --prefix=${ep_install_dir} 
      --with-CC=gcc 
      --with-CXX=g++ 
      --with-F77=gfortran 
      --with-MPICC=${ep_install_dir}/bin/mpicc 
      --with-hdf5=${ep_install_dir}
      --without-petsc 
      --without-hypre 
      --with-silo=${ep_install_dir}
      --without-blaslapack 
      --without-cubes 
      --without-eleven 
      --without-kinsol 
      --without-sundials 
      --without-x 
      --with-doxygen 
      --with-dot 
      --enable-opt 
      --enable-implicit-template-instantiation 
      --disable-deprecated
    DEPENDS
      ${SAMRAI_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

else()
  msvMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()

list(APPEND MSVTK_SUPERBUILD_EP_ARGS -DSAMRAI_DIR:PATH=${SAMRAI_DIR})


