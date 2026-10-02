// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

/////////////////////////////
// @per_os_impl Platform Memory Allocation

// basic
internal void *reserve_memory(u64 size);
internal bool32 commit_memory(void *ptr, u64 size);
internal void decommit_memory(void *ptr, u64 size);
internal void release_memory(void *ptr, u64 size);

// memory placeholders
internal bool32 memory_placeholders_supported(void);
internal void  *reserve_memory_placeholders(u64 size, u64 block_size);
internal void   release_memory_placeholders(void *ptr, u64 size, u64 block_size);
internal bool32 unmap_memory_preserve_placeholder(void *ptr, u64 size);
internal bool32 split_memory_placeholder(void *ptr, u64 size, u64 first_size);
internal bool32 coalesce_memory_placeholders(void *ptr, u64 size);
union Rng1u64;
internal void   prefetch_memory_ranges(u64 count, union Rng1u64 *ranges);

// generic memory page fault handler (calls may run concurrently)
typedef bool32 Memory_Read_Fault_Function(void *address, void *user_data);
internal bool32 memory_read_fault_handler_set(Memory_Read_Fault_Function *func, void *user_data);

// large pages
internal void *reserve_memory_large(u64 size);
internal bool32 commit_memory_large(void *ptr, u64 size);

#endif // BASE_MEMORY_H
