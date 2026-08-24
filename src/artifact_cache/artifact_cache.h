// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef ARTIFACT_CACHE_H
#define ARTIFACT_CACHE_H

////////////////////////////////
//~ rjf: Artifact Computation Statuses

typedef enum AC_Status
{
  AC_Status_Good,
  AC_Status_NeedRetry,
  AC_Status_Failed,
}
AC_Status;

////////////////////////////////
//~ rjf: Artifact Handle Type

typedef struct AC_Artifact AC_Artifact;
struct AC_Artifact
{
  u64 u64[4];
};

////////////////////////////////
//~ rjf: Artifact Computation Function Types

typedef AC_Artifact AC_Create_Function_Type(String8 key, bool32 *cancel_signal, AC_Status *status_out, u64 *gen_out);
typedef void AC_Destroy_Function_Type(AC_Artifact artifact);

typedef u32 AC_Flags;
typedef enum AC_FlagsEnum
{
  AC_Flag_WaitForFresh = (1<<0),
  AC_Flag_HighPriority = (1<<1),
  AC_Flag_Wide = (1<<2),
}
AC_FlagsEnum;

typedef struct AC_Artifact_Params AC_Artifact_Params;
struct AC_Artifact_Params
{
  AC_Create_Function_Type *create;
  AC_Destroy_Function_Type *destroy;
  u64 slots_count;
  u64 gen;
  u64 evict_threshold_us;
  bool32 *stale_out;
  AC_Flags flags;
};

////////////////////////////////
//~ rjf: Cache Types

typedef struct AC_Request AC_Request;
struct AC_Request
{
  String8 key;
  u64 gen;
  bool32 *cancel_signal;
  AC_Create_Function_Type *create;
};

typedef struct AC_Request_Node AC_Request_Node;
struct AC_Request_Node
{
  AC_Request_Node *next;
  AC_Request v;
};

typedef struct AC_Node_Ptr AC_Node_Ptr;
struct AC_Node_Ptr
{
  AC_Node_Ptr *next;
  struct AC_Node *node;
};

typedef struct AC_Node AC_Node;
struct AC_Node
{
  AC_Node *next;
  AC_Node *prev;
  
  // rjf: key/gen/value
  String8 key;
  u64 last_requested_gen;
  u64 last_completed_gen;
  AC_Artifact val;
  
  // rjf: metadata
  Access_Pt access_pt;
  u64 working_count;
  u64 completion_count;
  u64 evict_threshold_us;
  u64 last_touched_ac_request_gen;
  bool32 cancelled;
  bool32 other_nodes_depend_on_me;
};

typedef struct AC_Slot AC_Slot;
struct AC_Slot
{
  AC_Node *first;
  AC_Node *last;
};

typedef struct AC_Cache AC_Cache;
struct AC_Cache
{
  // rjf: link / key for cache-cache
  AC_Cache *next;
  AC_Create_Function_Type *create;
  AC_Destroy_Function_Type *destroy;
  
  // rjf: artifact cache
  u64 slots_count;
  AC_Slot *slots;
  Stripe_Array stripes;
  AC_Node_Ptr **stripe_free_node_ptrs;
};

typedef struct AC_Request_Batch AC_Request_Batch;
struct AC_Request_Batch
{
  Mutex mutex;
  Arena *arena;
  AC_Request_Node *first_wide;
  AC_Request_Node *last_wide;
  AC_Request_Node *first_thin;
  AC_Request_Node *last_thin;
  u64 wide_count;
  u64 thin_count;
};

typedef struct AC_TCTX AC_TCTX;
struct AC_TCTX
{
  u64 _unused_;
};

typedef struct AC_Shared AC_Shared;
struct AC_Shared
{
  Arena *arena;
  u64 request_gen;
  
  // rjf: cache cache
  u64 cache_slots_count;
  AC_Cache **cache_slots;
  Stripe_Array cache_stripes;
  
  // rjf: requests
  AC_Request_Batch req_batches[2]; // 0: high priority, 1: low priority
  
  // rjf: cancel thread
  Thread cancel_thread;
  Semaphore cancel_thread_semaphore;
};

////////////////////////////////
//~ rjf: Globals

global AC_Shared *ac_shared = 0;
thread_static AC_TCTX *ac_tctx = 0;

////////////////////////////////
//~ rjf: Layer Initialization

internal void ac_init(void);

////////////////////////////////
//~ rjf: Cache Lookups

internal AC_Artifact ac_artifact_from_key_(Access *access, String8 key, AC_Artifact_Params *params, u64 endt_us);
#define ac_artifact_from_key(access, key, create_fn, destroy_fn, endt_us, ...) ac_artifact_from_key_((access), (key), &(AC_Artifact_Params){.create = (create_fn), .destroy = (destroy_fn), .evict_threshold_us = (2000000), __VA_ARGS__}, (endt_us))

////////////////////////////////
//~ rjf: Asynchronous Tick

#if !defined(NEED_ASYNC)
# define NEED_ASYNC 1
#endif
internal void ac_async_tick(void);

////////////////////////////////
//~ rjf: Cancel Thread

internal void ac_cancel_thread_entry_point(void *p);

#endif // ARTIFACT_CACHE_H
