#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

#include "cengine/client/network.h"

// enable/disable blocking on a socket
// true on success, false if there was an error
bool sock_set_blocking (int32_t fd, bool isBlocking) {

	bool retval = false;

	if (fd >= 0) {
		int flags = fcntl (fd, F_GETFL, 0);
		if (flags >= 0) {
			// flags = isBlocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);   // original
			flags = isBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);

			retval = (fcntl (fd, F_SETFL, flags) == 0) ? true : false;
		}
	}

	return retval;

}

char *sock_ip_to_string (const struct sockaddr *address) {

	char *ipstr = NULL;

	if (address) {
		ipstr = (char *) calloc (INET6_ADDRSTRLEN, sizeof (char));
		if (ipstr) {
			switch (address->sa_family) {
				case AF_INET:
					inet_ntop (
						AF_INET, 
						&((struct sockaddr_in *) address)->sin_addr,
						ipstr, INET6_ADDRSTRLEN
					);
					break;

				case AF_INET6:
					inet_ntop (
						AF_INET6, 
						&((struct sockaddr_in6 *) address)->sin6_addr,
						ipstr, INET6_ADDRSTRLEN
					);
					break;

				default: {
					free (ipstr);
					ipstr = NULL;
				} break;
			}
		}
	}

	return ipstr;

}

bool sock_ip_equal (const struct sockaddr *a, const struct sockaddr *b) {

	if (a && b) {
		if (a->sa_family != b->sa_family) return false;

		switch (a->sa_family) {
			case AF_INET: {
				struct sockaddr_in *a_in = (struct sockaddr_in *) a;
				struct sockaddr_in *b_in = (struct sockaddr_in *) b;
				return a_in->sin_port == b_in->sin_port
					&& a_in->sin_addr.s_addr == b_in->sin_addr.s_addr;
			}

			case AF_INET6: {
				struct sockaddr_in6 *a_in6 = (struct sockaddr_in6 *) a;
				struct sockaddr_in6 *b_in6 = (struct sockaddr_in6 *) b;
				return a_in6->sin6_port == b_in6->sin6_port
					&& a_in6->sin6_flowinfo == b_in6->sin6_flowinfo
					&& memcmp(a_in6->sin6_addr.s6_addr, b_in6->sin6_addr.s6_addr,
							SOCK_SIZEOF_MEMBER (struct in6_addr, s6_addr)) == 0
					&& a_in6->sin6_scope_id == b_in6->sin6_scope_id;
			}

			default: return false;
		}
	}

	return false;

}

in_port_t sock_ip_port (const struct sockaddr *address) {

	in_port_t retval = 0;

	if (address) {
		switch (address->sa_family) {
			case AF_INET: retval = ntohs (((struct sockaddr_in *) address)->sin_port); break;
			case AF_INET6: retval = ntohs (((struct sockaddr_in6 *) address)->sin6_port); break;
			default: break;
		}
	}

	return retval;

}