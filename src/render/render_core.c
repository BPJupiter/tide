// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////
// Generated Code

#include "generated/render.meta.c"

/////////////
// Helpers

internal u64 r_bytes_per_pixel_from_tex2dfmt(R_Tex2DFmt fmt)
{
    u64 num_bits = 0;
    for EachIndex(channel_idx, 4) {
        R_ChannelSizeKind size_kind = r_size_kind_from_tex2dfmt_channel(fmt, channel_idx);
        switch (size_kind)
        {
            default:{}break;
            case R_ChannelSizeKind_2: {num_bits += 2;}break;
            case R_ChannelSizeKind_8: {num_bits += 8;}break;
            case R_ChannelSizeKind_10:{num_bits += 10;}break;
            case R_ChannelSizeKind_11:{num_bits += 11;}break;
            case R_ChannelSizeKind_16:{num_bits += 16;}break;
            case R_ChannelSizeKind_24:{num_bits += 24;}break;
            case R_ChannelSizeKind_32:{num_bits += 32;}break;
        }
    }

    u64 num_bits_rounded = num_bits + 7;
    num_bits_rounded -= num_bits_rounded % 8;
    u64 num_bytes = num_bits_rounded / 8;
    return num_bytes;
}

internal Mat4x4f32 r_sample_channel_map_from_tex2dfmt(R_Tex2DFmt fmt)
{
    Mat4x4f32 result = {
        {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1},
        }
    };
    return result;
}

internal Mat4x4f32 r_sample_channel_map_from_tex2dformat(R_Tex2DFormat fmt)
{
    Mat4x4f32 result = {
        {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1},
        }
    };
    switch (fmt)
    {
        default:{}break;
        case R_Tex2DFormat_R8: {
            MemoryZeroArray(result.v[0]);
            result.v[0][0] = result.v[0][1] = result.v[0][2] = result.v[0][3] = 1.f;
        } break;
    }
    return result;
}

//////////////////////////
// Basic Type Functions

internal R_Handle r_handle_zero(void)
{
    R_Handle handle = {0};
    return handle;
}

internal bool32 r_handle_match(R_Handle a, R_Handle b)
{
    return MemoryMatchStruct(&a, &b);
}

//////////////////////////
// Batch Type Functions

internal R_Batch_List r_batch_list_make(u64 instance_size)
{
    R_Batch_List list = {0};
    list.bytes_per_inst = instance_size;
    return list;
}

internal void *r_batch_list_push_inst(Arena *arena, R_Batch_List *list, u64 batch_inst_cap)
{
    void *inst = 0;
    {
        R_Batch_Node *n = list->last;
        if (n == 0 || n->v.byte_count + list->bytes_per_inst > n->v.byte_cap) {
            n = push_array(arena, R_Batch_Node, 1);
            n->v.byte_cap = batch_inst_cap * list->bytes_per_inst;
            n->v.v = push_array_no_zero(arena, u8, n->v.byte_cap);
            SLLQueuePush(list->first, list->last, n);
            list->batch_count += 1;
        }
        inst = n->v.v + n->v.byte_count;
        n->v.byte_count += list->bytes_per_inst;
        list->byte_count += list->bytes_per_inst;
    }
    return inst;
}

/////////////////////////
// Pass Type Functions

internal R_Pass *r_pass_from_kind(Arena *arena, R_Pass_List *list, R_PassKind kind)
{
    R_Pass_Node *n = list->last;
    if (!r_pass_kind_batch_table[kind]) {
        n = 0;
    }
    if (n == 0 || n->v.kind != kind) {
        n = push_array(arena, R_Pass_Node, 1);
        SLLQueuePush(list->first, list->last, n);
        list->count += 1;
        n->v.kind = kind;
        n->v.params = push_array(arena, u8, r_pass_kind_params_size_table[kind]);
    }
    return &n->v;
}
