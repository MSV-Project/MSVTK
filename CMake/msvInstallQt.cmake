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
# -------------------------------------------------------------------------
# Find and install Qt
# -------------------------------------------------------------------------
set(QT_INSTALL_LIB_DIR ${MSVTK_INSTALL_LIB_DIR})

set(QTLIBLIST QTCORE QTGUI QTNETWORK QTXML QTTEST QTSCRIPT QTSQL QTSVG QTOPENGL QTWEBKIT PHONON QTXMLPATTERNS)
if(UNIX OR APPLE)
  list(APPEND QTLIBLIST QTDBUS)
endif()
foreach(qtlib ${QTLIBLIST})
  if(QT_${qtlib}_LIBRARY_RELEASE)
    if(WIN32)
      get_filename_component(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
      set(qtlibsuffix "4")
      if (CMAKE_INSTALL_DEBUG_LIBRARIES)
        set (qtlibsuffix "d4")
      endif()
      install(FILES ${QT_DLL_PATH_tmp}/${qtlib}${qtlibsuffix}.dll
        DESTINATION bin COMPONENT Runtime)
    elseif((UNIX OR APPLE) AND NOT PACKAGE_WITH_BUNDLE)
      # Install .so and versioned .so.x.y
      get_filename_component(QT_LIB_DIR_tmp ${QT_${qtlib}_LIBRARY_RELEASE} PATH)
      get_filename_component(QT_LIB_NAME_tmp ${QT_${qtlib}_LIBRARY_RELEASE} NAME)
      install(DIRECTORY ${QT_LIB_DIR_tmp}/
        DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime
        FILES_MATCHING PATTERN "${QT_LIB_NAME_tmp}*"
        PATTERN "${QT_LIB_NAME_tmp}*.debug" EXCLUDE)
    elseif(APPLE)
      install(DIRECTORY "${QT_${qtlib}_LIBRARY_RELEASE}"
        DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime)
    else()
      message(CRITICAL "Not a valid configuration")
    endif()
  endif()
endforeach()
