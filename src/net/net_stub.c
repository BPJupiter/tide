// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

internal void net_init(void){}
internal void net_async_tick(void){}
internal NET_Session net_session_open(u16 listener_port, NET_Wakeup_Function_Type *wakeup_hook){NET_Session s = {0}; return s;}
internal void net_session_close(NET_Session session){}
internal bool32 net_send(NET_Session session, NET_Protocol protocol, NET_Endpoint endpoint, String8 data, u64 endt_us){return 0;}
internal bool32 net_recv(Arena *arena, NET_Session session, NET_Protocol *protocol_out, NET_Endpoint *endpoint_out, String8 *data_out, u64 endt_us){return 0;}
