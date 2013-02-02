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

set(CPACK_INSTALL_CMAKE_PROJECTS)
#set(CMAKE_INSTALL_DEBUG_LIBRARIES true)

include(${MSVTK_CMAKE_DIR}/msvInstallCMakeProjects.cmake)

if(NOT PACKAGE_WITH_BUNDLE)
  include(${MSVTK_CMAKE_DIR}/msvInstallQt.cmake)
  #if(MSVTK_USE_PYTHONQT)
  #  include(${MSVTK_CMAKE_DIR}/msvInstallPythonQt.cmake)
  #endif()
  #if(MSVTK_USE_QtTesting)
  #  include(${MSVTK_CMAKE_DIR}/SlicerBlockInstallQtTesting.cmake)
  #endif()
  include(InstallRequiredSystemLibraries)
else()

  # Generate qt.conf
  file(WRITE ${MSVTK_BINARY_DIR}/CMake/qt.conf-to-install
"[Paths]
Plugins = ${MSVTK_QtPlugins_DIR}
")
  # .. and install
  install(FILES ${MSVTK_BINARY_DIR}/CMake/qt.conf-to-install
          DESTINATION ${MSVTK_INSTALL_ROOT}Resources
          COMPONENT Runtime
          RENAME qt.conf)

  set(executable_path @executable_path)
  set(MSVTK_cpack_bundle_fixup_directory ${MSVTK_BINARY_DIR}/CMake/msvCPackBundleFixup)
  configure_file(
    "${MSVTK_SOURCE_DIR}/CMake/msvCPackBundleFixup.cmake.in"
    "${MSVTK_cpack_bundle_fixup_directory}/msvCPackBundleFixup.cmake"
    @ONLY)
  # HACK - For a given directory, "install(SCRIPT ...)" rule will be evaluated first,
  #        let's make sure the following install rule is evaluated within its own directory.
  #        Otherwise, the associated script will be executed before any other relevant install rules.
  file(WRITE ${MSVTK_cpack_bundle_fixup_directory}/CMakeLists.txt
    "install(SCRIPT \"${MSVTK_cpack_bundle_fixup_directory}/msvCPackBundleFixup.cmake\")")
  add_subdirectory(${MSVTK_cpack_bundle_fixup_directory} ${MSVTK_cpack_bundle_fixup_directory}-binary)
endif()

# Install MSVTK
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${MSVTK_BINARY_DIR};MSVTK;ALL;/")

# -------------------------------------------------------------------------
# Package properties
# -------------------------------------------------------------------------
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multi Scale Visualization Toolkit")

set(CPACK_MONOLITHIC_INSTALL ON)

set(CPACK_PACKAGE_NAME "MSVTK")
set(CPACK_PACKAGE_VENDOR "MSVTK Community")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${MSVTK_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${MSVTK_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MSVTK_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MSVTK_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MSVTK_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${MSVTK_VERSION}")
set(CPACK_SYSTEM_NAME "${MSVTK_OS}-${MSVTK_ARCHITECTURE}")

if(APPLE)
  set(CPACK_PACKAGE_ICON "${MSVTK_SOURCE_DIR}/msvLogo.icns")
elseif(WIN32)
  set(CPACK_PACKAGE_ICON "${MSVTK_SOURCE_DIR}\\\\msvLogo.png")
else()
  set(CPACK_PACKAGE_ICON "${MSVTK_SOURCE_DIR}/msvLogo.png")
endif()

# MSVTK does require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)
set(CPACK_PACKAGE_EXECUTABLES)
set(CMAKE_INSTALL_DO_STRIP TRUE)

foreach(app ${MSVTK_APPLICATIONS_SUBDIRS})
  if(MSVTK_APP_${app})
    get_target_property(${app}_EXECUTABLE_NAME ${app} OUTPUT_NAME)
    list(APPEND CPACK_PACKAGE_EXECUTABLES "${${app}_EXECUTABLE_NAME}" "${app}")
  endif()
endforeach()
# -------------------------------------------------------------------------
# File extensions
# -------------------------------------------------------------------------
set(FILE_EXTENSIONS)

if(FILE_EXTENSIONS)

  # For NSIS (Win32) now, we will add MacOSX support later (get back to Wes)

  if(WIN32 AND NOT UNIX)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
    foreach(ext ${FILE_EXTENSIONS})
      string(LENGTH "${ext}" len)
      math(EXPR len_m1 "${len} - 1")
      string(SUBSTRING "${ext}" 1 ${len_m1} ext_wo_dot)
      set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
            WriteRegStr HKCR \\\"${APPLICATION_NAME}\\\" \\\"\\\" \\\"${APPLICATION_NAME} supported file\\\"
            WriteRegStr HKCR \\\"${APPLICATION_NAME}\\\\shell\\\\open\\\\command\\\" \\\"\\\" \\\"$\\\\\\\"$INSTDIR\\\\${EXECUTABLE_NAME}.exe$\\\\\\\" $\\\\\\\"%1$\\\\\\\"\\\"
            WriteRegStr HKCR \\\"${ext}\\\" \\\"\\\" \\\"${APPLICATION_NAME}\\\"
            WriteRegStr HKCR \\\"${ext}\\\" \\\"Content Type\\\" \\\"application/x-${ext_wo_dot}\\\"
          ")
      set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
            DeleteRegKey HKCR \\\" ${APPLICATION_NAME}\\\"
            DeleteRegKey HKCR \\\"${ext}\\\"
          ")
    endforeach()
  endif()
endif()

# -------------------------------------------------------------------------
# Disable source generator enabled by default
# -------------------------------------------------------------------------
set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "Enable to build TBZ2 source packages" FORCE)
set(CPACK_SOURCE_TGZ  OFF CACHE BOOL "Enable to build TGZ source packages" FORCE)
set(CPACK_SOURCE_TZ   OFF CACHE BOOL "Enable to build TZ source packages" FORCE)

# -------------------------------------------------------------------------
# Enable generator
# -------------------------------------------------------------------------
if(UNIX OR APPLE)
  set(CPACK_GENERATOR "TGZ")
  if(PACKAGE_WITH_BUNDLE)
    set(CPACK_GENERATOR "DragNDrop")
  endif()
elseif(WIN32)
  set(CPACK_GENERATOR "NSIS")
endif()

include(CPack)
