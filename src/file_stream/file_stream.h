// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef FILE_STREAM_H
#define FILE_STREAM_H

////////////////
// Path Cache

typedef struct FS_Node FS_Node;
struct FS_Node {
    FS_Node *next;
    String8 path;
    u64 gen;
    u64 last_modified_timestamp;
    u64 size;
};

typedef struct FS_Slot FS_Slot;
struct FS_Slot {
    FS_Node *first;
    FS_Node *last;
};

/////////////////////////
// Shared State Bundle

typedef struct FS_Shared FS_Shared;
struct FS_Shared {
    Arena *arena;
    u64 change_gen;
    u64 slots_count;
    FS_Slot *slots;
    Stripe_Array stripes;
};

/////////////
// Globals

global FS_Shared *fs_shared = 0;

///////////////////
// Top-Level API

internal void fs_init(void);

///////////////////////
// Change Generation

internal u64 fs_change_gen(void);

//////////////////////////////////////////
// Artifact Cache Hooks / Accessing API

internal AC_Artifact fs_artifact_create(String8 key, bool32 *cancel_signal, AC_Status *status_out, u64 *gen_out);
internal void fs_artifact_destroy(AC_Artifact artifact);

internal C_Key fs_key_from_path_range(String8 path, Rng1u64 range, u64 endt_us);
internal u128 fs_hash_from_path_range(String8 path, Rng1u64 range, u64 endt_us);
#define fs_key_from_path(path, endt_us) fs_key_from_path_range((path), r1u64(0, max_u64), (endt_us))
#define fs_hash_from_path(path, endt_us) fs_hash_from_path_range((path), r1u64(0, max_u64), (endt_us))

///////////////////////
// Asynchronous Tick

#if !defined(NEED_ASYNC)
# define NEED_ASYNC 1
#endif
internal void fs_async_tick(void);

#endif // FILE_STREAM_H
