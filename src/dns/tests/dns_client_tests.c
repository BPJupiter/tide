
Test(example_client_exchange)
{
    Temp scratch = scratch_begin(0, 0);
    
    Dns_Msg *msg = dns_msg_alloc(scratch, str8_lit("www.example.org"), Dns_Type_A);
    Dns_Client client = dns_client_alloc(scratch);
    Dns_Msg *response = dns_client_exchange(client msg proto addr);
    
    scratch_end(scratch);
}
