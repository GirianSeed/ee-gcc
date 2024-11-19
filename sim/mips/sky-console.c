#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

pid_t
fork_terminal(FILE**out, FILE**in, char* title, char* cmd) {
    /* Create 2 pipes.  Pipe 0 will written from parent, read from child. */
    /*                  Pipe 1 will read from parent, written from child. */

    pid_t child;
    int pipe0[2], pipe1[2];
    int parent_write_fd, parent_read_fd, child_write_fd, child_read_fd;

    pipe(pipe0); pipe(pipe1);

    child_read_fd   = pipe0[0];
    parent_write_fd = pipe0[1];
    parent_read_fd  = pipe1[0];
    child_write_fd  = pipe1[1];
    
    if ((child = fork()) == 0) {
	/* child */
	char buf0[10], buf1[10];

	close(parent_read_fd);
	close(parent_write_fd);

	sprintf(buf0, "%d", child_read_fd);
	sprintf(buf1, "%d", child_write_fd);
	execlp("xterm", "xterm", "-T", title, "-e", cmd, buf0, buf1, 0);
    }

    close(child_write_fd); 
    close(child_read_fd);

    *in = fdopen(parent_read_fd, "r");
    *out = fdopen(parent_write_fd, "w");

    setbuf(*in, 0);
    setbuf(*out, 0);

    return child;
}
