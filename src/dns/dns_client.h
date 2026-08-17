#ifndef DNS_CLIENT_H
#define DNS_CLIENT_H

typedef struct Dns_Client Dns_Client;
struct Dns_Client {
    Net_Client dialer;
};

internal Dns_Client dns_client_alloc(Arena *arena, Net_AddressType type);
internal void       dns_client_release(Dns_Client client);
internal Dns_Msg    dns_client_exchange(Arena *arena, Dns_Client client, Dns_Msg msg, Net_Address address);

#endif // DNS_CLIENT_H
