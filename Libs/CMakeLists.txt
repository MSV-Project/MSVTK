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
project(msvLibs)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------
set(dirs
# FileWatcher
  )

list(APPEND dirs
  VTK
  Qt
  )

if(ENABLE_IBAMR)
  list(APPEND dirs 
    IBAMR
    )
endif()  
#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  message(STATUS "Configuring library: ${dir}")
  add_subdirectory(${dir})
endforeach()

#-----------------------------------------------------------------------------
# Generate list of include directories
#-----------------------------------------------------------------------------
set(libraries)
set(include_dirs)
foreach(dir ${dirs})
  foreach(project ${${dir}_PROJECTS})
    set(library msv${dir}${project})
    list(APPEND libraries ${library})
    list(APPEND include_dirs ${${library}_INCLUDE_DIRS})
  endforeach()
endforeach()

# Set variables MSVTK_Libs_*
set(MSVTK_Libs_LIBRARIES ${libraries} CACHE INTERNAL "MSVTK Libs libraries" FORCE)
set(MSVTK_Libs_INCLUDE_DIRS ${include_dirs} CACHE INTERNAL "MSVTK Libs includes" FORCE)
#message("MSVTK_Libs_LIBRARIES: ${MSVTK_Libs_LIBRARIES}")
#message("MSVTK_Libs_INCLUDE_DIRS: ${MSVTK_Libs_INCLUDE_DIRS}")
