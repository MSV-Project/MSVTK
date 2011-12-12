project(msvQtWidgets)
set(KIT QT_WIDGETS)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${VTK_INCLUDE_DIRS}
  ${CTK_INCLUDE_DIRS}
  ${msvVTKParallel_INCLUDE_DIRS}
  ${msvVTKWidgets_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.(h|txx)")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(msvQtWidgets_SRCS
  msvQTimePlayerWidget.cxx
  )

set(msv${KIT}_MOC_SRCS
  msvQTimePlayerWidget.h
  )

set(msv${KIT}_UI_SRCS
  Resources/UI/msvQTimePlayerWidget.ui
  )

set(msv${KIT}_QRC
  Resources/msvQWidgets.qrc
  )

QT4_WRAP_CPP(msv${KIT}_SRCS ${msv${KIT}_MOC_SRCS})
QT4_WRAP_UI(msv${KIT}_UI_CXX ${msv${KIT}_UI_SRCS})
QT4_ADD_RESOURCES(msv${KIT}_QRC_SRCS ${msv${KIT}_QRC})
# Abstract/pure virtual classes

#set_source_files_properties(
#  ABSTRACT
#  )

# Helper classes

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})
set(srcs ${msvQtWidgets_SRCS})

add_library(${lib_name} ${srcs}
  ${msv${KIT}_SRCS}
  ${msv${KIT}_UI_CXX}
  ${msv${KIT}_QRC_SRCS}
  )

target_link_libraries(${lib_name}
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  ${VTK_LIBRARIES}
  msvVTKParallel
  msvVTKWidgets
  )

# Apply user-defined properties to the library target.
if(MSVTK_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${MSVTK_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

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
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
