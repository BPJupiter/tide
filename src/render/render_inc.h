// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef RENDER_INC_H
#define RENDER_INC_H

///////////////////////
// Backend Constants

#define R_BACKEND_STUB 0
#define R_BACKEND_WGPU 1

///////////////////////
// Decide on Backend

#if !defined(R_BACKEND) && OS_WINDOWS
# define R_BACKEND R_BACKEND_WGPU
#elif !defined(R_BACKEND) && OS_LINUX
# define R_BACKEND R_BACKEND_WGPU
#endif

///////////////////
// Main Includes

#include "render_core.h"

//////////////////////
// Backend Includes

#if R_BACKEND == R_BACKEND_STUB
# include "stub/render_stub.h"
#elif R_BACKEND == R_BACKEND_WGPU
# include "wgpu/render_wgpu.h"
#else
# error Renderer backend not specified.
#endif

#endif // RENDER_INC_H
