
#ifndef NET_CORE_H
#define NET_CORE_H

///////////////////////////
// Networking Structures

typedef struct Net_Socket Net_Socket;
struct Net_Socket {
    u64 u64[1];
};

typedef enum Net_AddressType {
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

/////////////////////////////////////
// @per_os_impl Network Primitives

internal Net_Socket net_socket_alloc(Net_TransportProtcol protocol);

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal Net_Listener  net_listener_alloc(Net_TransportProtocol protocol, u16 port);
internal Net_Client    net_listener_accept(Net_Listener listener, Arena *arena);
internal void          net_listener_close(Net_Listener listener);

///////////////////////////////////////////
// @per_os_impl Network Client Functions

internal Net_Client    net_client_connect(Arena *arena, Net_TransportProtocol protocol, Net_Address target);
internal u64           net_client_pack_raw(Net_Client client, void *data, u64 size);
#define net_client_pack(client, v) net_client_pack_raw((client), (&(v)), sizeof((v)))
internal u64           net_client_unpack_raw(Net_Client client, void *out, u64 size);
#define net_client_unpack(client, ptr) net_client_unpack_raw((client), (ptr), sizeof((v)))
internal void          net_client_close(Net_Client client);



#endif // NET_ADDRESS_H
