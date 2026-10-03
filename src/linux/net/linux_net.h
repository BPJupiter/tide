// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef LINUX_NET_H
#define LINUX_NET_H

////////////////
// Includes

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

////////////////////
// Implementation Types

typedef struct LNX_NET_Connection LNX_NET_Connection;
struct LNX_NET_Connection
{
    LNX_NET_Connection *next;
    LNX_NET_Connection *prev;
    NET_Endpoint endpoint;
    NET_Protocol protocol;
    int socket;
};


typedef struct LNX_NET_Connection_Slot LNX_NET_Connection_Slot;
struct LNX_NET_Connection_Slot
{
    LNX_NET_Connection *first;
    LNX_NET_Connection *last;
};

typedef struct LNX_NET_Session LNX_NET_Session;
struct LNX_NET_Session
{
    LNX_NET_Session *next;
    LNX_NET_Session *prev;
    Arena *arena;
    Guarded_Ring *u2s_ring;
    Guarded_Ring *s2u_ring;
    int epoll_fd;
    int tcp_listen_socket;
    u64 connection_slots_count;
    Stripe_Array connection_stripes;
    LNX_NET_Connection_Slot *connection_slots;
    Thread listener_thread;
};

typedef struct LNX_NET_State LNX_NET_State;
struct LNX_NET_State
{
    Arena *arena;
    RWMutex session_rw_mutex;
    LNX_NET_Session *first_session;
    LNX_NET_Session *last_session;
};

////////////////
// Globals

global LNX_NET_State *lnx_net_state = 0;

///////////////////////
// Conversion Helpers

internal bool32 lnx_net_endpoint_is_mapped_v4(NET_Endpoint ep);
internal struct sockaddr_storage lnx_net_sockaddr_storage_from_endpoint(NET_Endpoint ep);
internal NET_Endpoint lnx_net_endpoint_from_sockaddr_storage(struct sockaddr_storage storage);

///////////////////////
// Listener Thread

internal void lnx_net_listener_thread_entry_point(void *p);

#endif // LINUX_NET_H
