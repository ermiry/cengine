#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/cerver.h"
#include "cengine/cerver/client.h"
#include "cengine/cerver/connection.h"
#include "cengine/cerver/handler.h"
#include "cengine/cerver/packets.h"

#include "cengine/threads/thread.h"

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

void connection_remove_auth_data (Connection *connection);

Connection *connection_new (void) {

    Connection *connection = (Connection *) malloc (sizeof (Connection));
    if (connection) {
        memset (connection, 0, sizeof (Connection));

        connection->ip = NULL;
        memset (&connection->address, 0, sizeof (struct sockaddr_storage));

        connection->connected = false;
        connection->max_sleep = CONNECTION_MAX_SLEEP;

        connection->cerver = NULL;

        connection->sock_receive = NULL;

        connection->auth_data = NULL;
        connection->delete_auth_data = NULL;
        connection->auth_packet = NULL;
    }

    return connection;

}

void connection_delete (void *ptr) {

    if (ptr) {
        Connection *connection = (Connection *) ptr;

        str_delete (connection->name);
        str_delete (connection->ip);

        close (connection->sock_fd);

        cerver_delete (connection->cerver);

        sock_receive_delete (connection->sock_receive);

        connection_remove_auth_data (connection);

        free (connection);
    }

}

int connection_comparator (void *one, void *two) {

    return str_compare (((Connection *) one)->name, ((Connection *) two)->name);

}

// sets the connection max sleep (wait time) to try to connect to the cerver
void connection_set_max_sleep (Connection *connection, u32 max_sleep) {

    if (connection) connection->max_sleep = max_sleep;

}

// sets the connection auth data and a method to destroy it once the connection has ended
void connection_set_auth_data (Connection *connection, void *auth_data, size_t auth_data_size, Action delete_auth_data) {

    if (connection && auth_data) {
        connection_remove_auth_data (connection);

        connection->auth_data = auth_data;
        connection->delete_auth_data = delete_auth_data;
    } 

}

// removes the connection auth data and its destroy method
void connection_remove_auth_data (Connection *connection) {

    if (connection) {
        if (connection->delete_auth_data) 
            connection->delete_auth_data (connection->auth_data);
        else free (connection->auth_data);

        connection->delete_auth_data = NULL;
        connection->auth_data = NULL;

        if (connection->auth_packet) {
            packet_delete (connection->auth_packet);
            connection->auth_packet = NULL;
        }
    }

}

// generates the connection auth packet to be send to the server
// this is also generated automatically whenever the cerver ask for authentication
void connection_generate_auth_packet (Connection *connection) {

    if (connection) {
        if (connection->auth_data) {
            connection->auth_packet = packet_generate_request (AUTH_PACKET, CLIENT_AUTH_DATA, 
                connection->auth_data, connection->auth_data_size);
        }
    }

}

// try to connect a client to an address (server) with exponential backoff
static u8 connection_try (Connection *connection, const struct sockaddr_storage address) {

    i32 numsec;
    for (numsec = 2; numsec <= connection->max_sleep; numsec <<= 1) {
        if (!connect (connection->sock_fd, 
            (const struct sockaddr *) &address, 
            sizeof (struct sockaddr))) 
            return 0;

        if (numsec <= connection->max_sleep / 2) sleep (numsec);
    } 

    return 1;

}

// sets up the new connection values
static u8 connection_init (Connection *connection) {

    u8 retval = 1;

    if (connection) {
        // init the new connection socket
        switch (connection->protocol) {
            case IPPROTO_TCP: 
                connection->sock_fd = socket ((connection->use_ipv6 == 1 ? AF_INET6 : AF_INET), SOCK_STREAM, 0);
                break;
            case IPPROTO_UDP:
                connection->sock_fd = socket ((connection->use_ipv6 == 1 ? AF_INET6 : AF_INET), SOCK_DGRAM, 0);
                break;

            default: cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Unkonw protocol type!"); return 1;
        }

        if (connection->sock_fd > 0) {
            // if (connection->async) {
            //     if (sock_set_blocking (connection->sock_fd, connection->blocking)) {
            //         connection->blocking = false;

                    // get the address ready
                    if (connection->use_ipv6) {
                        struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &connection->address;
                        addr->sin6_family = AF_INET6;
                        addr->sin6_addr = in6addr_any;
                        addr->sin6_port = htons (connection->port);
                    } 

                    else {
                        struct sockaddr_in *addr = (struct sockaddr_in *) &connection->address;
                        addr->sin_family = AF_INET;
                        addr->sin_addr.s_addr = inet_addr (connection->ip->str);
                        addr->sin_port = htons (connection->port);
                    }

                    retval = 0;     // connection setup was successfull
                // }

                // else {
                //     #ifdef CLIENT_DEBUG
                //     cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                //         "Failed to set the socket to non blocking mode!");
                //     #endif
                //     close (connection->sock_fd);
            //     }
            // }
        }

        else cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create new socket!");
    }

    return retval;

}

// creates a new connection that is ready to be started
Connection *connection_create (const char *name,
     const char *ip_address, u16 port, Protocol protocol, bool use_ipv6) {

    Connection *connection = NULL;

    if (ip_address) {
        connection = connection_new ();
        if (connection) {
            connection->name = str_new (name);
            connection->ip = str_new (ip_address);
            connection->port = port;
            connection->protocol = protocol;
            connection->use_ipv6 = use_ipv6;

            connection->connected = false;

            connection->sock_receive = sock_receive_new ();

            // set up the new connection to be ready to be started
            if (connection_init (connection)) {
                #ifdef CLIENT_DEBUG
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to set up the new connection!");
                #endif
                connection_delete (connection);
                connection = NULL;
            }
        }
    }

    return connection;

}

// starts a connection -> connects to the specified ip and port
// returns 0 on success, 1 on error
int connection_start (Connection *connection) {

    return (connection ? connection_try (connection, connection->address) : 1);

}

// starts listening and receiving data in the connection sock
void client_connection_update (void *ptr) {

    if (ptr) {
        ClientConnection *cc = (ClientConnection *) ptr;
        thread_set_name (c_string_create ("connection-%s", cc->connection->name->str));

        while (cc->client->running && cc->connection->connected) {
            client_receive (cc->client, cc->connection);
        }

        client_connection_aux_delete (cc);
    }

}

// ends a connection
void connection_end (Client *client, Connection *connection) {

    if (connection) {
        if (connection->connected) {
            // send a close connection packet
            Packet *packet = packet_generate_request (REQUEST_PACKET, CLIENT_CLOSE_CONNECTION, NULL, 0);
            if (packet) {
                packet_set_network_values (packet, client, connection);
                packet_send (packet, 0, NULL);
                packet_delete (packet);
            }

            close (connection->sock_fd);

            connection->connected = false;
        } 
    } 

}