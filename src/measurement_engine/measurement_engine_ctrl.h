#ifndef MEASUREMENT_ENGINE_CTRL_H
#define MEASUREMENT_ENGINE_CTRL_H

/////////////////////
// Traceroute Cache

// NOTE(fbt): this is currently a preliminary brainstorm.
// the actual layout for traceroute cache can take a number
// of different patterns, so I'm putting of designing the structure
// of this until I know what my access pattern is going to be.

typedef struct M_Traceroute_Data M_Traceroute_Data;
struct M_Traceroute_Data
{
    // links
    M_Traceroute_Data *next;
    M_Traceroute_Data *prev;

    // measurement info
    f32 rtt;
    Date_Time timestamp;
};

typedef struct M_Traceroute_Hop M_Traceroute_Hop;
struct M_Traceroute_Hop
{
    // links
    M_Traceroute_Hop *first;
    M_Traceroute_Hop *last;
    M_Traceroute_Hop *next;
    M_Traceroute_Hop *prev;

    // hops
    M_Traceroute_Data *data_first;
    M_Traceroute_Data *data_last;

    // network endpoint info
    NET_Endpoint endpoint;
    String8 domain;
};

/////////////////////
// Main State Types

typedef struct M_Ctrl_State M_Ctrl_State;
struct M_Ctrl_State
{
    M_Traceroute_Hop *root_hop;
};

#endif // MEASUREMENT_ENGINE_CTRL_H
