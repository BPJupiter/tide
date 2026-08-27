
#ifndef NET_CORE_H
#define NET_CORE_H

///////////////////////////
// Networking Structures

typedef struct NET_Socket NET_Socket;
struct NET_Socket {
    u64 u64[1];
};

typedef enum NET_AddressFamily {
    NET_AddressFamily_Any,
    NET_AddressFamily_IPv4,
    NET_AddressFamily_IPv6,
    NET_AddressFamily_COUNT
} NET_AddressFamily;

typedef struct NET_Address NET_Address;
struct NET_Address {
    union {
        u128 v6;
        struct {
            u8 _padding[12];
            u32 v4;
        };
    } ip;
    NET_AddressFamily family;
    u16 port;
};

typedef enum NET_TransportProtocol {
    NET_TransportProtocol_RAW,
    NET_TransportProtocol_TCP,
    NET_TransportProtocol_UDP,
    NET_TransportProtocol_COUNT,
} NET_TransportProtocol;

typedef struct NET_Listener NET_Listener;
struct NET_Listener {
    u16 port;
    NET_AddressFamily family;
    NET_TransportProtocol protocol;
    NET_Socket socket;
};

typedef struct NET_Client NET_Client;
struct NET_Client {
    Arena *arena;
    NET_AddressFamily family;
    NET_TransportProtocol protocol;
    NET_Socket socket;
    NET_Address address;
    bool32 connected;
    Ring *recv_buffer;
    Ring *send_buffer;
};


//////////////////////////
// Networking Constants

#define NET_UDP_MTU 1500
#define NET_CLIENT_DEFAULT_BUFFER_SIZE Kilobytes(64)

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

internal bool32  net_str8_to_ipv4(u32 *out, String8 s);
internal bool32  net_str8_to_ipv6(u128 *out, String8 s);
internal String8 net_ipv4_to_str8(Arena *arena, u32 ip);
internal String8 net_ipv6_to_str8(Arena *arena, u128 ip);
internal bool32  net_str8_to_address(NET_Address *out, String8 s);
internal String8 net_address_to_str8(Arena *arena, NET_Address address);

/////////////////////////////////////
// @per_os_impl NETworking Primitives

internal NET_Socket net_socket_alloc(NET_AddressFamily family, NET_TransportProtocol protocol);
internal void       net_socket_release(NET_Socket socket);

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal NET_Listener net_listener_alloc(NET_AddressFamily family, NET_TransportProtocol protocol, u16 port);
internal NET_Client   net_listener_accept(Arena *arena, NET_Listener listener);
internal void         net_listener_close(NET_Listener listener);

///////////////////////////////////////////
// @per_os_impl Network Client Functions

// Note: send and recv operations behave as you would expect with a TCP client,
//       with the connected address stored in the NET_Client structure.
//
//       For UDP, the NET_Address used for sendto and recvfrom functions
//       is bundled within the NET_Client structure and should be written to
//       and from as appropriate when performing UDP operations.

internal NET_Client net_client_alloc(Arena *arena, NET_AddressFamily family, NET_TransportProtocol protocol);
internal NET_Client net_client_connect(NET_Client client, NET_Address target);
internal s64        net_client_send_raw(NET_Client *client, u32 size, void *data);
internal s64        net_client_recv_raw(NET_Client *client, u32 size, void *out);
internal bool32     net_client_send_from_ring(NET_Client *client);
internal bool32     net_client_recv_to_ring(NET_Client *client);
internal void       net_client_close(NET_Client client);



#endif // NET_CORE_H
