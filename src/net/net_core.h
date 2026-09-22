// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef NET_CORE_H
#define NET_CORE_H

typedef enum NET_Protocol {
    NET_Protocol_TCP,
    NET_Protocol_UDP,
    NET_Protocol_COUNT,
} NET_Protocol;

typedef u8 NET_EndpointKind;
typedef enum NET_EndpointKindEnum { 
    NET_EndpointKind_IPv4,
    NET_EndpointKind_IPv6,
    NET_EndpoitnKind_COUNT
} NET_EndpointKindEnum;

typedef struct NET_Endpoint NET_Endpoint;
struct NET_Endpoint
{
    u16 port;
    NET_EndpointKind kind;
    u8 _pad_0;
    u32 _pad_1;
    u128 address;
};

////////////////////////////////////
// Host <-> Network Byte ordering

#if ARCH_LITTLE_ENDIAN
# define net_to_host_u16(x)  bswap_u16(x)
# define net_to_host_u32(x)  bswap_u32(x)
# define net_to_host_u64(x)  bswap_u64(x)
# define net_to_host_u128(x) bswap_u128(x)
# define host_to_net_u16(x)  bswap_u16(x)
# define host_to_net_u32(x)  bswap_u32(x)
# define host_to_net_u64(x)  bswap_u64(x)
# define host_to_net_u128(x) bswap_u128(x)
#else
# define net_to_host_u16(x)  (x)
# define net_to_host_u32(x)  (x)
# define net_to_host_u64(x)  (x)
# define net_to_host_u128(x) (x)
# define host_to_net_u16(x)  (x)
# define host_to_net_u32(x)  (x)
# define host_to_net_u64(x)  (x)
# define host_to_net_u128(x) (x)
#endif

///////////////////////////////
// String <-> Binary Formats

internal NET_Endpoint net_endpoint_from_string_port(String8 address, u16 port);
internal NET_Endpoint net_endpoint_from_string(String8 address_and_port);
internal String8 net_string_from_endpoint(Arena *arena, NET_Endpoint endpoint);

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal u16 net_listener_alloc(u16 port);
internal u16 net_listener_close(u16 port);

///////////////////////////////
// @per_os_impl Top-Level Layer Calls

#if !defined(NEED_ASYNC)
# define NEED_ASYNC 1
#endif
internal void net_init(void);
internal void net_async_tick(void);

//////////////////////////
// @per_os_impl Sends

internal u64 net_send(u8 *ptr, u64 size, NET_Protocol *protocol_in, NET_Endpoint *entpoint_in, u64 endt_us);
#define net_send_struct(ptr, protocol_in, endpoint_in, endt_us) net_send((ptr), sizeof(*(ptr)), (protocol_in), (endpoint_in), (endt_us))

///////////////////////////
// @per_os_impl Receives

internal u64 net_recv_from_port(u16 port, u8 *ptr, u64 size, NET_Protocol *protocol_out, NET_Endpoint *endpoint_out, u64 endt_us);
#define net_recv_struct_from_port(port, ptr, protocol_out, endpoint_out, endt_us) net_recv((port), (ptr), sizeof(*(ptr)), (protocol_out), (endpoint_out), (endt_us))

#endif // NET_CORE_H
