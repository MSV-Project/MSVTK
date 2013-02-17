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

#! \brief Search and configure Qt libraries
#! Qt Plugins can be specified to be use by the project.
#! \ingroup CMakeUtilities
macro(msvMacroSetupQt)

  set(minimum_required_qt_version "4.6")

  find_package(Qt4)

  # This option won't show up in the main CMake configure panel
  #mark_as_advanced(QT_QMAKE_EXECUTABLE)

  if(QT4_FOUND)

    if("${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}" VERSION_LESS "${minimum_required_qt_version}")
      message(FATAL_ERROR "error: MSVTK requires Qt >= ${minimum_required_qt_version} -- you cannot use Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}.")
    endif()

    #set(QT_USE_QTNETWORK ON)
    #set(QT_USE_QTSQL ON)
    set(QT_USE_QTOPENGL ON)
    #set(QT_USE_QTXML ON)
    set(QT_USE_QTTEST ${BUILD_TESTING})
    include(${QT_USE_FILE})

    # Set variable QT_INSTALLED_LIBRARY_DIR that will contains
    # Qt shared library
    set(QT_INSTALLED_LIBRARY_DIR ${QT_LIBRARY_DIR})
    if(WIN32)
      get_filename_component(QT_INSTALLED_LIBRARY_DIR ${QT_QMAKE_EXECUTABLE} PATH)
    endif()

  else(QT4_FOUND)
    message(WARNING "Warning: Qt4 was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable")
  endif(QT4_FOUND)

endmacro()
