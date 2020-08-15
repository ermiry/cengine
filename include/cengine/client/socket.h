#ifndef _CLIENT_SOCKET_H_
#define _CLIENT_SOCKET_H_

#include <pthread.h>

#include "client/config.h"

struct _Socket {

    int sock_fd;

	char *packet_buffer;
	size_t packet_buffer_size;

	pthread_mutex_t *read_mutex;
	pthread_mutex_t *write_mutex;

};

typedef struct _Socket Socket;

CLIENT_PRIVATE void socket_delete (void *socket_ptr);

CLIENT_PRIVATE void *socket_create_empty (void);

CLIENT_PRIVATE Socket *socket_create (int fd);

#endif