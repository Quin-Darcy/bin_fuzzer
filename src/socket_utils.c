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
    if (listen(listen_socket, 10) < 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to start listening. (%d)\n",
            getpid(), __FILE__, __func__, errno);
        return -1;
    }

    printf("[%d][%s][%s] Socket (%d) listening ... \n", 
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
                    
                    char host[NI_MAXHOST], service[NI_MAXSERV];
                    int result = getnameinfo((struct sockaddr*)&client_address, client_len,
                                            host, sizeof(host),
                                            service, sizeof(service),
                                            NI_NUMERICHOST | NI_NUMERICSERV);

                    if (result == 0) {
                        printf("[%d][%s][%s] New connection from %s:%s\n", 
                            getpid(), __FILE__, __func__, host, service);
                    } else {
                        fprintf(stderr, "[%d][%s][%s] getnameinfo() failed: %s\n", 
                            getpid(), __FILE__, __func__, gai_strerror(result));
                    }
                }
            }
        }
    }
}

int start_server(const char* local_address, const char* local_port, int *listener_socket)
{
    printf("[%d][%s][%s] Starting server at %s:%s ...\n",
        getpid(), __FILE__, __func__, local_address, local_port);
    int ret = -1;

    // Resolve and fetch socket address for server socket
    struct addrinfo *listener_socket_address;
    if (get_socket_address(local_address, local_port, &listener_socket_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to configure local address.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Create a socket and bind it to the fetched address
    if (create_socket(listener_socket) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create listener socket.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Bind the socket to the local address
    if (bind_socket(*listener_socket, listener_socket_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to bind socket.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Start the socket listening
    if (start_listening(*listener_socket) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to start listening.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Manage connections in loop
    if (manage_connections(*listener_socket) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to manage connections.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    ret = 0;

cleanup:
    if (listener_socket_address != NULL) {
        freeaddrinfo(listener_socket_address);
    }

    if (*listener_socket > 0) {
        close(*listener_socket);
    } 

    return ret;
}

int configure_io(int socket_fd) {
    printf("[%d][%s][%s] Redirecting IO to socket (%d) ...\n",
        getpid(), __FILE__, __func__, socket_fd);

    // Redirect stdout to socket
    if (dup2(socket_fd, STDOUT_FILENO) == -1) {
        fprintf(stderr, "[%d][%s][%s] Failed to redirect stdout to socket %d. Error: %s\n", 
            getpid(), __FILE__, __func__, socket_fd, strerror(errno));
        return -1;
    }

    // Redirect stdin from socket
    if (dup2(socket_fd, STDIN_FILENO) == -1) {
        fprintf(stderr, "[%d][%s][%s] Failed to redirect stdin from socket %d. Error: %s\n", 
            getpid(), __FILE__, __func__, socket_fd, strerror(errno));
        return -1;
    }

    // Close the original socket descriptor to avoid descriptor leakage
    // This does not affect the redirection as the descriptors are already duplicated
    if (close(socket_fd) == -1) {
        fprintf(stderr, "[%d][%s][%s] Failed to close original socket descriptor %d. Error: %s\n", 
            getpid(), __FILE__, __func__, socket_fd, strerror(errno));
        // Continue since the IO redirection was successful and this is not a critical error
    }

    printf("[%d][%s][%s] Successfully redirected IO to and from the socket.\n",
        getpid(), __FILE__, __func__);

    return 0;
}

int start_client(const char* remote_address, const char* remote_port)
{
    int ret = -1;
    printf("[%d][%s][%s] Creating client to connect to %s:%s ...\n",
        getpid(), __FILE__, __func__, remote_address, remote_port);

    int client_socket;
    int attempts = 0;
    int connected = 0;

    while (attempts < 5 && !connected) {
        if (create_socket(&client_socket) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to create client socket.\n", 
                getpid(), __FILE__, __func__);
            
            attempts++;
            sleep(1);
            continue;
        }

        if (connect_socket(client_socket, remote_address, remote_port) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to create connect client socket.\n", 
                getpid(), __FILE__, __func__);
            
            if (client_socket > 0) {
                close(client_socket);
            }

            attempts++;
            sleep(1);
            continue;
        } 

        connected = 1;
    }

    // If we exited the retry loop and never connected, return an error
    if (connected == 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to connect client to remote socket.\n",
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Loop for sending and receiving data
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  // Listen to stdin for anything to send
        FD_SET(client_socket, &read_fds); // Listen to socket for incoming messages

        int maxfd = (STDIN_FILENO > client_socket ? STDIN_FILENO : client_socket) + 1;
        int activity = select(maxfd, &read_fds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            break; 
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char buf[1024];
            ssize_t nbytes = read(STDIN_FILENO, buf, sizeof(buf));
            if (nbytes > 0) {
                // Send data read from stdin to socket
                send(client_socket, buf, nbytes, 0);
            }
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            char buf[1024];
            ssize_t nbytes = recv(client_socket, buf, sizeof(buf), 0);
            if (nbytes > 0) {
                // Output data received from socket
                write(STDOUT_FILENO, buf, nbytes);
            } else if (nbytes == 0) {
                // Connection closed
                printf("Connection closed by server.\n");
                break;
            } else {
                perror("Recv error");
                break;
            }
        }
    }
    ret = 0;

cleanup:
    if (client_socket > 0) {
        close(client_socket);
    }
    return ret;
}