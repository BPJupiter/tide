// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef WIN32_NET_H
#define WIN32_NET_H

#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")

typedef struct W32_NET_Connection W32_NET_Connection;
struct W32_NET_Connection
{
    W32_NET_Connection *next;
    W32_NET_Connection *prev;
    NET_Endpoint endpoint;
    NET_Protocol protocol;
    SOCKET socket;
    u8 recv_buffer[4096];
    DWORD recv_size;
    OVERLAPPED recv_overlapped;
};

typedef struct W32_NET_Connection_Slot W32_NET_Connection_Slot;
struct W32_NET_Connection_Slot
{
    W32_NET_Connection *first;
    W32_NET_Connection *last;
};

typedef struct W32_NET_Session W32_NET_Session;
struct W32_NET_Session
{
    W32_NET_Session *next;
    W32_NET_Session *prev;
    Arena *arena;
    Guarded_Ring *u2s_ring;
    Guarded_Ring *s2u_ring;
    HANDLE iocp;
    SOCKET tcp_listen_socket;
    SOCKET tcp_accept_socket;
    OVERLAPPED tcp_accept_overlapped;
    u8 tcp_accept_buffer[4096];
    DWORD tcp_accept_size_out;
    Thread tcp_listener_thread;
    u64 connection_slots_count;
    Stripe_Array connection_stripes;
    W32_NET_Connection_Slot *connection_slots;
    LPFN_ACCEPTEX lpfnAcceptEx;
    NET_Wakeup_Function_Type *wakeup_hook;
};

typedef struct W32_NET_State W32_NET_State;
struct W32_NET_State
{
    Arena *arena;
    RWMutex session_rw_mutex;
    W32_NET_Session *first_session;
    W32_NET_Session *last_session;
};

global W32_NET_State *w32_net_state = 0;

//////////////////////
// Conversion Helpers

internal bool32 w32_net_endpoint_is_mapped_v4(NET_Endpoint ep);
internal struct sockaddr_storage w32_net_sockaddr_storage_from_endpoint(NET_Endpoint ep);
internal NET_Endpoint w32_net_endpoint_from_sockaddr_storage(struct sockaddr_storage storage);


//////////////////////
// Listeners Threads

internal void w32_net_listener_thread_entry_point(void *p);

#endif // WIN32_NET_H
