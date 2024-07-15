#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

int create_process(pid_t *pid);
int cleanup_process(const pid_t pid);
int run_bin(const pid_t pid, const char* bin_file);

#endif