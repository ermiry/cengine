#ifndef _CERVER_CLIENT_CERVER_H_
#define _CERVER_CLIENT_CERVER_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/packets.h"

struct _Packet;

typedef enum ServerType {

    FILE_SERVER = 1,
    WEB_SERVER, 
    GAME_SERVER

} ServerType;

struct _Token;

struct _Cerver {

    bool use_ipv6;  
    Protocol protocol;
    u16 port; 
    String *ip;

    String *name;
    ServerType type;
    bool auth_required;

    bool uses_sessions;
    struct _Token *token;

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
    ServerType type;
    bool auth_required;

    bool uses_sessions;

} SCerver;

#endif