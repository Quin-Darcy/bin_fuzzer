#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SOCKET_FAMILY AF_INET
#define SOCKET_TYPE SOCK_STREAM
#define SOCKET_PROTOCOL IPPROTO_TCP
#define SOCKET_FLAGS AI_PASSIVE

int get_socket_address(const char* address, const char* port, struct addrinfo **socket_address);
int create_socket(int *new_socket);
int bind_socket(const int bound_socket, const struct addrinfo *bind_address);
int start_listening(const int listen_socket);
int connect_socket(const int local_socket, const char* address, const char* port);
int manage_connections(const int socket_listen);
int start_server(const char* local_address, const char* local_port);
int start_client(const char* remote_address, const char* remote_port);

#endif