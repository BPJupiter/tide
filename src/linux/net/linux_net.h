
///////////////////////////////////
// NETworking Conversion Helpers

internal void lnx_sockaddr_storage_to_net_address(NET_Address *out, struct sockaddr_storage *in);
internal void lnx_net_address_to_sockaddr_storage(struct sockaddr_storage *out, NET_Address *in);
