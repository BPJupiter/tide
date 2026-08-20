// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#undef LAYER_COLOR
#define LAYER_COLOR 0xc22121ff

#include "render_core.c"

#if R_BACKEND == R_BACKEND_STUB
# include "stub/render_stub.c"
#elif R_BACKEND == R_BACKEND_WGPU
# include "wgpu/render_wgpu.c"
#else
# error Renderer backend not specified.
#endif
