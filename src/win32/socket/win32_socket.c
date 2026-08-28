// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ rjf: Listener Threads

internal void
w32_sock_tcp_listener_thread_entry_point(void *p)
{
  ThreadNameF("w32_sock_tcp_listener_thread");
  for(;;)
  {
    SOCKET new_socket = accept(w32_sock_state->tcp_listen_socket, 0, 0);
    if(new_socket != INVALID_SOCKET)
    {
      // TODO(rjf)
    }
  }
}

internal void
w32_sock_udp_listener_thread_entry_point(void *p)
{
  ThreadNameF("w32_sock_udp_listener_thread");
  for(;;)
  {
    SOCKET new_socket = accept(w32_sock_state->udp_listen_socket, 0, 0);
    if(new_socket != INVALID_SOCKET)
    {
      // TODO(rjf)
    }
  }
}

////////////////////////////////
//~ rjf: @per_os_impl Top-Level Layer Calls

internal void
sock_init(void)
{
  // NOTE(rjf): winsock2 is already initialized by the base layer for RIO function grabbing.
  
  //- rjf: set up state
  Arena *arena = arena_alloc();
  w32_sock_state = push_array(arena, W32_SOCK_State, 1);
  w32_sock_state->arena = arena;
  w32_sock_state->u2s_ring = guarded_ring_alloc(arena, Kilobytes(256));
  w32_sock_state->s2u_ring = guarded_ring_alloc(arena, Kilobytes(256));
  
  //- rjf: create listener sockets
  w32_sock_state->tcp_listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  w32_sock_state->udp_listen_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  {
    DWORD ipv6only = 0;
    setsockopt(w32_sock_state->tcp_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
    setsockopt(w32_sock_state->udp_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
  }
  
  //- rjf: bind listener sockets
  {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET6;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SOCKET_PORT);
    bind(w32_sock_state->tcp_listen_socket, (SOCKADDR *)&server_addr, sizeof(server_addr));
    bind(w32_sock_state->udp_listen_socket, (SOCKADDR *)&server_addr, sizeof(server_addr));
  }
  
  //- rjf: start listening
  {
    listen(w32_sock_state->tcp_listen_socket, SOMAXCONN);
    listen(w32_sock_state->udp_listen_socket, SOMAXCONN);
  }
  
  //- rjf: launch one-off listener threads to block & accept connections
  w32_sock_state->tcp_listener_thread = thread_launch(w32_sock_tcp_listener_thread_entry_point, 0);
  w32_sock_state->udp_listener_thread = thread_launch(w32_sock_udp_listener_thread_entry_point, 0);
}

internal void
sock_async_tick(void)
{
    // pop sends
    if (lane_idx() == 0)
    {
        Temp scratch = scratch_begin(0, 0);
        
        Ring_Guard guard = guarded_ring_open(w32_sock_state->u2s_ring);
        {
            SOCK_Protocol protocol = {0};
            SOCK_Endpoint endpoint = {0};
            u64 size = 0;
            guarded_ring_Try_read_struct(&guard, &protocol);
            guarded_ring_try_read_struct(&guard, &endpoint);
            guarded_ring_try_read_struct(&guard, &size);
            u8 *data = push_array(scratch.arena, u8, size);
            guarded_ring_try_read(&guard, size, data);
        }
        guarded_ring_close(&guard);
        
        scratch_end(scratch);
    }
}

////////////////////////////////
//~ rjf: @per_os_impl Sends

internal u64
sock_send(u8 *ptr, u64 size, SOCK_Protocol *protocol_in, SOCK_Endpoint *endpoint_in, u64 endt_us)
{
    u64 result = 0;
    u64 needed_size = sizeof(*protocol_in) + sizeof(*endpoint_in) + sizeof(u64) + size;
    Ring_Guard guard = guarded_ring_open(w32_sock_state->u2s_ring);
    {
        void *dst = guarded_ring_push_or_wait(&guard, needed_size, endt_us);
        if(dst != 0)
        {
            MemoryCopy((u8 *)dst + 0, protocol_in, sizeof(*protocol_in));
            MemoryCopy((u8 *)dst + sizeof(*protocol_in), endpoint_in, sizeof(*endpoint_in));
            MemoryCopy((u8 *)dst + sizeof(*protocol_in) + sizeof(*endpoint_in), &size, sizeof(size));
            MemoryCopy((u8 *)dst + sizeof(*protocol_in) + sizeof(*endpoint_in) + sizeof(u64), ptr, size);
            result = needed_size;
        }
    }
    guarded_ring_close(&guard);
    return result;
}

////////////////////////////////
//~ rjf: @per_os_impl Receives

internal u64
sock_recv(u8 *ptr, u64 size, SOCK_Protocol *protocol_out, SOCK_Endpoint *endpoint_out, u64 endt_us)
{
    u64 received_size = 0;
    u64 header_size = sizeof(*protocol_out) + sizeof(*endpoint_out) + sizeof(u64);
    {
        Ring_Guard guard = guarded_ring_open(w32_sock_state->s2u_ring);
        {
            void *src = guarded_ring_pop_or_wait(&guard, header_size, endt_us);
            if(src != 0)
            {
                u64 payload_size = 0;
                MemoryCopy(protocol_out,  (u8 *)src + 0, sizeof(*protocol_out));
                MemoryCopy(endpoint_out,  (u8 *)src + sizeof(*protocol_out), sizeof(*endpoint_out));
                MemoryCopy(&payload_size, (u8 *)src + sizeof(*protocol_out) + sizeof(*endpoint_out), sizeof(u64));
                void *payload = guarded_ring_pop_or_wait(&guard, payload_size, max_u64);
                MemoryCopy(ptr, payload, payload_size);
                received_size = payload_size;
            }
        }
        guarded_ring_close(&guard);
    }
    return received_size;
}
