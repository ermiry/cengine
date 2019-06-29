#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/connection.h"
#include "cengine/cerver/packets.h"

#include "cengine/utils/log.h"

static ProtocolID protocol_id = 0;
static ProtocolVersion protocol_version = { 0, 0 };

void packets_set_protocol_id (ProtocolID proto_id) { protocol_id = proto_id; }

void packets_set_protocol_version (ProtocolVersion version) { protocol_version = version; }

static PacketHeader *packet_header_new (PacketType packet_type, size_t packet_size) {

    PacketHeader *header = (PacketHeader *) malloc (sizeof (PacketHeader));
    if (header) {
        memset (header, 0, sizeof (PacketHeader));
        header->protocol_id = protocol_id;
        header->protocol_version = protocol_version;
        header->packet_type = packet_type;
        header->packet_size = packet_size;
    }

    return header;

}

static inline void packet_header_delete (PacketHeader *header) { if (header) free (header); }

Packet *packet_new (void) {

    Packet *packet = (Packet *) malloc (sizeof (Packet));
    if (packet) {
        memset (packet, 0, sizeof (Packet));
        packet->connection = NULL;
        packet->custom_type = NULL;
        packet->data = NULL;
        packet->header = NULL;  
        packet->packet = NULL;
    }

    return packet;

}

void packet_delete (void *ptr) {

    if (ptr) {
        Packet *packet = (Packet *) ptr;

        packet->connection = NULL;
        str_delete (packet->custom_type);
        if (packet->data) free (packet->data);
        packet_header_delete (packet->header);
        if (packet->packet) free (packet->packet);

        free (packet);
    }

}

// FIXME:
// prepares the packet to be ready to be sent
u8 packet_generate (Packet *packet) {

    u8 retval = 0;

    if (packet) {
        packet->header = packet_header_new (packet->packet_type, packet->packet_size);

        // create the packet buffer to be sent
    }

    return retval;

}

// TODO: check for errno appropierly
u8 packet_send_tcp (i32 socket_fd, const void *packet, size_t packet_size, int flags) {

    if (packet) {
        ssize_t sent;
        const void *p = packet;
        while (packet_size > 0) {
            sent = send (socket_fd, p, packet_size, flags);
            if (sent < 0) return 1;
            p += sent;
            packet_size -= sent;
        }

        return 0;
    }

    return 1;

}

// FIXME: correctly send an udp packet!!
u8 packet_send_udp (const void *packet, size_t packet_size) {

    ssize_t sent;
    const void *p = packet;
    // while (packet_size > 0) {
    //     sent = sendto (server->serverSock, begin, packetSize, 0, 
    //         (const struct sockaddr *) &address, sizeof (struct sockaddr_storage));
    //     if (sent <= 0) return -1;
    //     p += sent;
    //     packetSize -= sent;
    // }

    return 0;

}

u8 packet_send (Connection *connection, Packet *packet, int flags) {

    u8 retval = 1;

    if (connection && packet) {
        switch (connection->protocol) {
            case PROTOCOL_TCP: 
                retval = packet_send_tcp (connection->sock_fd, 
                    packet->packet, packet->packet_size, flags);
                break;
            case PROTOCOL_UDP:
                break;

            default: break;
        }
    }

    return retval;

}

// FIXME:
// check for packets with bad size, protocol, version, etc
u8 packet_check (Packet *packet) {

    /*if (packetSize < sizeof (PacketHeader)) {
        #ifdef CLIENT_DEBUG
        cengine_log_msg (stderr, WARNING, NO_TYPE, "Recieved a to small packet!");
        #endif
        return 1;
    } 

    PacketHeader *header = (PacketHeader *) packetData;

    if (header->protocolID != PROTOCOL_ID) {
        #ifdef CLIENT_DEBUG
        logMsg (stdout, WARNING, PACKET, "Packet with unknown protocol ID.");
        #endif
        return 1;
    }

    Version version = header->protocolVersion;
    if (version.major != PROTOCOL_VERSION.major) {
        #ifdef CLIENT_DEBUG
        logMsg (stdout, WARNING, PACKET, "Packet with incompatible version.");
        #endif
        return 1;
    }

    // compare the size we got from recv () against what is the expected packet size
    // that the client created 
    if (packetSize != header->packetSize) {
        #ifdef CLIENT_DEBUG
        logMsg (stdout, WARNING, PACKET, "Recv packet size doesn't match header size.");
        #endif
        return 1;
    } 

    if (expectedType != DONT_CHECK_TYPE) {
        // check if the packet is of the expected type
        if (header->packetType != expectedType) {
            #ifdef CLIENT_DEBUG
            logMsg (stdout, WARNING, PACKET, "Packet doesn't match expected type.");
            #endif
            return 1;
        }
    }

    return 0;   // packet is fine */

}