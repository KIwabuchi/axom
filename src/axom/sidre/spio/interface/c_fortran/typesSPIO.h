// typesSPIO.h
// This file is generated by Shroud 0.12.2. Do not edit.
//
// Copyright (c) 2017-2024, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
// For C users and C++ implementation

#ifndef TYPESSPIO_H
#define TYPESSPIO_H

#ifdef __cplusplus
extern "C" {
#endif

// helper capsule_SPIO_IOManager
struct s_SPIO_IOManager
{
  void* addr; /* address of C++ memory */
  int idtor;  /* index of destructor */
};
typedef struct s_SPIO_IOManager SPIO_IOManager;

// helper capsule_data_helper
struct s_SPIO_SHROUD_capsule_data
{
  void* addr; /* address of C++ memory */
  int idtor;  /* index of destructor */
};
typedef struct s_SPIO_SHROUD_capsule_data SPIO_SHROUD_capsule_data;

void SPIO_SHROUD_memory_destructor(SPIO_SHROUD_capsule_data* cap);

#ifdef __cplusplus
}
#endif

#endif  // TYPESSPIO_H
