project(msvExamples)

cmake_minimum_required(VERSION 2.8)
include_regular_expression("^.*$")

# -----------------------------------------------------------------------------
# Disable deprecation warnings for standard C and STL functions in VS2005 and
# later (no, we don't need IF(CMAKE_COMPILER_2005) ... )
# -----------------------------------------------------------------------------
IF(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
ENDIF(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)

IF(NOT MSVTK_BINARY_DIR)
  find_package(MSVTK REQUIRED)
  include(${MSVTK_USE_FILE})
ENDIF(NOT MSVTK_BINARY_DIR)

set(dirs
# FileWatcher
  )

list(APPEND dirs
  Time
  Embedding
  )

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  message(STATUS "Configuring examples: msv${dir}")
  add_subdirectory(${dir})
endforeach()
