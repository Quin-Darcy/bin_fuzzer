#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "process_utils.h"
#include "socket_utils.h"


const char* BIN_FILE = "/home/arbegla/projects/C/binary_interactions/inter_process_comms/src/test_binary";
const char* SRV_ADDRESS = "127.0.0.1";
const char* SRV_PORT = "8080";

const size_t NUM_CLIENTS = 1;

int main() 
{
    // Create first child process to run the server socket in
    pid_t pid;
    if (create_process(&pid) != 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create process for server.\n", 
            getpid(), __FILE__, __func__);
        return -1;
    }

    int srv_socket_fd;
    if (pid == 0) {
        // This is the server process
        if (start_server(SRV_ADDRESS, SRV_PORT, &srv_socket_fd) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to start server.\n",
                getpid(), __FILE__, __func__);
            return -1;
        }
    }

    // Wait for server to get ready
    sleep(2);

    size_t i;
    for (i = 0; i < NUM_CLIENTS; i++) {
        // Create new process within which to run client
        if (create_process(&pid) != 0) {
            fprintf(stderr, "[%d][%s][%s] Failed to create process for client.\n", 
                getpid(), __FILE__, __func__);
            continue;
        }

        if (pid == 0) {
            // This is a client process
            if (start_client(SRV_ADDRESS, SRV_PORT) != 0) {
                fprintf(stderr, "[%d][%s][%s] Failed to start client.\n",
                    getpid(), __FILE__, __func__);
                exit(1);
            }
            exit(0);
        }
    }

    // Parent process waits for all child processes to complete
    while ((pid = wait(NULL)) > 0) {
        printf("[%d][%s][%s] Process %d completed.\n",
            getpid(), __FILE__, __func__, pid);
    }
}