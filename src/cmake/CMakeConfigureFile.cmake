#
# CMakeConfigureFile.cmake - Create header of configuration options
#

if( (CMAKE_CXX_STANDARD EQUAL 11) OR (CMAKE_CXX_STANDARD EQUAL 14) )
    set(ATK_USE_CXX11 TRUE)
endif()


## Add a definition to the generated config file for each library dependency   
## (optional and built-in) that we might need to know about in the code
## Note: BLT adds USE_MPI and USE_OPENMP as compile define flags for targets
##       that are configured with MPI and OPENMP, respectively.

set(TPL_DEPS CONDUIT HDF5 SPARSEHASH FMT BOOST MPI)  # vars of the form DEP_FOUND
foreach(dep in ${TPL_DEPS})
    if( ${dep}_FOUND OR ENABLE_${dep} )
        set(ATK_USE_${dep} TRUE  )
    endif()
endforeach()

# Handle MPI Fortran headers
if(ENABLE_MPI AND ENABLE_FORTRAN)
  if(MPI_Fortran_USE_MPIF)
    set(ATK_USE_MPIF_HEADER TRUE)
  endif()
endif()


# If Sparsehash was found, ATK_USE_SPARSEHASH was set above in the TPL_DEPS
# loop.  If not, we must use a standard container--std::unordered_map when
# using C++11, std::map otherwise.  std::map is expected to perform poorly
# with large amounts of Sidre objects, so it is recommended to make sure
# Sparsehash is available for non-C++ 11 builds.
if(NOT ATK_USE_SPARSEHASH)
  if(ATK_USE_CXX11)
    set(ATK_USE_STD_UNORDERED_MAP TRUE)
  else()
    set(ATK_USE_STD_MAP TRUE)
  endif()
endif()


## Add a configuration define for each enabled toolkit component
set(COMPS COMMON LUMBERJACK SLIC SLAM SIDRE MINT QUEST SPIO)
foreach(comp in ${COMPS})
    if( ENABLE_${comp} )
        set(ATK_USE_${comp} TRUE)
    endif()
endforeach()



configure_file(
    components/common/src/config.hpp.in
    ${HEADER_INCLUDES_DIRECTORY}/common/config.hpp
)
