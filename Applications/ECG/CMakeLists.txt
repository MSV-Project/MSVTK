set(KIT ECG)
project(msv${KIT})

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------

set(msv${KIT}_SRCS
  msvQECGMainWindow.cxx
  msvVTKECGButtonsManager.cxx
  msvQECGAboutDialog.cxx
  )

set(msv${KIT}_MOC_SRCS
  msvQECGMainWindow.h
  msvQECGAboutDialog.h
  )

set(msv${KIT}_UI_SRCS
  Resources/UI/msvQECGMainWindow.ui
  Resources/UI/msvQECGAboutDialog.ui
  )

set(msv${KIT}_QRC
  Resources/msvQECG.qrc
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


# --------------------------------------------------------------------------
# Build the executable
# --------------------------------------------------------------------------

add_executable(${KIT}
  ecg.cxx
  )
set_target_properties(${KIT} PROPERTIES OUTPUT_NAME ecg)

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
