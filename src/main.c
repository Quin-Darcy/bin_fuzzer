#include <stdio.h>
#include "process_utils.h"
#include "socket_utils.h"


const char* BIN_FILE = "/home/arbegla/projects/C/binary_interactions/inter_process_comms/src/test_binary";
const char* PORT = "8080";

int main() 
{
    struct addrinfo *bind_address;
    if (configure_local_address(PORT, &bind_address) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to configure local address.\n", getpid(), __FILE__, __func__);
        return -1;
    }

    int listener_socket;
    if (create_listener_socket(bind_address, &listener_socket) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create listener socket.\n", getpid(), __FILE__, __func__);
        return -1;
    }

    pid_t pid;
    if (create_process(&pid) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create process.\n", getpid(), __FILE__, __func__);
        return -1;
    }

    if (run_bin(pid, BIN_FILE) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to run %s.\n", getpid(), __FILE__, __func__, BIN_FILE);
        return -1;
    }

    if (cleanup_process(pid) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to cleanup process (%d).\n", getpid(), __FILE__, __func__, pid);
        return -1;
    }

    return 0;
}