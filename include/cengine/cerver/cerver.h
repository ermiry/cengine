#ifndef _CERVER_CLIENT_CERVER_H_
#define _CERVER_CLIENT_CERVER_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/packets.h"

struct _Cerver;
struct _Packet;
struct _Token;

typedef enum CerverType {

    CUSTOM_CERVER = 0,
    FILE_CERVER,
    GAME_CERVER,
    WEB_CERVER, 

} CerverType;

struct _CerverStats {

    time_t threshold_time;                          // every time we want to reset cerver stats (like packets), defaults 24hrs
    
    u64 client_n_packets_received;                  // packets received from clients
    u64 client_receives_done;                       // receives done to clients
    u64 client_bytes_received;                      // bytes received from clients

    u64 on_hold_n_packets_received;                 // packets received from on hold connections
    u64 on_hold_receives_done;                      // received done to on hold connections
    u64 on_hold_bytes_received;                     // bytes received from on hold connections

    u64 total_n_packets_received;                   // total number of cerver packets received (packet header + data)
    u64 total_n_receives_done;                      // total amount of actual calls to recv ()
    u64 total_bytes_received;                       // total amount of bytes received in the cerver
    
    u64 n_packets_sent;                             // total number of packets that were sent
    u64 total_bytes_sent;                           // total amount of bytes sent by the cerver

    u64 current_active_client_connections;          // all of the current active connections for all current clients
    u64 current_n_connected_clients;                // the current number of clients connected 
    u64 current_n_hold_connections;                 // current numbers of on hold connections (only if the cerver requires authentication)
    u64 total_n_clients;                            // the total amount of clients that were registered to the cerver (no auth required)
    u64 unique_clients;                             // n unique clients connected in a threshold time (check used authentication)
    u64 total_client_connections;                   // the total amount of client connections that have been done to the cerver

    struct _PacketsPerType *received_packets;
    struct _PacketsPerType *sent_packets;

};

typedef struct _CerverStats CerverStats;

extern void cerver_stats_print (struct _Cerver *cerver);

struct _Cerver {

    bool use_ipv6;  
    Protocol protocol;
    u16 port; 
    String *ip;

    String *name;
    CerverType type;
    bool auth_required;

    bool uses_sessions;
    struct _Token *token;

    // this only get updated if we are an admin in the cerver
    CerverStats *stats;

};

typedef struct _Cerver Cerver;

extern Cerver *cerver_new (void);

extern void cerver_delete (void *ptr);

// handles cerver type packets
extern void cerver_packet_handler (struct _Packet *packet);

// serialized cerver structure
typedef struct SCerver {

    bool use_ipv6;  
    Protocol protocol;
    u16 port; 

    char name[32];
    CerverType type;
    bool auth_required;

    bool uses_sessions;

} SCerver;

#endif