# Copyright (c) 2017-2019, Lawrence Livermore National Security, LLC and
# other Axom Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (BSD-3-Clause)


#------------------------------------------------------------------------------
# Setup Conduit
#
# This file defines:
#  CONDUIT_FOUND - If Conduit was found
#  CONDUIT_INCLUDE_DIRS - The Conduit include directories
#  
#  If found, the conduit CMake targets will also be imported
#------------------------------------------------------------------------------

# first Check for CONDUIT_DIR

if(NOT CONDUIT_DIR)
    MESSAGE(FATAL_ERROR "Could not find Conduit. Conduit requires explicit CONDUIT_DIR.")
endif()

if(NOT EXISTS ${CONDUIT_DIR}/lib/cmake/conduit.cmake)
    MESSAGE(FATAL_ERROR "Could not find Conduit cmake include file (${CONDUIT_DIR}/lib/cmake/conduit.cmake)")
endif()

include(${CONDUIT_DIR}/lib/cmake/conduit.cmake)

set(CONDUIT_INCLUDE_DIRS ${CONDUIT_DIR}/include/conduit)

# handle the QUIETLY and REQUIRED arguments and set CONDUIT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(CONDUIT  DEFAULT_MSG
                                  CONDUIT_INCLUDE_DIRS
                                  )





