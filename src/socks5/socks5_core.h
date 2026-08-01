
#ifndef SOCKS5_H
#define SOCKS5_H

// NOTE: ONLY Socks5 is supported.
typedef enum Socks_Version {
    Socks_Version_Reserved = 0x00,
    Socks_Version_4        = 0x04,
    Socks_Version_5        = 0x05,
} Socks_Version;

// rfc 1928: "Compliant implementations MUST support GSSAPI
//            and SHOULD support USERNAME/PASSWORD authentication methods."
// NOTE: 0x03 - 0x7F are IANA assigned.
//       0x80 - 0xFE are reserved for private methods.
typedef enum Socks_AuthMethod {
    Socks_AuthMethod_NoAuth   = 0x00,
    Socks_AuthMethod_GssApi   = 0x01,
    Socks_AuthMethod_UserPass = 0x02,
    Socks_AUthMethod_NoMethod = 0xff,
} Socks_AuthMethod;

typedef enum Socks_Command {
    Socks_Command_Connect      = 0x01,
    Socks_Command_Bind         = 0x02,
    Socks_Command_UdpAssociate = 0x03,
} Socks_Command;

typedef enum Socks_AddressType {
    Socks_AddressType_Ipv4       = 0x01,
    Socks_AddressType_DomainName = 0x03,
    Socks_AddressType_Ipv6       = 0x04,
} Socks_AddressType;

// NODE: 0x09 - 0xFF unassigned.
typedef enum Socks_ReplyCode {
    Socks_ReplyCode_Succeeded,
    Socks_ReplyCode_GeneralFailure,
    Socks_ReplyCode_NotAllowed,
    Socks_ReplyCode_NetworkUnreachable,
    Socks_ReplyCode_HostUnreachable,
    Socks_ReplyCode_ConnectionRefused,
    Socks_ReplyCode_TtlExpired,
    Socks_ReplyCode_UnsupportedCommand,
    Socks_ReplyCode_UnsupportedAddressType,
    Socks_ReplyCode_COUNT
} Socks_ReplyCode;

typedef enum Socks_ConnectionStatus {
    Socks_ConnectionStatus_Null,
    Socks_ConnectionStatus_PendingRecvMethods,
    Socks_ConnectionStatus_RecvdMethods,
    Socks_ConnectionStatus_PendingSendMethod,
    Socks_ConnectionStatus_SentMethod,
    Socks_ConnectionStatus_PendingRecvRequest,
    Socks_ConnectionStatus_RecvdRequest,
    Socks_ConnectionStatus_PendingSendReply,
    Socks_ConnectionStatus_SentReply,
    Socks_ConnectionStatus_Connected,
    Socks_ConnectionStatus_COUNT
} Socks_ConnectionStatus;

typedef struct Socks_Conn_Params Socks_Conn_Params;
struct Socks_Conn_Params {
    u64 id;
    Socks_Command command;
    Socks_AddressType address_type;
    String8 dest_address;
    u16 port;
    String8 auth_username;
    String8 auth_password;
};

typedef struct Socks_Connection Socks_Connection;
struct Socks_Connection {
    bool32 active;
    Arena *arena;

    // request parameters

    // working state
    Socks_ConnectionStatus status;
    Net_Client client;
    Net_Client target;
};

typedef struct Socks_State Socks_State;
struct Socks_State {
    Arena *arena;
    Net_Listener listener;
    Net_Poll_Handle poller;
    Guarded_Ring *ring;
    Socks_Connection *connections;
    u64 connection_capacity;
};

/////////////
// Globals

#define INITIAL_CONNECTIONS 64
global Socks_State *socks_state = 0;

///////////////////////////
// Top-Level Layer Calls

#if !defined(NEED_ASYNC)
# define NEED_ASYNC 1
#endif
internal void socks_init(void);
internal void socks_async_tick(void);

//////////////////////
// Poll for Clients

internal void socks_poll_for_connections(Guarded_Ring *out_ring, );
internal void socks_


#endif // SOCKS5_H
