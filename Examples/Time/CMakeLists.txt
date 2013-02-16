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

project(msvTimeExamples)
set(KIT TimeExamples)

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

add_definitions( -DDATA_TESTING_PATH="${PROJECT_SOURCE_DIR}/../Data/" )

# msvQTimePlayerWidget
add_executable(TimePlayerWidgetExample msvQTimePlayerWidgetExample.cxx)
target_link_libraries(TimePlayerWidgetExample msvQtWidgets)

# msvVTKFileSeriesReader
add_executable(FileSeriesReaderExample msvVTKFileSeriesReaderExample.cxx)
target_link_libraries(FileSeriesReaderExample msvVTKParallel)
