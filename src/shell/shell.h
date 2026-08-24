
#ifndef SHELL_H
#define SHELL_H

///////////////////////////////////
// @per_os_impl Shell Operations

internal void sh_message(bool32 error, String8 title, String8 message);
internal String8 sh_pick_file(Arena *arena, String8 title, String8 initial_path);
internal void sh_show_in_file_browser(String8 path);
internal void sh_open_in_browser(String8 url);
internal bool32 sh_install_or_uninstall_self(bool32 write, bool32 install);

#endif // SHELL_H
