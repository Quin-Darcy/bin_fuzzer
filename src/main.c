#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"
#include "socket_utils.h"


const char* BIN_FILE = "/home/arbegla/projects/C/binary_interactions/inter_process_comms/src/test_binary";
const char* SRV_ADDRESS = "127.0.0.1";
const char* SRV_PORT = "8080";

int main() 
{
    // Resolve and fetch socket address for server socket
    struct addrinfo *listener_socket_address;
    if (get_socket_address(SRV_ADDRESS, SRV_PORT, &listener_socket_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to configure local address.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Create a socket and bind it to the fetched address
    int listener_socket;
    if (create_socket(&listener_socket) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create listener socket.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Bind the socket to the local address
    if (bind_socket(listener_socket, listener_socket_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to bind socket.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    // Start the socket listening
    if (start_listening(listener_socket) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to start listening.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    pid_t pid;
    if (create_process(&pid) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create process.\n", 
            getpid(), __FILE__, __func__);
        goto cleanup;
    }

    int client_socket;
    if (pid == 0) {
        if (create_socket(&client_socket) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to create client socket.\n", 
                getpid(), __FILE__, __func__);
            exit(1);
        }

        if (connect_socket(client_socket, SRV_ADDRESS, SRV_PORT) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to create connect client socket.\n", 
                getpid(), __FILE__, __func__);
            exit(1);
        } 
    } else {
        if (manage_connections(listener_socket) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to manage connections.\n", 
                getpid(), __FILE__, __func__);
            goto cleanup;
        }
    }

    /*
    if (run_bin(pid, BIN_FILE) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to run %s.\n", 
            getpid(), __FILE__, __func__, BIN_FILE);
        goto cleanup;
    }
    */

    if (pid != 0) {
        if (cleanup_process(pid) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to cleanup process (%d).\n", 
                getpid(), __FILE__, __func__, pid);
            goto cleanup;
        }
    }

cleanup:
    if (listener_socket_address != NULL) {
        freeaddrinfo(listener_socket_address);
    }

    if (listener_socket > 0) {
        close(listener_socket);
    }

    if (client_socket > 0) {
        close(client_socket);
    }
}