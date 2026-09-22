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
    NET_Endpoint endpoint;
    NET_Protocol protocol;
    SOCKET socket;
};

typedef struct W32_NET_Listener W32_NET_Listener;
struct W32_NET_Listener
{
    u16 port;
    Guarded_Ring *s2u_ring;
    SOCKET listen_socket;
};

typedef struct W32_NET_Listener_Node W32_NET_Listener_Node;
struct W32_NET_Listener_Node
{
    W32_NET_Listener_Node *next;
    W32_NET_Listener_Node *prev;
    W32_NET_Listener v;
};

typedef struct W32_NET_Listener_List W32_NET_Listener_List;
{
    W32_NET_Listener_Node *first;
    W32_NET_Listener_Node *last;
    u64 count;
};

typedef struct W32_NET_State W32_NET_State;
struct W32_NET_State
{
    Arena *arena;
    Guarded_Ring *u2s_ring;
    W32_NET_Listener_List tcp_listener_list;
    W32_NET_Listener_List udp_listener_list;
    Thread tcp_listeners_thread;
    Thread udp_listeners_thread;
};

global W32_NET_State *w32_net_state = 0;

////////////////////////////
// Error Printing Helpers

internal void w32_net_print_winsock_error(const char *msg);

/////////////////////////////////////////////
// Networking Conversion Helpers

internal NET_Endpoint w32_net_endpoint_from_sockaddr_storage(SOCKADDR_STORAGE *storage);
internal SOCKADDR_STORAGE w32_net_sockaddr_storage_from_endpoint(NET_Endpoint *ep);

//////////////////////
// Listeners Threads

internal void w32_net_tcp_listeners_thread_entry_point(void *p);
internal void w32_net_udp_listeners_thread_entry_point(void *p);

#endif // WIN32_NET_H
