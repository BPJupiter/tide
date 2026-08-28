// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef SOCKET_H
#define SOCKET_H

#if !defined(SOCKET_PORT)
# define SOCKET_PORT 7423
#endif

typedef enum SOCK_Protocol
{
  SOCK_Protocol_TCP,
  SOCK_Protocol_UDP,
}
SOCK_Protocol;

typedef u8 SOCK_EndpointKind;
typedef enum SOCK_EndpointKindEnum
{
  SOCK_EndpointKind_IPv4,
  SOCK_EndpointKind_IPv6,
}
SOCK_EndpointKindEnum;

typedef struct SOCK_Endpoint SOCK_Endpoint;
struct SOCK_Endpoint
{
  u16 port;
  SOCK_EndpointKind kind;
  u8 _pad_0;
  u32 _pad_1;
  union
  {
    u8 address_u8[16];
    u16 address_u16[8];
  };
};

////////////////////////////////
//~ rjf: Helpers

internal SOCK_Endpoint sock_endpoint_from_string_port(String8 address, u16 port);
internal SOCK_Endpoint sock_endpoint_from_string(String8 address_and_port);

////////////////////////////////
//~ rjf: @per_os_impl Top-Level Layer Calls

#if !defined(NEED_ASYNC)
# define NEED_ASYNC 1
#endif
internal void sock_init(void);
internal void sock_async_tick(void);

////////////////////////////////
//~ rjf: @per_os_impl Sends

internal u64 sock_send(u8 *ptr, u64 size, SOCK_Protocol *protocol_in, SOCK_Endpoint *endpoint_in, u64 endt_us);
#define sock_send_struct(ptr, protocol_in, endpoint_in, endt_us) sock_send((ptr), sizeof(*(ptr)), (protocol_in), (endpoint_in), (endt_us))

////////////////////////////////
//~ rjf: @per_os_impl Receives

internal u64 sock_recv(u8 *ptr, u64 size, SOCK_Protocol *protocol_out, SOCK_Endpoint *endpoint_out, u64 endt_us);
#define sock_recv_struct(ptr, protocol_out, endpoint_out, endt_us) sock_recv((ptr), sizeof(*(ptr)), (protocol_out), (endpoint_out), (endt_us))

#endif // SOCKET_H
