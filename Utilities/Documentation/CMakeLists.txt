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

find_package( Doxygen QUIET)

if( DOXYGEN_FOUND )
  if( DOXYGEN_DOT_FOUND )

    # Compile a doxygen input filter for processing CMake scripts
    include(msvFunctionCMakeDoxygenFilterCompile)
    msvFunctionCMakeDoxygenFilterCompile(NAMESPACE "CMake")

    # Automatically generate documentation at build time
    set(all_arg)
    if(DOCUMENTATION_TARGET_IN_ALL)
      set(all_arg "ALL")
    endif()

    #
    # User documentation
    #
    set( MSVTK_USER_DOCUMENTATION "NO" )
    set( MSVTK_ADDITIONAL_EXCLUDE_PATTERN "*_p.h\  *Private.*" )
    set( MSVTK_DOCUMENTATION_HTML_TARZ TRUE)

    configure_file( Doxyfile.txt.in
      ${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt
      )

    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/generate_doc.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake
      @ONLY
      )

    set(userdoc_output "${CMAKE_CURRENT_BINARY_DIR}/doc/html/index.html")

    # Run doxygen as a custom command driven by a custom target.
    add_custom_command(
      OUTPUT ${userdoc_output}
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake"
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt"
      )

    add_custom_target(doc ${all_arg} DEPENDS ${userdoc_output})
    set_target_properties(doc PROPERTIES LABELS Documentation)

    #
    # Dev documentation
    #
    set( MSVTK_USER_DOCUMENTATION "YES" )
    set( MSVTK_ADDITIONAL_EXCLUDE_PATTERN "" )
    set( MSVTK_ADDITIONAL_FILE_PATTERN "*.c \  *.cc \  *.cxx \  *.cpp \  *.c++ \  *.hxx \  *.hpp \  *.h++ \  *.txx \  *.C \  *.CC \  *.C++ \  *.moc" )


    configure_file( Doxyfile.txt.in
      ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
      )

    add_custom_target( doc-dev
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
      )
    set_target_properties(doc-dev PROPERTIES LABELS Documentation)

  else( DOXYGEN_DOT_FOUND )
    message( WARNING
      "Warning: Dot not found - Documentation will not be created"
      )
  endif( DOXYGEN_DOT_FOUND )
endif( DOXYGEN_FOUND )
