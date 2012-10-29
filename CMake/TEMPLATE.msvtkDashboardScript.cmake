#
# OS: Linux x86_64
# Hardware: x86_64 GNU/Linux
# GPU: NA
#

# Note: The specific version and processor type of this machine should be reported in the 
# header above. Indeed, this file will be send to the dashboard as a NOTE file. 

cmake_minimum_required(VERSION 2.8)

#
# For additional information, see http://www.commontk.org/index.php/Dashboard_setup
#

#
# Dashboard properties
#
set(MY_OPERATING_SYSTEM "Linux") # Windows, Linux, Darwin... 
set(MY_COMPILER "GCC-4.7")

if (NOT MY_QT_VERSION)
  set(MY_QT_VERSION "4.7")
endif()

set(QT_QMAKE_EXECUTABLE "/usr/bin/qmake")
set(CTEST_SITE "melcor.kitware") # for example: mymachine.kitware, mymachine.dkfz, ...
set(CTEST_DASHBOARD_ROOT "$ENV{HOME}/Dashboards/MSVTK")

if(NOT EXISTS "${CTEST_DASHBOARD_ROOT}/Logs")
  file(MAKE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/Logs/")
endif()  

if (NOT MY_CMAKE_VERSION)
   set(MY_CMAKE_VERSION "CMake 2.8.8")
endif()

set(CTEST_CMAKE_COMMAND   "/usr/bin/cmake")
if (NOT CTEST_CMAKE_GENERATOR)
  set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
endif()
if (NOT MY_BITNESS)
  set(MY_BITNESS "64")
endif()
#
# Dashboard options
#
set(WITH_KWSTYLE FALSE)
set(WITH_MEMCHECK FALSE)
set(WITH_COVERAGE FALSE)
set(WITH_DOCUMENTATION FALSE)

if (NOT CTEST_BUILD_CONFIGURATION)
  set(CTEST_BUILD_CONFIGURATION "Debug")
endif()

set(CTEST_TEST_TIMEOUT 500)
set(CTEST_BUILD_FLAGS "") # Use multiple CPU cores to build
set(CTEST_LOG_FILE "${CTEST_DASHBOARD_ROOT}/Logs/Nightly_${CTEST_BUILD_CONFIGURATION}}_${MY_BITNESS}bits.log")

# experimental: 
#     - run_ctest() macro will be called *ONE* time
#     - binary directory will *NOT* be cleaned
# continuous: 
#     - run_ctest() macro will be called EVERY 5 minutes ... 
#     - binary directory will *NOT* be cleaned
#     - configure/build will be executed *ONLY* if the repository has been updated
# nightly: 
#     - run_ctest() macro will be called *ONE* time
#     - binary directory *WILL BE* cleaned
if (NOT SCRIPT_MODE)
  set(SCRIPT_MODE "experimental") # "experimental", "continuous", "nightly"
endif()

#
# Project specific properties
#
set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/source")
set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/build_${CTEST_BUILD_CONFIGURATION}_${MY_BITNESS}bits_${SCRIPT_MODE}_${MY_QT_VERSION}_${MY_CMAKE_VERSION}")

# set additional CMakeCache options 
set(ADDITIONAL_CMAKECACHE_OPTION "")

# set any extra environment variables here
set(ENV{DISPLAY} ":0")

find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
find_program(CTEST_GIT_COMMAND NAMES git)
message ("********************************************************COMMAND : ${CTEST_GIT_COMMAND}")

#
# Git repository - Overwrite the default value provided by the driver script
#
# set(GIT_REPOSITORY http://github.com/YOURUSERNAME/MSVTK.git)

##########################################
# WARNING: DO NOT EDIT BEYOND THIS POINT #
##########################################
set(CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Project specific properties
#
set(CTEST_PROJECT_NAME "MSVTK")
set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM}_${MY_COMPILER}_${MY_BITNESS}_QT${MY_QT_VERSION}_${MY_CMAKE_VERSION}_${CTEST_BUILD_CONFIGURATION}")

#
# Display build info
#
message("site name: ${CTEST_SITE}")
message("build name: ${CTEST_BUILD_NAME}")
message("script mode: ${SCRIPT_MODE}")
message("coverage: ${WITH_COVERAGE}, memcheck: ${WITH_MEMCHECK}")

#
# Convenient macro allowing to download a file
#
macro(downloadFile url dest)
  file(DOWNLOAD ${url} ${dest} STATUS status)
  list(GET status 0 error_code)
  list(GET status 1 error_msg)
  if(error_code)
    message(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  endif()
endmacro()

#
# Download and include dashboard driver script
#
set(url http://midas3.kitware.com/midas/download?items=25472)
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile(${url} ${dest})

include(${dest})
