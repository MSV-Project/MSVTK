# - Find IBAMR
# Find the IBAMR libraries 
#
#  This module defines the following variables:
#     IBAMR_FOUND

#=============================================================================
# Copyright 20011-2012 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Assume not found.
SET(IBAMR_FOUND 0)

# Construct consitent error messages for use below.
SET(IBAMR_DIR_MESSAGE "IBAMR not found.  Set the IBAMR_DIR cmake cache entry to the directory containing IBAMRConfig.cmake.")

# Check whether IBAMR has already been found.
IF(IBAMR_DIR)
  IF(EXISTS ${IBAMR_DIR}/IBAMRConfig.cmake)
    SET(IBAMR_FOUND 1)
    INCLUDE(${IBAMR_DIR}/IBAMRConfig.cmake)
  ENDIF()
ENDIF(IBAMR_DIR)

#-----------------------------------------------------------------------------
IF(NOT IBAMR_FOUND)
  # IBAMR not found, explain to the user how to specify its location.
  IF(IBAMR_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${IBAMR_DIR_MESSAGE})
  ELSE(IBAMR_FIND_REQUIRED)
    MESSAGE(STATUS ${IBAMR_DIR_MESSAGE})
  ENDIF(IBAMR_FIND_REQUIRED)
ENDIF(NOT IBAMR_FOUND)
