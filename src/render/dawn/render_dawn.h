
#ifndef RENDER_DAWN_H
#define RENDER_DAWN_H

#include <webgpu/webgpu.h>
#include <dawn/dawn_proc_table.h>
#include <dawn/dawn_proc.h>

#if OS_WINDOWS
# pragma comment(lib, "webgpu_dawn")
# pragma comment(lib, "dawn_proc")
#endif

//////////////////////
// Backend Includes

#if OW_WINDOWS
# include "render/dawn/win32/render_dawn_win32.h"
#elif OS_LINUX
# include "render/dawn/linux/render_dawn_linux.h"
#else
# error OS portion of Dawn rendering backend not defined.
#endif

////////////////////
// Generated Code

#include "generated/render_dawn.meta.h"

/////////////////////////
// Shader Metadata Types

typedef struct D_Dawn_Vertex_Attribute R_Dawn_Vertex_Attribute;
struct D_Dawn_Vertex_Attribute {
    u64 shader_location;
    String8 name;
    WGPUVertexFormat format;
    u64 offset;
};

typedef struct R_Dawn_Vertex_Attribute_Array R_Dawn_Vertex_Attribute_Array;
struct R_Dawn_Vertex_Attribute_Array {
    R_Dawn_Vertex_Attribute *v;
    u64 count;
};

/////////////////////////
// C-Side Shader Types

typedef struct R_Dawn_Uniforms_Rect R_Dawn_Uniforms_Rect;
struct R_Dawn_Uniforms_Rect {
    Vec2f32 viewport_size;
    f32 opacity;
    f32 _padding0_;
    Mat4x4f32 texture_sample_channel_map;
    Vec2f32 texture_t2d_size;
    Vec2f32 translate;
    Vec4f32 xform[3];
    Vec2f32 xform_scale;
};

typedef struct R_Dawn_Uniforms_Blur_Pass R_Dawn_Uniforms_Blur_Pass;
struct R_Dawn_Uniforms_Blur_Pass {
    Rng2f32 rect;
    Vec4f32 corner_radii;
    Vec2f32 direction;
    Vec2f32 viewport_size;
    u32 blur_count;
    u8 _padding0_[204];
};
StaticAssert(sizeof(R_Dawn_Uniforms_Blur_Pass) % 256 == 0, NotAligned); // dynamic uniform buffer offset must be aligned to minUniformBufferOffsetAlignment (256 bytes)

typedef struct R_Dawn_Uniforms_Blur R_Dawn_Uniforms_Blur;
struct R_Dawn_Uniforms_Blur {
    R_Dawn_Uniforms_Blur_Pass passes[Axis2_COUNT];
    Vec4f32 kernel[32];
};

typedef struct R_Dawn_Uniforms_Mesh R_Dawn_Uniform_Mesh;
struct R_Dawn_Uniforms_Mesh {
    Mat4x4f32 xform;
};

///////////////////////
// Format Info Types

typedef struct R_Dawn_Format_Info R_Dawn_Format_Info;
struct R_Dawn_Format_Info {
    WGPUTextureFormat format;
    u64 byte_per_texel;
};

/////////////////////
// Main State Type

typedef struct R_Dawn_Tex2D R_Dawn_Tex2D;
struct R_Dawn_Tex2D {
    R_Dawn_Tex2D *next;
    u64 generation;
    WGPUTexture texture;
    WGPUTextureView view;
    R_ResourceKind kind;
    Vec2s32 size;
    R_Tex2D_Format format;
};

typedef R_Dawn_Buffer R_Dawn_Buffer;
struct R_Dawn_Buffer {
    R_Dawn_Buffer *next;
    u64 generation;
    WGPUBuffer buffer;
    R_ResourceKind kind;
    u64 size;
};

typedef struct R_Dawn_Window R_Dawn_Window;
struct R_Dawn_Window {
    R_Dawn_Window *next;
    u64 generation;

    // surface/framebuffer
    WGPUSurface surface;
    WGPUTextureFormat surface_format;
    WGPUTexture framebuffer;
    WGPUTextureView framebuffer_view;

    // stating buffer
    WGPUTexture stage_color;
    WGPUTextureView stage_color_view;
    WGPUTexture stage_scratch_color;
    WGPUTextureView stage_scratch_color_view;

