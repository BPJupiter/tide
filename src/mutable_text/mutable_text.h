// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef MUTABLE_TEXT_H
#define MUTABLE_TEXT_H

////////////////////////////////
//~ rjf: Cache Types

typedef struct MTX_Node MTX_Node;
struct MTX_Node
{
    MTX_Node *next;
    MTX_Node *prev;
    u128 key;
};

typedef struct MTX_Slot MTX_Slot;
struct MTX_Slot
{
    MTX_Node *first;
    MTX_Node *last;
};

typedef struct MTX_Stripe MTX_Stripe;
struct MTX_Stripe
{
    Arena *arena;
    MTX_Node *free_node;
    RWMutex rw_mutex;
};

////////////////////////////////
//~ rjf: Mutation Thread Types

typedef struct MTX_Op MTX_Op;
struct MTX_Op
{
    Rng1u64 range;
    String8 replace;
};

typedef struct MTX_Mut_Thread MTX_Mut_Thread;
struct MTX_Mut_Thread
{
    u64 ring_size;
    u8 *ring_base;
    u64 ring_read_pos;
    u64 ring_write_pos;
    CondVar cv;
    Mutex mutex;
    Thread thread;
};

////////////////////////////////
//~ rjf: Shared State

typedef struct MTX_Shared MTX_Shared;
struct MTX_Shared
{
    Arena *arena;
    
    // rjf: buffer cache
    u64 slots_count;
    u64 stripes_count;
    MTX_Slot *slots;
    MTX_Stripe *stripes;
    
    // rjf: mut threads
    u64 mut_threads_count;
    MTX_Mut_Thread *mut_threads;
};

////////////////////////////////
//~ rjf: Globals

global MTX_Shared *mtx_shared = 0;

////////////////////////////////
//~ rjf: Main Layer Initialization

internal void mtx_init(void);

////////////////////////////////
//~ rjf: Buffer Operations

internal void mtx_push_op(C_Key buffer_key, MTX_Op op);

////////////////////////////////
//~ rjf: Mutation Threads

internal void mtx_enqueue_op(MTX_Mut_Thread *thread, C_Key buffer_key, MTX_Op op);
internal void mtx_dequeue_op(Arena *arena, MTX_Mut_Thread *thread, C_Key *buffer_key_out, MTX_Op *op_out);
internal void mtx_mut_thread__entry_point(void *p);

#endif // MUTABLE_TEXT_H
