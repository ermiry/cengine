#ifndef _CERVER_PACKETS_H_
#define _CERVER_PACKETS_H_

#include <stdlib.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/connection.h"

typedef u32 ProtocolID;

extern void packets_set_protocol_id (ProtocolID protocol_id);

typedef struct ProtocolVersion {

	u16 major;
	u16 minor;
	
} ProtocolVersion;

extern void packets_set_protocol_version (ProtocolVersion version);

// these indicate what type of packet we are sending/recieving
typedef enum PacketType {

    SERVER_PACKET = 0,
    CLIENT_PACKET,
    ERROR_PACKET,
	REQUEST_PACKET,
    AUTH_PACKET,
    GAME_PACKET,

    APP_ERROR_PACKET,
    APP_PACKET,

    CUSTOM_PACKET = 70,

    TEST_PACKET = 100,
    DONT_CHECK_TYPE,

} PacketType;

typedef struct PacketHeader {

	ProtocolID protocol_id;
	ProtocolVersion protocol_version;
	PacketType packet_type;
    size_t packet_size;

} PacketHeader;

// these indicate the data and more info about the packet type
typedef enum RequestType {

    SERVER_INFO = 0,
    SERVER_TEARDOWN,

    CLIENT_DISCONNET,

    REQ_GET_FILE,
    POST_SEND_FILE,
    
    REQ_AUTH_CLIENT,
    CLIENT_AUTH_DATA,
    SUCCESS_AUTH,

    LOBBY_CREATE,
    LOBBY_JOIN,
    LOBBY_LEAVE,
    LOBBY_UPDATE,
    LOBBY_DESTROY,

    GAME_INIT,      // prepares the game structures
    GAME_START,     // strat running the game
    GAME_INPUT_UPDATE,
    GAME_SEND_MSG,

} RequestType;

typedef struct RequestData {

    RequestType type;

} RequestData;

struct _Packet {

    // the connection the packet belongs to
    Connection *connection;

    PacketType packet_type;
    String *custom_type;

    // serilized data
    size_t data_size;
    void *data;

    // the actual packet to be sent
    PacketHeader *header;
    size_t packet_size;
    void *packet;

};

typedef struct _Packet Packet;

extern Packet *packet_new (void);

extern void packet_delete (void *ptr);

// prepares the packet to be ready to be sent
extern u8 packet_generate (Packet *packet);

// sends a packet using the tcp protocol
extern u8 packet_send_tcp (i32 socket_fd, const void *packet, size_t packet_size, int flags);

// sends a packet using the udp protocol
extern u8 packet_send_udp (const void *packet, size_t packet_size);

// sends a packet in the connection
extern u8 packet_send (Connection *connection, Packet *packet, int flags);

// check for packets with bad size, protocol, version, etc
extern u8 packet_check (Packet *packet);

#endif