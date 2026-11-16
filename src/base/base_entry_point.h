// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef BASE_ENTRY_POINT_H
#define BASE_ENTRY_POINT_H

/////////////////
// Process Entry Point Functions
internal void main_thread_base_entry_point(int argc, char **argv);
internal void supplement_thread_base_entry_point(void (*entry_point)(void *params), void *params);
internal u64  update_tick_idx(void);
internal bool32 update(void);
internal void async_thread_entry_point(void *params);

//////////////
// Library Entry/Exit Point Functions
internal void library_base_entry_point_on_load(void);
internal void library_base_exit_point_on_unload(void);

#endif // BASE_ENTRY_POINT_H
