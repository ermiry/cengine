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

	CLIENT_ERR_NONE                    = 0,

	// internal server error, like no memory
	CLIENT_ERR_CERVER_ERROR            = 1, 

	CLIENT_ERR_FAILED_AUTH             = 2,  

	CLIENT_ERR_CREATE_LOBBY            = 3,
	CLIENT_ERR_JOIN_LOBBY              = 4,
	CLIENT_ERR_LEAVE_LOBBY             = 5,
	CLIENT_ERR_FIND_LOBBY              = 6,

	CLIENT_ERR_GAME_INIT               = 7,
	CLIENT_ERR_GAME_START              = 8,

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
extern u8 client_error_register (struct _Client *client, ClientErrorType error_type,
	Action action, void *action_args, Action delete_action_args, 
    bool create_thread, bool drop_after_trigger);

// unregisters the action associated with the error types
// deletes the action args using the delete_action_args () if NOT NULL
// returns 0 on success, 1 on error
extern u8 client_error_unregister (struct _Client *client, ClientErrorType error_type);

// triggers all the actions that are registred to an error
// returns 0 on success, 1 on error
extern u8 client_error_trigger (ClientErrorType error_type, 
	struct _Client *client, struct _Connection *connection, 
	const char *error_message);

#pragma region data

// structure that is passed to the user registered method
typedef struct ClientErrorData {

    struct _Client *client;
    struct _Connection *connection;

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

// serialized error data
typedef struct SError {

	time_t timestamp;
	u32 error_type;
	char msg[64];

} SError;

#pragma endregion

#endif