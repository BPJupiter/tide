
////////////////////////////
// Error Printing Helpers

internal void w32_print_winsock_error(const char *msg);

/////////////////////////////////////////////
// NETworking Conversion Helpers

internal void w32_sockaddr_storage_to_net_address(NET_Address *out, SOCKADDR_STORAGE *in);
internal void w32_net_address_to_sockaddr_storage(SOCKADDR_STORAGE *out, NET_Address *in);
