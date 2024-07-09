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

int configure_local_address(const char* port, struct addrinfo **bind_address);
int create_listener_socket(const struct addrinfo *bind_address, int *listener_socket);

#endif