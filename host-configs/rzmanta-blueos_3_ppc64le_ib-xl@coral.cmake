##################################
# !!!! This is a generated file, edit at own risk !!!!
##################################

# Copyright (c) 2017-2019, Lawrence Livermore National Security, LLC and
# other Axom Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (BSD-3-Clause)

##################################

# SYS_TYPE: blueos_3_ppc64le_ib
# Compiler Spec: xl@coral
##################################

# CMake executable path: /usr/workspace/wsrzc/axom/thirdparty_libs/builds/2018_11_14_15_17_37/xl-coral/cmake-3.9.6/bin/cmake

##############
# Compilers
##############

# C compiler used by spack
set(CMAKE_C_COMPILER "/usr/tce/packages/xl/xl-2018.05.18/bin/xlc" CACHE PATH "")

# C++ compiler used by spack
set(CMAKE_CXX_COMPILER "/usr/tce/packages/xl/xl-2018.05.18/bin/xlC" CACHE PATH "")

# Fortran compiler used by spack
set(ENABLE_FORTRAN ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER "/usr/tce/packages/xl/xl-2018.05.18/bin/xlf2003" CACHE PATH "")

##############
# TPLs
##############

# Root directory for generated TPLs
set(TPL_ROOT "/usr/workspace/wsrzc/axom/thirdparty_libs/builds/2018_11_14_15_17_37/xl-coral" CACHE PATH "")

# hdf5 from uberenv
set(HDF5_DIR "${TPL_ROOT}/hdf5-1.8.19" CACHE PATH "")

# scr not built by uberenv

# conduit from uberenv
set(CONDUIT_DIR "${TPL_ROOT}/conduit-0.3.1" CACHE PATH "")

# mfem from uberenv
set(MFEM_DIR "${TPL_ROOT}/mfem-3.4.0" CACHE PATH "")

# python not built by uberenv

set(ENABLE_DOCS OFF CACHE BOOL "")

# shroud not built by uberenv

# uncrustify not built by uberenv

# lcov and genhtml not built by uberenv

##############
# MPI
##############

set(ENABLE_MPI ON CACHE BOOL "")

set(MPI_C_COMPILER "/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-xl-2018.05.18/bin/mpicc" CACHE PATH "")

set(MPI_CXX_COMPILER "/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-xl-2018.05.18/bin/mpicxx" CACHE PATH "")

set(MPI_Fortran_COMPILER "/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-xl-2018.05.18/bin/mpif90" CACHE PATH "")

set(MPIEXEC "/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-xl-2018.05.18/bin/mpirun" CACHE PATH "")

set(MPIEXEC_NUMPROC_FLAG "-np" CACHE PATH "")

set(BLT_MPI_COMMAND_APPEND "mpibind" CACHE PATH "")

##############
# Other machine specifics
##############

set(ENABLE_GTEST_DEATH_TESTS ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER_ID "XL" CACHE PATH "All of BlueOS compilers report clang due to nvcc, override to proper compiler family")

set(CMAKE_C_COMPILER_ID "XL" CACHE PATH "All of BlueOS compilers report clang due to nvcc, override to proper compiler family")

set(CMAKE_CXX_COMPILER_ID "XL" CACHE PATH "All of BlueOS compilers report clang due to nvcc, override to proper compiler family")

set(BLT_FORTRAN_FLAGS "-WF,-C! -qxlf2003=polymorphic" CACHE PATH "Convert C-style comments to Fortran and link fortran exes to C++ libraries")


