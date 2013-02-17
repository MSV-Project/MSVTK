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

set(KIT HAI)
project(msv${KIT})

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------

set(msv${KIT}_SRCS
  msvQHAIMainWindow.cxx
  msvQHAIAboutDialog.cxx
  )

set(msv${KIT}_MOC_SRCS
  msvQHAIMainWindow.h
  msvQHAIAboutDialog.h
  )

set(msv${KIT}_UI_SRCS
  Resources/UI/msvQHAIMainWindow.ui
  Resources/UI/msvQHAIAboutDialog.ui
  )

set(msv${KIT}_QRC
  Resources/msvQHAI.qrc
  )

QT4_WRAP_CPP(msv${KIT}_SRCS ${msv${KIT}_MOC_SRCS})
QT4_WRAP_UI(msv${KIT}_UI_CXX ${msv${KIT}_UI_SRCS})
QT4_ADD_RESOURCES(msv${KIT}_QRC_SRCS ${msv${KIT}_QRC})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MSVTK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${CTK_INCLUDE_DIRS}
  ${msvVTKParallel_INCLUDE_DIRS}
  ${msvVTKWidgets_INCLUDE_DIRS}
  ${msvQtWidgets_INCLUDE_DIRS}
  )

include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Build the library so it can be tested in Testing/Cpp.
# --------------------------------------------------------------------------

add_library(msv${KIT}
  ${msv${KIT}_SRCS}
  ${msv${KIT}_UI_CXX}
  ${msv${KIT}_QRC_SRCS}
  )

target_link_libraries(msv${KIT}
  ${MSVTK_LIBRARIES}
  ${CTK_LIBRARIES}
  ${VTK_LIBRARIES}
  msvVTKParallel
  msvVTKWidgets
  msvQtWidgets
  )

install(TARGETS msv${KIT}
  RUNTIME DESTINATION ${MSVTK_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${MSVTK_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${MSVTK_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the executable
# --------------------------------------------------------------------------

add_executable(${KIT}
  hai.cxx
  )
set_target_properties(${KIT} PROPERTIES OUTPUT_NAME hai)

target_link_libraries(${KIT}
  msv${KIT}
  )

#-----------------------------------------------------------------------------
# Configure
# --------------------------------------------------------------------------
set(MY_LIBRARY_EXPORT_DIRECTIVE "MSV_${KIT}_EXPORT")
set(MY_EXPORT_HEADER_PREFIX MSV_${KIT})
set(MY_LIBNAME ${PROJECT_NAME})

configure_file(
  ${MSVTK_SOURCE_DIR}/CMake/msvExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Export.h
  )

set(dynamicHeaders
  "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Export.h")

# --------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Install
# --------------------------------------------------------------------------
if(NOT PACKAGE_WITH_BUNDLE)
  set(${KIT}_INSTALL_DESTINATION_ARGS RUNTIME DESTINATION ${MSVTK_INSTALL_BIN_DIR})
else()
  set(${KIT}_INSTALL_DESTINATION_ARGS RUNTIME DESTINATION ".")
endif()

install(TARGETS ${KIT}
  ${${KIT}_INSTALL_DESTINATION_ARGS}
  COMPONENT Runtime)
