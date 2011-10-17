set(KIT ECG)
project(msvtk${KIT})

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------

set(msvtk${KIT}_SRCS
  qMSECGMainWindow.cxx
  )

set(msvtk${KIT}_MOC_SRCS
  qMSECGMainWindow.h
  )

set(msvtk${KIT}_UI_SRCS
  Resources/UI/qMSECGMainWindow.ui
  )

set(msvtk${KIT}_QRC
  Resources/qMSECG.qrc
  )

QT4_WRAP_CPP(msvtk${KIT}_SRCS ${msvtk${KIT}_MOC_SRCS})
QT4_WRAP_UI(msvtk${KIT}_UI_CXX ${msvtk${KIT}_UI_SRCS})
QT4_ADD_RESOURCES(msvtk${KIT}_QRC_SRCS ${msvtk${KIT}_QRC})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MSVTK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${CTK_INCLUDE_DIRS}
  )

include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Build the library so it can be tested in Testing/Cpp.
# --------------------------------------------------------------------------

add_library(msvtk${KIT}
  ${msvtkECG_SRCS}
  ${msvtkECG_UI_CXX}
  ${msvtkECG_QRC_SRCS}
  )

target_link_libraries(msvtk${KIT}
  ${MSVTK_LIBRARIES}
  ${CTK_LIBRARIES}
  ${VTK_LIBRARIES}
  )


# --------------------------------------------------------------------------
# Build the executable
# --------------------------------------------------------------------------

if(NOT APPLE)
  #set(msvECG_EXE_SUFFIX -real)
  set(msvtkECG_EXE_SUFFIX )
endif()

add_executable(${KIT}
  ecg.cxx
  )

target_link_libraries(${KIT}
  msvtk${KIT}
  )

#-----------------------------------------------------------------------------
# Configure
# --------------------------------------------------------------------------
string(TOUPPER ${KIT} UPPER_CASE_KIT)
set(MY_LIBRARY_EXPORT_DIRECTIVE "MSVTK_${UPPER_CASE_KIT}_EXPORT")
set(MY_EXPORT_HEADER_PREFIX vtkMS${KIT})
set(MY_LIBNAME msvtk${KIT})

configure_file(
  ${MSVTK_SOURCE_DIR}/CMake/vtkMSExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
  )

set(dynamicHeaders
  "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

# --------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
