#ifndef SKY_CONSOLE_H_
#define SKY_CONSOLE_H_

#include <unistd.h>
#include <stdio.h>

pid_t fork_terminal(FILE**out, FILE**in, char* title, char* cmd);

#endif
