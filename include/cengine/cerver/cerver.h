#ifndef _CERVER_CLIENT_CERVER_H_
#define _CERVER_CLIENT_CERVER_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/packets.h"

struct _Packet;

typedef enum CerverType {

    CUSTOM_CERVER = 0,
    FILE_CERVER,
    GAME_CERVER,
    WEB_CERVER, 

} CerverType;

struct _Token;

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