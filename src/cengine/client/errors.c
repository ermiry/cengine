#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "client/types/types.h"
#include "client/types/string.h"

#include "client/collections/dlist.h"

#include "client/client.h"
#include "client/connection.h"
#include "client/errors.h"
#include "client/packets.h"

#include "client/threads/thread.h"

#include "client/utils/utils.h"
#include "client/utils/log.h"

u8 client_error_unregister (Client *client, const ClientErrorType error_type);

#pragma region data

static ClientErrorData *client_error_data_new (void) {

	ClientErrorData *error_data = (ClientErrorData *) malloc (sizeof (ClientErrorData));
	if (error_data) {
		error_data->client = NULL;
		error_data->connection = NULL;

		error_data->action_args = NULL;

		error_data->error_message = NULL;
	}

	return error_data;

}

void client_error_data_delete (ClientErrorData *error_data) {

	if (error_data) {
		str_delete (error_data->error_message);

		free (error_data);
	}

}

static ClientErrorData *client_error_data_create (
	const Client *client, const Connection *connection, 
	void *args,
	const char *error_message) {

	ClientErrorData *error_data = client_error_data_new ();
	if (error_data) {
		error_data->client = client;
		error_data->connection = connection;

		error_data->action_args = args;

		error_data->error_message = error_message ? str_new (error_message) : NULL;
	}

	return error_data;

}

#pragma endregion

#pragma region errors

static ClientError *client_error_new (void) {

	ClientError *client_error = (ClientError *) malloc (sizeof (ClientError));
	if (client_error) {
		client_error->type = CLIENT_ERROR_NONE;

		client_error->create_thread = false;
		client_error->drop_after_trigger = false;

		client_error->action = NULL;
		client_error->action_args = NULL;
		client_error->delete_action_args = NULL;
	}

	return client_error;

}

static void client_error_delete (void *client_error_ptr) {

	if (client_error_ptr) {
		ClientError *client_error = (ClientError *) client_error_ptr;

		if (client_error->action_args) {
			if (client_error->delete_action_args) 
				client_error->delete_action_args (client_error->action_args);
		}

		free (client_error_ptr);
	}

}

static ClientError *client_error_get (const Client *client, const ClientErrorType error_type, 
    ListElement **le_ptr) {

    if (client) {
        if (client->registered_errors) {
            ClientError *error = NULL;
            for (ListElement *le = dlist_start (client->registered_errors); le; le = le->next) {
                error = (ClientError *) le->data;
                if (error->type == error_type) {
                    if (le_ptr) *le_ptr = le;
                    return error;
                } 
            }
        }
    }

    return NULL;

}

static void client_error_pop (DoubleList *list, ListElement *le) {

    if (le) {
        void *data = dlist_remove_element (list, le);
        if (data) client_error_delete (data);
    }

}

// registers an action to be triggered when the specified error occurs
// if there is an existing action registered to an error, it will be overrided
// a newly allocated ClientErrorData structure will be passed to your method 
// that should be free using the client_error_data_delete () method
// returns 0 on success, 1 on error
u8 client_error_register (Client *client, const ClientErrorType error_type,
	Action action, void *action_args, Action delete_action_args, 
    bool create_thread, bool drop_after_trigger) {

	u8 retval = 1;

	if (client) {
		if (client->registered_errors) {
			ClientError *error = client_error_new ();
			if (error) {
				error->type = error_type;

				error->create_thread = create_thread;
				error->drop_after_trigger = drop_after_trigger;

				error->action = action;
				error->action_args = action_args;
				error->delete_action_args = delete_action_args;

				// search if there is an action already registred for that error and remove it
				(void) client_error_unregister (client, error_type);

				if (!dlist_insert_after (
					client->registered_errors,
					dlist_end (client->registered_errors),
					error
				)) {
					retval = 0;
				}
			}
		}
	}

	return retval;

}

