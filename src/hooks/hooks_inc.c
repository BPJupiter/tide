// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#include "hooks.c"

#if HK_BACKEND == HK_BACKEND_DETOURS
# include "detours/hooks_detours.c"
#else
# error Hooks backend not specified.
#endif

