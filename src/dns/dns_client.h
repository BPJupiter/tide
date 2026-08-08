#ifndef DNS_CLIENT_H
#define DNS_CLIENT_H

typedef struct Dns_Client Dns_Client;
struct Dns_Client {
    Net_Client dialer;
};

internal Dns_Client dns_client_alloc(Arena *arena)
{
    Dns_Client client = {0};
    client.dialer = net_client_alloc(arena, Net_TransportProtocol_UDP);
    return client;
}

internal void dns_client_release(Dns_Client client)
{
    net_client_close(client.dialer);
}

internal Dns_Msg *dns_client_exchange(Dns_Client client, Dns_Msg *msg, Net_TransportProtocol protocol, Net_Address address)
{
    
}

#endif // DNS_CLIENT_H
