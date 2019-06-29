#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/cerver.h"
#include "cengine/cerver/connection.h"

#include "cengine/utils/log.h"

void connection_remove_auth_data (Connection *connection);

Connection *connection_new (void) {

    Connection *connection = (Connection *) malloc (sizeof (Connection));
    if (connection) {
        memset (connection, 0, sizeof (Connection));

        connection->ip = NULL;
        memset (&connection->address, 0, sizeof (struct sockaddr_storage));

        connection->cerver = NULL;

        connection->auth_action = NULL;
        connection->auth_data = NULL;
        connection->destroy_auth_data = NULL;

        connection->success_auth_action = NULL;
        connection->success_auth_args = NULL;
    }

    return connection;

}

void connection_delete (void *ptr) {

    if (ptr) {
        Connection *connection = (Connection *) ptr;

        str_delete (connection->name);
        str_delete (connection->ip);

        cerver_delete (connection->cerver);

        connection_remove_auth_data (connection);

        free (connection);
    }

}

int connection_comparator (void *one, void *two) {

    if (one && two) 
        return str_compare (((Connection *) one)->name, ((Connection *) two)->name);

}

// sets the auth action to be triggered if the server asks for authentication
// also takes a method to free the auth data when the connection is destroyed
void connection_set_auth_action (Connection *connection,
    Action auth_method, void *auth_data, Action destroy_auth_data) {

    if (connection && auth_method) {
        connection->auth_action = auth_method;
        connection->auth_data = auth_data;
        connection->destroy_auth_data = destroy_auth_data;
    }

}

// sets the connection auth data and a method to destroy it once the connection has ended
void connection_set_auth_data (Connection *connection, void *auth_data, Action destroy_auth_data) {

    if (connection && auth_data) {
        connection->auth_data = auth_data;
        connection->destroy_auth_data = destroy_auth_data;
    } 

}

// removes the connection auth data and its destroy method
void connection_remove_auth_data (Connection *connection) {

    if (connection) {
        if (connection->destroy_auth_data) 
            connection->destroy_auth_data (connection->auth_data);
        else free (connection->auth_data);

        connection->destroy_auth_data = NULL;
        connection->auth_data = NULL;
    }

}

// sets the connection success auth action that is executed if the authentication to a server
// was successfull
void connection_set_success_auth (Connection *connection, Action succes_action, void *args) {

    if (connection) {
        connection->success_auth_action = succes_action;
        connection->success_auth_args = args;
    }

}

// try to connect a client to an address (server) with exponential backoff
static u8 connection_try (Connection *connection, const struct sockaddr_storage address) {

    i32 numsec;
    for (numsec = 2; numsec <= CONNECTION_MAX_SLEEP; numsec <<= 1) {
        if (!connect (connection->sock_fd, 
            (const struct sockaddr *) &address, 
            sizeof (struct sockaddr))) 
            return 0;

        if (numsec <= CONNECTION_MAX_SLEEP / 2) sleep (numsec);
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

            default: cengine_log_msg (stderr, ERROR, NO_TYPE, "Unkonw protocol type!"); return 1;
        }

        if (connection->sock_fd > 0) {
            if (connection->async) {
                if (sock_set_blocking (connection->sock_fd, connection->blocking)) {
                    connection->blocking = false;

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
                        addr->sin_addr.s_addr = inet_addr (connection->ip);
                        addr->sin_port = htons (connection->port);
                    }

                    retval = 0;     // connection setup was successfull
                }

                else {
                    #ifdef CLIENT_DEBUG
                    cengine_log_msg (stderr, ERROR, NO_TYPE, 
                        "Failed to set the socket to non blocking mode!");
                    #endif
                    close (connection->sock_fd);
                }
            }
        }

        else cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create new socket!");
    }

    return retval;

}

// creates a new connection that is ready to be started
Connection *connection_create (const char *name,
     const char *ip_address, u16 port, u8 protocol, bool use_ipv6, bool async) {

    Connection *connection = NULL;

    if (ip_address) {
        connection = connection_new ();
        if (connection) {
            connection->name = str_new (name);
            connection->ip = str_new (ip_address);
            connection->port = port;
            connection->protocol = protocol;
            connection->use_ipv6 = use_ipv6;
            connection->async = async;
            connection->blocking = true;

            connection->isConnected = false;

            // set up the new connection to be ready to be started
            if (connection_init (connection)) {
                #ifdef CLIENT_DEBUG
                cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to set up the new connection!");
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

// FIXME:
// ends a connection
// returns 0 on success, 1 on error
int connection_end (Connection *connection) {

    int retval = 1;

    if (connection) {
        close (connection->sock_fd);

        // if (connection->server) {
        //     if (connection->server->ip) free (connection->server->ip);
        //     free (connection->server);
        // }
    }

    return retval;

}