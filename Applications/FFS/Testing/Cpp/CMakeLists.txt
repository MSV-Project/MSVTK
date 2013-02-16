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

set(KIT FFS)
set(KIT_TEST_SRCS
  msvFFSTest1.cxx
  msvFFSTest2.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(msv${KIT}CxxTests ${KIT}CxxTests.cxx ${KIT_TEST_SRCS})
set_target_properties(msv${KIT}CxxTests PROPERTIES COMPILE_DEFINITIONS NDIM=3)
target_link_libraries(msv${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#

add_test(
  NAME ffsTest1
  COMMAND $<TARGET_FILE:msv${KIT}CxxTests> ffsTest1
  --ffs $<TARGET_FILE_DIR:msv${KIT}CxxTests>
)

simple_test( msvQFFSMainWindowTest1 )
