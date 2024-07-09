#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <sys/types.h>

int create_process(pid_t *pid);
int cleanup_process(const pid_t pid);
int run_bin(const pid_t pid, const char* bin_file);

#endif