#
# OS: Ubuntu 11.04 3.0.0-14-generic
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
set(MY_COMPILER "gcc4.6.1")

if (NOT MY_QT_VERSION)
  set(MY_QT_VERSION "474")
endif()

set(QT_QMAKE_EXECUTABLE "/home/michael/Projects/QtSDK1_1_3/Desktop/Qt/${MY_QT_VERSION}/gcc/bin/qmake.exe")
set(CTEST_GIT_COMMAND "/usr/lib/git-core/git")
set(CTEST_SITE "Vahalla.kitware") # for example: mymachine.kitware, mymachine.dkfz, ...
set(CTEST_DASHBOARD_ROOT "/home/michael/Projects/DashboardScripts/MSVTK/")

if (NOT MY_CMAKE_VERSION)
   set(MY_CMAKE_VERSION "CMake 2.8.5")
endif()

set(CTEST_CMAKE_COMMAND "/usr/bin/cmake")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(MY_BITNESS "64")

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
set(CTEST_BUILD_FLAGS "-j5") # Use multiple CPU cores to build
set(CTEST_LOG_FILE "/home/michael/Projects/DashboardScripts/MSVTK/Logs/Nightly-${CTEST_BUILD_CONFIGURATION}-${MY_BITNESS}.log")

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
set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}MSVTK")
set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}MSVTK-Superbuild-${CTEST_BUILD_CONFIGURATION}-${SCRIPT_MODE}-${MY_QT_VERSION}-${MY_CMAKE_VERSION}")

# Additionnal CMakeCache options - For example:

set(ADDITIONNAL_CMAKECACHE_OPTION "
  GIT_EXECUTABLE:FILEPATH=/usr/lib/git-core/git
")

# set any extra environment variables here
set(ENV{DISPLAY} ":0")

find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
find_program(CTEST_GIT_COMMAND NAMES git)

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
set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM}-${MY_COMPILER}-QT${MY_QT_VERSION}-${MY_CMAKE_VERSION}-${CTEST_BUILD_CONFIGURATION}")

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
set(url http://midas.kitware.com/bitstream/view/17238)
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile(${url} ${dest})
include(${dest})
