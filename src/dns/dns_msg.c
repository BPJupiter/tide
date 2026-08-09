
/////////////
// Globals

// dns_id_func() by default returns a 16-bit random number to be used as a message id.
// the number is planned to be drawn from a cryptographically secure random number
// generator, but for now just returns a static value.
// This being a variable the function can be reassigned to a custom function.
// For instance, to make it return a static value for testing.

internal u16 dns_id_func_default(void);
internal u16 (*dns_id_func)(void) = dns_id_func_default;

internal u16 dns_id_func_default(void)
{
    return 0xCAFE;
}

Dns_Msg *dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type)
{
    Dns_Msg *msg = push_array(arena, Dns_Msg, 1);
    msg->header.id = dns_id_func();
    msg->header.recursion_desired = true;
    msg->question.name = str8_to_fqdn(arena, domain);
    msg->question.class = Dns_Class_INET;
    msg->question.type = type;
    return msg;
}
