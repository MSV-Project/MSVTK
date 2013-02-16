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
#! \brief Add a ctest test and add the Qt library path to the PATH environment
#! variable.
macro(msv_test TEST_NAME)
  add_test(NAME ${TEST_NAME} COMMAND ${ARGN})
  if (WIN32)
    set(PATH
      "${QT_LIBRARY_DIR};$ENV{PATH}"
      )
    string(REPLACE ";" "\\;" PATH "${PATH}")
    set_tests_properties(${TEST_NAME} PROPERTIES ENVIRONMENT
      "PATH=${PATH}"
      )
  endif()
endmacro()

#! \brief Add ctest test.
macro(simple_test TEST_NAME)
  msv_test(${TEST_NAME}
    $<TARGET_FILE:msv${KIT}CxxTests> ${TEST_NAME} ${ARGN}
    )
endmacro()

#! \brief Add ctest test with data as input.
macro(simple_test_with_data TEST_NAME)
  simple_test(${TEST_NAME} ${ARGN} -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
endmacro()