// unregisters the action associated with the error types
// deletes the action args using the delete_action_args () if NOT NULL
// returns 0 on success, 1 on error
u8 client_error_unregister (Client *client, const ClientErrorType error_type) {

	u8 retval = 1;

    if (client) {
        if (client->registered_errors) {
            ClientError *error = NULL;
            for (ListElement *le = dlist_start (client->registered_errors); le; le = le->next) {
                error = (ClientError *) le->data;
                if (error->type == error_type) {
                    client_error_delete (dlist_remove_element (client->registered_errors, le));
					retval = 0;

                    break;
                }
            }
        }
    }

	return retval;

}

// triggers all the actions that are registred to an error
// returns 0 on success, 1 on error
u8 client_error_trigger (const ClientErrorType error_type, 
	const Client *client, const Connection *connection, 
	const char *error_message
) {

	u8 retval = 1;

    if (client) {
        ListElement *le = NULL;
        ClientError *error = client_error_get (client, error_type, &le);
        if (error) {
            // trigger the action
            if (error->action) {
                if (error->create_thread) {
                    pthread_t thread_id = 0;
                    retval = thread_create_detachable (
                        &thread_id,
                        (void *(*)(void *)) error->action, 
                        client_error_data_create (
                            client, connection,
                            error,
							error_message
                        )
                    );
                }

                else {
                    error->action (client_error_data_create (
                        client, connection, 
                        error,
						error_message
                    ));

					retval = 0;
                }
                
                if (error->drop_after_trigger) client_error_pop (client->registered_errors, le);
            }
        }
    }

	return retval;

}

#pragma endregion

#pragma region handler

// handles error packets
void error_packet_handler (Packet *packet) {

	if (packet) {
		if (packet->data_size >= sizeof (SError)) {
			char *end = (char *) packet->data;
			SError *s_error = (SError *) end;

			switch (s_error->error_type) {
				case CLIENT_ERROR_CERVER_ERROR: 
					client_error_trigger (
						CLIENT_ERROR_CERVER_ERROR,
						packet->client, packet->connection,
						s_error->msg
					); 
				break;

				case CLIENT_ERROR_FAILED_AUTH: {
					if (client_error_trigger (
						CLIENT_ERROR_FAILED_AUTH,
						packet->client, packet->connection,
						s_error->msg
					)) {
						// not error action is registered to handle the error
						char *status = c_string_create ("Failed to authenticate - %s", s_error->msg);
						if (status) {
							client_log_error (status);
							free (status);
						}
					}
				} break;

				case CLIENT_ERROR_CREATE_LOBBY:
					client_error_trigger (
						CLIENT_ERROR_CREATE_LOBBY,
						packet->client, packet->connection,
						s_error->msg
					); 
					break;
				case CLIENT_ERROR_JOIN_LOBBY:
					client_error_trigger (
						CLIENT_ERROR_JOIN_LOBBY,
						packet->client, packet->connection,
						s_error->msg
					); 
					break;
				case CLIENT_ERROR_LEAVE_LOBBY: 
					client_error_trigger (
						CLIENT_ERROR_LEAVE_LOBBY,
						packet->client, packet->connection,
						s_error->msg
					); 
					break;
				case CLIENT_ERROR_FIND_LOBBY: 
					client_error_trigger (
						CLIENT_ERROR_FIND_LOBBY,
						packet->client, packet->connection,
						s_error->msg
					); 
					break;

				case CLIENT_ERROR_GAME_INIT: 
					client_error_trigger (
						CLIENT_ERROR_GAME_INIT,
						packet->client, packet->connection,
						s_error->msg
					); 
					break;
				case CLIENT_ERROR_GAME_START: 
					client_error_trigger (
						CLIENT_ERROR_GAME_START,
						packet->client, packet->connection,
						s_error->msg
					); 
					break;

				default: 
					client_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE, "Unknown error received from cerver!"); 
					break;
			}
		}
	}

}

#pragma endregion

#pragma region main

u8 client_errors_init (Client *client) {

    u8 retval = 1;

    if (client) {
        client->registered_errors = dlist_init (client_error_delete, NULL);
        retval = client->registered_errors ? 0 : 1;
    }

    return retval;

}

void client_errors_end (Client *client) { 

    if (client) dlist_delete (client->registered_errors);

}

#pragma endregion