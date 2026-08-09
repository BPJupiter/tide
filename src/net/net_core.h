
#ifndef NET_CORE_H
#define NET_CORE_H

///////////////////////////
// Networking Structures

typedef struct Net_Socket Net_Socket;
struct Net_Socket {
    u64 u64[1];
};

typedef enum Net_AddressType {
    Net_AddressType_ANY,
    Net_AddressType_Ipv4,
    Net_AddressType_Ipv6,
    Net_AddressType_COUNT
} Net_AddressType;

typedef struct Net_Address Net_Address;
struct Net_Address {
    union {
        u128 v6;
        struct {
            u8 _padding[12];
            u32 v4;
        };
    } ip;
    Net_AddressType address_type;
    u16 port;
};

typedef enum Net_TransportProtocol {
    Net_TransportProtocol_RAW,
    Net_TransportProtocol_TCP,
    Net_TransportProtocol_UDP,
    Net_TransportProtocol_COUNT,
} Net_TransportProtocol;

typedef struct Net_Listener Net_Listener;
struct Net_Listener {
    u16 port;
    Net_TransportProtocol protocol;
    Net_Socket socket;
};

typedef struct Net_Client Net_Client;
struct Net_Client {
    Arena *arena;
    Net_TransportProtocol protocol;
    Net_Socket socket;
    Net_Address address;
    bool32 connected;
    Ring *read_buffer;
    Ring *write_buffer;
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

internal bool32  net_str8_to_ipv4(String8 s, u32 *out);
internal bool32  net_str8_to_ipv6(String8 s, u128 *out);
internal String8 net_ipv4_to_str8(Arena *arena, u32 ip);
internal String8 net_ipv6_to_str8(Arena *arena, u128 ip);
internal bool32  net_str8_to_address(String8 s, Net_Address *out);
internal String8 net_address_to_str8(Arena *arena, Net_Address address);

/////////////////////////////////////
// @per_os_impl Networking Primitives

internal Net_Socket net_socket_alloc(Net_TransportProtocol protocol);
internal void       net_socket_release(Net_Socket socket);

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal Net_Listener  net_listener_alloc(Net_TransportProtocol protocol, u16 port);
internal Net_Client    net_listener_accept(Arena *arena, Net_Listener listener);
internal void          net_listener_close(Net_Listener listener);

///////////////////////////////////////////
// @per_os_impl Network Client Functions

internal Net_Client    net_client_alloc(Arena *arena, Net_TransportProtocol protocol);
internal Net_Client    net_client_connect(Net_Client client, Net_Address target);
internal bool32        net_client_pack_raw(Net_Client client, u64 size, void *data);
#define net_client_pack(client, v) net_client_pack_raw((client), (&(v)), sizeof((v)))
internal bool32        net_client_unpack_raw(Net_Client client, u64 size, void *out);
#define net_client_unpack(client, ptr) net_client_unpack_raw((client), (ptr), sizeof(*(ptr)))
internal void          net_client_close(Net_Client client);



#endif // NET_CORE_H
