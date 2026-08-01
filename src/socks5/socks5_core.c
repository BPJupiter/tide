
///////////////////////////
// Top-Level Layer Calls

internal void socks_init(void)
{
    Arena *arena = arena_alloc();
    socks_state = push_array(arena, Socks_State, 1);
    socks_state->arena = arena;
    socks_state->listener = net_listener_alloc(Net_TransportProtocol_TCP,
                                               1080);
    socks_state->connections = push_array(arena,
                                          Socks_Connection,
                                          INITIAL_CONNECTIONS);
    socks_state->connection_capacity = INITIAL_CONNECTIONS;
    for (u32 idx = 0; idx < socks_state->connection_capacity; idx++) {
        socks_state->connections[idx].arena = arena_alloc();
    }
}

internal void socks_async_tick(void)
{
    // pop connects; fill connection slots;
    if (lane_idx() == 0) {
        for (;;) {
            Temp scratch = scratch_begin(0, 0);

            // find a free connection slot
            u64 free_slot = socks_state->connection_capacity;
            for (u32 idx = 0; idx < socks_state->connection_capacity; idx++) {
                if (!socks_state->active_connections[idx].active) {
                    free_slot = idx;
                    break;
                }
            }

            // do we have to grow our connection capacity?
            if (free_slot == socks_state->connection_capacity) {
                // Our socks_state global only stores itself in memory first,
                // and then our connections exclusively after that.
                // As we're adding nothing else onto this arena,
                // we can just grow and our original pointer
                // can now safely index into the new memory region.
                Socks_Connection *new_connections = push_array(socks_state->arena, Socks_Connection, socks_state->connection_capacity);
                for (u32 idx = 0; idx < socks_state->connection_capacity; idx++) {
                    new_connections[idx].arena = arena_alloc();
                }
                socks_state->connection_capacity *= 2;
            }

            // Fill request slot
            Socks_Connection *conn = 0;
            {
                conn = &socks_state->active_connections[free_slot];
                conn->active = true;
                arena_clear(conn->arena);
                conn->status = Socks_ConnectionStatus_Null;
            }

            {
                bool32 good = false;

                // open connection
                Net_Client client = {0};
                client = net_listener_accept(socks_state->listener, conn->arena);
            }
        }
    }
    lane_sync();

    
    
}
