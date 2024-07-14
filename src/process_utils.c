#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "process_utils.h"

int create_process(pid_t *pid)
{
    printf("[%d][%s][%s] Creating new process ...\n", 
        getpid(), __FILE__, __func__);

    *pid = fork();
    if (*pid < 0) {
        fprintf(stderr, "[%d][%s][%s] Failed to create child process.\n", 
            getpid(), __FILE__, __func__);
        return -1;
    } else if (*pid > 0) {
        printf("[%d][%s][%s] Created child process (%d).\n", 
            getpid(), __FILE__, __func__, *pid);
    } 
    return 0;
}

int cleanup_process(const pid_t pid)
{
    if (pid <= 0) {
        fprintf(stderr, "[%d][%s][%s] Invalid PID received.\n", 
            getpid(), __FILE__, __func__);
        return -1;
    }

    printf("[%d][%s][%s] Cleaning up process (%d) ...\n", 
        getpid(), __FILE__, __func__, pid);

    int status;
    waitpid(pid, &status, 0);
    printf("[%d][%s][%s] Child process (%d) finished with status %d.\n", 
        getpid(), __FILE__, __func__, pid, status);
    return 0;
}

int run_bin(const pid_t pid, const char* bin_file)
{
    if (pid == 0) {
        printf("[%d][%s][%s] Running %s in process (%d) ...\n", 
            getpid(), __FILE__, __func__, bin_file, getpid());
        const char* args[] = {bin_file, NULL};
        if (execvp(args[0], (char* const*)args) == -1) {
            fprintf(stderr, "[%d][%s][%s] Failed to run execve() with %s: (%d).\n", 
                getpid(), __FILE__, __func__, bin_file, errno);
            return -1;
        }
    }
    return 0;
}