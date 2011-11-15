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

set(MSVTK_DEPENDENCIES VTK CTK)

#-----------------------------------------------------------------------------
# WARNING - No change should be required after this comment
#           when you are adding a new external project dependency.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Declare MSVTK_EXTERNAL_LIBRARY_DIRS variable - This variable stores
# the library output directory associated with the different external project
# It's then used in Utilities/LastConfigureStep/MSVTKGenerateMSVTKConfig.cmake to
# configure MSVTKConfig.cmake.in
# This variable would then be exposed to project building against MSVTK
#SET(MSVTK_EXTERNAL_LIBRARY_DIRS)

#-----------------------------------------------------------------------------
# Git protocole option
#
option(MSVTK_USE_GIT_PROTOCOL "If behind a firewall turn this OFF to use http instead." ON)

set(git_protocol "git")
if(NOT MSVTK_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#
include(ExternalProject)
include(msvMacroEmptyExternalProject)
include(msvMacroCheckExternalProjectDependency)

#SET(ep_base "${CMAKE_BINARY_DIR}/SuperBuild")
#SET_PROPERTY(DIRECTORY PROPERTY EP_BASE ${ep_base})

set(ep_install_dir ${CMAKE_BINARY_DIR}/SuperBuild/Install)
set(ep_suffix      "-cmake")

set(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS}")

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

# Use this value where semi-colons are needed in ep_add args:
set(sep "^^")

#-----------------------------------------------------------------------------
# Collect MSVTK library target dependencies
#

#ctkMacroCollectAllTargetLibraries("${MSVTK_LIBS_SUBDIRS}" "Libs" ALL_TARGET_LIBRARIES)
#ctkMacroCollectAllTargetLibraries("${MSVTK_PLUGINS_SUBDIRS}" "Plugins" ALL_TARGET_LIBRARIES)
#ctkMacroCollectAllTargetLibraries("${MSVTK_APPLICATIONS_SUBDIRS}" "Applications" ALL_TARGET_LIBRARIES)
#MESSAGE(STATUS ALL_TARGET_LIBRARIES:${ALL_TARGET_LIBRARIES})

#-----------------------------------------------------------------------------
# Initialize NON_MSVTK_DEPENDENCIES variable
#
# Using the variable ALL_TARGET_LIBRARIES initialized above with the help
# of the macro ctkMacroCollectAllTargetLibraries, let's get the list of all Non-MSVTK dependencies.
# NON_MSVTK_DEPENDENCIES is expected by the macro ctkMacroShouldAddExternalProject
#ctkMacroGetAllNonProjectTargetLibraries("${ALL_TARGET_LIBRARIES}" NON_MSVTK_DEPENDENCIES)
#MESSAGE(STATUS NON_MSVTK_DEPENDENCIES:${NON_MSVTK_DEPENDENCIES})

#-----------------------------------------------------------------------------
# Qt and Doxygen are expected to be setup by MSVTK/CMakeLists.txt just before it includes the SuperBuild script
#

#-----------------------------------------------------------------------------
# Include external projects
#

# This variable will contain the list of CMake variable specific to each external project
# that should passed to MSVTK.
# The item of this list should have the following form: -D<EP>_DIR:PATH=${<EP>_DIR}
# where '<EP>' is an external project name.
set(MSVTK_SUPERBUILD_EP_ARGS)

msvMacroCheckExternalProjectDependency(MSVTK)

#-----------------------------------------------------------------------------
# Makes sure ${MSVTK_BINARY_DIR}/MSVTK-build/bin exists
IF(NOT EXISTS ${MSVTK_BINARY_DIR}/MSVTK-build/bin)
  FILE(MAKE_DIRECTORY ${MSVTK_BINARY_DIR}/MSVTK-build/bin)
ENDIF()


#-----------------------------------------------------------------------------
# Generate cmake variable name corresponding to Libs, Plugins and Applications
#
#set(ctk_libs_bool_vars)
#foreach(lib ${MSVTK_LIBS_SUBDIRS})
#  list(APPEND ctk_libs_bool_vars MSVTK_LIB_${lib})
#endforeach()

#SET(ctk_plugins_bool_vars)
#foreach(plugin ${MSVTK_PLUGINS_SUBDIRS})
#  list(APPEND ctk_plugins_bool_vars MSVTK_PLUGIN_${plugin})
#endforeach()

#set(ctk_applications_bool_vars)
#foreach(app ${MSVTK_APPLICATIONS_SUBDIRS})
#  list(APPEND ctk_applications_bool_vars MSVTK_APP_${app})
#endforeach()

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#

set(ctk_cmake_boolean_args
  BUILD_TESTING
  #BUILD_QTDESIGNER_PLUGINS
  MSVTK_USE_KWSTYLE
  #WITH_COVERAGE
  #DOCUMENTATION_TARGET_IN_ALL
  #CTEST_USE_LAUNCHERS
  #${ctk_libs_bool_vars}
  #${ctk_plugins_bool_vars}
  #${ctk_applications_bool_vars}
  #${ctk_lib_options_list}
  )

set(ctk_superbuild_boolean_args)
foreach(ctk_cmake_arg ${ctk_cmake_boolean_args})
  list(APPEND ctk_superbuild_boolean_args -D${ctk_cmake_arg}:BOOL=${${ctk_cmake_arg}})
endforeach()

# MESSAGE("CMake args:")
# FOREACH(arg ${ctk_superbuild_boolean_args})
#   MESSAGE("  ${arg}")
# ENDFOREACH()

#-----------------------------------------------------------------------------
# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

#-----------------------------------------------------------------------------
# MSVTK Configure
#
SET(proj MSVTK-Configure)

ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ctk_superbuild_boolean_args}
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    -DMSVTK_SUPERBUILD:BOOL=OFF
    -DDOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
    -DDOXYGEN_EXECUTABLE:FILEPATH=${DOXYGEN_EXECUTABLE}
    -DMSVTK_SUPERBUILD_BINARY_DIR:PATH=${MSVTK_BINARY_DIR}
    -DMSVTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${MSVTK_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    -DMSVTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${MSVTK_CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    -DMSVTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${MSVTK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    -DMSVTK_INSTALL_BIN_DIR:STRING=${MSVTK_INSTALL_BIN_DIR}
    -DMSVTK_INSTALL_LIB_DIR:STRING=${MSVTK_INSTALL_LIB_DIR}
    -DMSVTK_INSTALL_INCLUDE_DIR:STRING=${MSVTK_INSTALL_INCLUDE_DIR}
    -DMSVTK_INSTALL_DOC_DIR:STRING=${MSVTK_INSTALL_DOC_DIR}
    -DMSVTK_BUILD_SHARED_LIBS:BOOL=${MSVTK_BUILD_SHARED_LIBS}
    -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DMSVTK_CXX_FLAGS:STRING=${MSVTK_CXX_FLAGS}
    -DMSVTK_C_FLAGS:STRING=${MSVTK_C_FLAGS}
    -DMSVTK_EXTERNAL_LIBRARY_DIRS:STRING=${MSVTK_EXTERNAL_LIBRARY_DIRS}
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    ${MSVTK_SUPERBUILD_EP_ARGS}
    #${dependency_args}
  SOURCE_DIR ${MSVTK_SOURCE_DIR}
  BINARY_DIR ${MSVTK_BINARY_DIR}/MSVTK-build
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${MSVTK_DEPENDENCIES}
  )

if(CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(msvtk_build_cmd "$(MAKE)")
else()
  set(msvtk_build_cmd ${CMAKE_COMMAND} --build ${MSVTK_BINARY_DIR}/MSVTK-build --config ${CMAKE_CFG_INTDIR})
endif()

#-----------------------------------------------------------------------------
# MSVTK
#
#MESSAGE(STATUS SUPERBUILD_EXCLUDE_MSVTKBUILD_TARGET:${SUPERBUILD_EXCLUDE_MSVTKBUILD_TARGET})
if(NOT DEFINED SUPERBUILD_EXCLUDE_MSVTKBUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_MSVTKBUILD_TARGET)
  set(MSVTKBUILD_TARGET_ALL_OPTION "ALL")
else()
  set(MSVTKBUILD_TARGET_ALL_OPTION "")
endif()

add_custom_target(MSVTK-build ${MSVTKBUILD_TARGET_ALL_OPTION}
  COMMAND ${msvtk_build_cmd}
  WORKING_DIRECTORY ${MSVTK_BINARY_DIR}/MSVTK-build
  )
add_dependencies(MSVTK-build MSVTK-Configure)

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of MSVTK project itself
#
add_custom_target(MSVTK
  COMMAND ${msvtk_build_cmd}
  WORKING_DIRECTORY ${MSVTK_BINARY_DIR}/MSVTK-build
  )
