// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef HOOKS_INC_H
#define HOOKS_INC_H

//////////////////////////
// Backend Constants

#define HK_BACKEND_DETOURS 1

/////////////////////////
// Decide On Backend

#if !defined(HK_BACKEND)
# if OS_WINDOWS
#  define HK_BACKEND HK_BACKEND_DETOURS
# elif OS_LINUX
# endif
#endif

/////////////////////////
// Main Includes

#include "hooks.h"

////////////////////////
// Backend Includes

#if HK_BACKEND == HK_BACKEND_DETOURS
# include "detours/hooks_detours.h"
#else
# error Hooks backend not specified.
#endif

#endif // HOOKS_INC_H
