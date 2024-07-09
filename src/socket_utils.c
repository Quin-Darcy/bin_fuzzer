#include <stdio.h>
#include "socket_utils.h"


int configure_local_address(const char* port, struct addrinfo **bind_address)
{
    printf("[%d][%s][%s] Configuring local address on port %s ...\n", getpid(), __FILE__, __func__, port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(0, port, &hints, bind_address) != 0) {
        fprintf(
            stderr, 
            "[%d][%s][%s] Failed to get address info: (%d).\n",
            getpid(),
            __FILE__,
            __func__,
            errno
        );
    }

    printf("[%d][%s][%s] Successfully configured local address on port %s.\n", getpid(), __FILE__, __func__, port);

    return 0;
}

int create_listener_socket(const struct addrinfo *bind_address, int *listener_socket)
{
    printf("[%d][%s][%s] Creating listener socket ...\n", getpid(), __FILE__, __func__);
    
    *listener_socket = socket(
        bind_address->ai_family,
        bind_address->ai_socktype,
        bind_address->ai_protocol
    );

    if (*listener_socket < 0) {
        fprintf(
            stderr, 
            "[%d][%s][%s] Failed to create listener socket: (%d).\n",
            getpid(),
            __FILE__,
            __func__,
            errno
        );
        return -1;
    }

    printf("[%d][%s][%s] Successfully created listener socket.\n", getpid(), __FILE__, __func__);
    return 0;
}