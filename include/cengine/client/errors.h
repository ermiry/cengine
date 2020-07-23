#ifndef _CLIENT_ERRORS_H_
#define _CLIENT_ERRORS_H_

#include <stdbool.h>

#include "client/types/types.h"
#include "client/types/string.h"

#include "client/packets.h"

struct _Client;
struct _Connection;
struct _Packet;

typedef enum ClientErrorType {

    CLIENT_ERROR_NONE                    = 0,

	CLIENT_ERROR_CERVER_ERROR            = 1, // internal server error, like no memory

	CLIENT_ERROR_FAILED_AUTH             = 2, // we failed to authenticate with the cerver

	CLIENT_ERROR_CREATE_LOBBY            = 3, // failed to create a new game lobby
	CLIENT_ERROR_JOIN_LOBBY              = 4, // a client / player failed to join an existin lobby
	CLIENT_ERROR_LEAVE_LOBBY             = 5, // a player failed to leave from a lobby
	CLIENT_ERROR_FIND_LOBBY              = 6, // failed to find a game lobby for a player

	CLIENT_ERROR_GAME_INIT               = 7, // the game failed to init properly
	CLIENT_ERROR_GAME_START              = 8, // the game failed to start

} ClientErrorType;

typedef struct ClientError {

	ClientErrorType type;
	bool create_thread;                 // create a detachable thread to run action
    bool drop_after_trigger;            // if we only want to trigger the event once

	Action action;                      // the action to be triggered
    void *action_args;                  // the action arguments
    Action delete_action_args;          // how to get rid of the data

} ClientError;

// registers an action to be triggered when the specified error occurs
// if there is an existing action registered to an error, it will be overrided
// a newly allocated ClientErrorData structure will be passed to your method 
// that should be free using the client_error_data_delete () method
// returns 0 on success, 1 on error
extern u8 client_error_register (struct _Client *client, const ClientErrorType error_type,
	Action action, void *action_args, Action delete_action_args, 
    bool create_thread, bool drop_after_trigger);

// unregisters the action associated with the error types
// deletes the action args using the delete_action_args () if NOT NULL
// returns 0 on success, 1 on error
extern u8 client_error_unregister (struct _Client *client, const ClientErrorType error_type);

// triggers all the actions that are registred to an error
// returns 0 on success, 1 on error
extern u8 client_error_trigger (const ClientErrorType error_type, 
	const struct _Client *client, const struct _Connection *connection, 
	const char *error_message
);

#pragma region data

// structure that is passed to the user registered method
typedef struct ClientErrorData {

    const struct _Client *client;
    const struct _Connection *connection;

    void *action_args;                  // the action arguments set by the user

	String *error_message;

} ClientErrorData;

extern void client_error_data_delete (ClientErrorData *error_data);

#pragma endregion

#pragma region handler

// handles error packets
extern void error_packet_handler (struct _Packet *packet);

#pragma endregion

#pragma region main

extern u8 client_errors_init (struct _Client *client);

extern void client_errors_end (struct _Client *client);

#pragma endregion

#pragma region serialization

#define ERROR_MESSAGE_LENGTH        128

// serialized error data
typedef struct SError {

    time_t timestamp;
    u32 error_type;
    char msg[ERROR_MESSAGE_LENGTH];

} SError;

#pragma endregion

#endif