    // geo3d buffer
    WGPUTexture geo3d_color;
    WGPUTextureView geo3d_color_view;
    WGPUTexture geo3d_depth;
    WGPUTextureView geo3d_depth_view;

    // last state
    Vec2s32 last_resolution;
};

typedef struct R_Dawn_Flush_Buffer R_Dawn_Flush_Buffer;
struct R_Dawn_Flush_Buffer {
    R_Dawn_Flush_Buffer *next;
    WGPUBuffer buffer;
};

typedef struct R_Dawn_State R_Dawn_State;
struct R_Dawn_State {
    // state
    Arena *arena;
    u64 window_count;
    R_Dawn_Window *first_free_window;
    R_Dawn_Tex2D *first_free_tex2d;
    R_Dawn_Buffer *first_free_buffer;
    R_Dawn_Tex2D *first_to_free_tex2d;
    R_Dawn_Buffer *first_to_free_buffer;
    RWMutex device_rw_mutex;

    // base dawn/webgpu objects
    DawnProcTable procs;
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;

    // pipeline-independent state
    WGPUSampler samples[R_Tex2DSampleKind_COUNT];
    WGPUBindGroupLayout tex2d_bind_group_layout;
    WGPUBindGroupLayout uniform_bind_group_layouts[R_Dawn_UniformTypeKind_COUNT];
    WGPUBuffer instance_scratch_buffer_64kb;

    // backups
    R_Handle backup_texture;

    // shaders/pipelines (webgpu pipelines are monolithic PSOs, unlike
    // d3d11's separate vs/ps stages or gl's linked program-per-stage-pair, so
    // shader modules and pipeline state objects are tracked in separate arrays)
    WGPUShaderModule shaders[R_Dawn_ShaderKind_COUNT];
    WGPURenderPipeline pipelines[R_Dawn_PipelineKind_COUNT];
    WGPUBuffer uniform_type_kind_buffers[R_Dawn_UniformTypeKind_COUNT];

    // buffers to flush at subsequent frame
    Arena *buffer_flush_arena;
    R_Dawn_Flush_Buffer *first_buffer_to_flush;
    R_Dawn_Flush_Buffer *last_buffer_to_flush;
};

/////////////
// Globals

global R_Dawn_State *r_dawn_state = 0;
global read_only R_Dawn_Window r_dawn_window_nil = {&r_dawn_window_nil};
global read_only R_Dawn_Tex2D r_dawn_tex2d_nil = {&r_dawn_tex2d_nil};
global read_only R_Dawn_Buffer r_dawn_buffer_nil = {&r_dawn_buffer_nil};

/////////////
// Helpers

internal R_Dawn_Window *r_dawn_window_from_handle(R_Handle handle);
internal R_Handle r_dawn_handle_from_window(R_Dawn_Window *window);
internal R_Dawn_Tex2D *r_dawn_tex2d_from_handle(R_Handle handle);
internal R_Handle r_dawn_handle_from_tex2d(R_Dawn_Tex2D *texture);
internal R_Dawn_Buffer *r_dawn_buffer_from_handle(R_Handle handle);
internal R_Handle r_dawn_handle_from_buffer(R_Dawn_Buffer *buffer);
internal WGPUBuffer r_dawn_instance_buffer_from_size(u64 size);
internal R_Dawn_Format_Info r_dawn_format_info_from_tex2d_format(R_Tex2D_Format fmt);
internal void r_dawn_buffer_usage_from_resource_kind(R_ResourceKind kind, WGPUBufferUsage *out_usage, bool32 *out_mapped_at_creation);
internal WGPUTextureUsage r_dawn_texture_usage_from_resource_kind(R_ResourceKind kind);
internal void r_dawn_uncaptured_error_callback(WGPUErrorType type, const char *message, void *userdata);
internal void r_dawn_device_lost_callback(WGPUDeviceLostReason reason, const char *message, void *userdata);

///////////////////////
// @per_os_impl Hooks

internal void r_dawn_os_init(Cmd_Line *cmdline);
internal WGPUSurface r_dawn_os_surface_from_window(WGPUInstance instance, WM_Window window);

#endif // RENDER_DAWN_H
