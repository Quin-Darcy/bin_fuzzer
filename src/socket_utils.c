#include <stdio.h>
#include "socket_utils.h"


int get_socket_address(
    const char* address, 
    const char* port, 
    struct addrinfo **socket_address
)
{
    printf(
        "[%d][%s][%s] Resolving socket address of %s:%s ...\n", 
        getpid(), __FILE__, __func__, address, port
    );

    struct addrinfo hints = {
        .ai_family = SOCKET_FAMILY,
        .ai_socktype = SOCKET_TYPE,
        .ai_flags = SOCKET_FLAGS,
        .ai_protocol = SOCKET_PROTOCOL
    };
    
    if (getaddrinfo(address, port, &hints, socket_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to resolve address: (%d).\n",
            getpid(), __FILE__, __func__, errno
        );
    }

    printf("[%d][%s][%s] Resolved address: ", getpid(), __FILE__, __func__);
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo((*socket_address)->ai_addr, (*socket_address)->ai_addrlen,
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST
    );
    printf("%s %s\n", address_buffer, service_buffer);

    return 0;
}

int create_socket(int *new_socket)
{
    printf("[%d][%s][%s] Creating new socket ...\n", 
        getpid(), __FILE__, __func__);
    
    *new_socket = socket(
        SOCKET_FAMILY,
        SOCKET_TYPE,
        SOCKET_PROTOCOL
    );

    if (*new_socket < 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create socket: (%d).\n",
            getpid(), __FILE__, __func__, errno
        );
        return -1;
    }
 
    printf("[%d][%s][%s] Successfully created socket (%d).\n", 
        getpid(), __FILE__, __func__, *new_socket);
    return 0;
}

int bind_socket(const int bound_socket, const struct addrinfo *bind_address)
{
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)bind_address->ai_addr;
    inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, sizeof ipstr);
    int port = ntohs(ipv4->sin_port);

    printf("[%d][%s][%s] Binding socket (%d) to %s:%d ...\n",
        getpid(), __FILE__, __func__, bound_socket, ipstr, port);

    if (bind(bound_socket, bind_address->ai_addr, bind_address->ai_addrlen) == -1) {
        fprintf(stderr, "[%d][%s][%s] Failed to bind socket. (%d)\n",
            getpid(), __FILE__, __func__, errno);
        return -1;
    }

    printf("[%d][%s][%s] Socket (%d) bound to %s:%d.\n", 
        getpid(), __FILE__, __func__, bound_socket, ipstr, port);

    return 0;
}

int start_listening(const int listen_socket)
{
    printf("[%d][%s][%s] Socket (%d) starting listening ...\n",
        getpid(), __FILE__, __func__, listen_socket);

    if (listen(listen_socket, 10) < 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to start listening. (%d)\n",
            getpid(), __FILE__, __func__, errno);
        return -1;
    }

    printf("[%d][%s][%s] Socket (%d) now listening.\n", 
        getpid(), __FILE__, __func__, listen_socket);

    return 0;
}

int connect_socket(const int local_socket, const char* address, const char* port)
{
    printf("[%d][%s][%s] Attempting to connect socket (%d) to  %s:%s ...\n",
        getpid(), __FILE__, __func__, local_socket, address, port);

    struct addrinfo *remote_address;
    if (get_socket_address(address, port, &remote_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to configure remote address (%d)\n", 
            getpid(), __FILE__, __func__, errno);
        return -1;
    }

    if (connect(local_socket, remote_address->ai_addr, remote_address->ai_addrlen) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to connect local socket to remote socket (%d)\n", 
            getpid(), __FILE__, __func__, errno);
        return -1;
    }

    printf("[%d][%s][%s] Socket (%d) to connected to %s:%s ...\n",
        getpid(), __FILE__, __func__, local_socket, address, port);

    return 0;
}

int manage_connections(const int socket_listen)
{
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;

    printf("[%d][%s][%s] Waiting for connections ... \n",
        getpid(), __FILE__, __func__);

    while(1) {
        fd_set reads;
        reads = master;

        if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "[%d][%s][%s] Error: select() failed. (%d)\n", 
                getpid(), __FILE__, __func__, errno);
            return -1;
        }

        int i;
        for (i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {
                if (i == socket_listen) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);

                    int socket_client = accept(socket_listen, 
                                                (struct sockaddr*)&client_address, &client_len);

                    if (socket_client == -1) {
                        fprintf(stderr, "[%d][%s][%s] accept() failed. (%d)\n", 
                            getpid(), __FILE__, __func__, errno);
                        return -1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket) {
                        max_socket = socket_client;
                    }

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address, 
                        client_len, address_buffer, 
                        sizeof(address_buffer), 0, 0, 
                        NI_NUMERICHOST);

                    printf("[%d][%s][%s] New connection from %s\n", 
                        getpid(), __FILE__, __func__, address_buffer);
                }
            }
        }
    }
}