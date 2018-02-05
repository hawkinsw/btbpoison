#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

extern int errno;

int main() {
	int child = 0;
	int counter = 5;
	int child_to_parent_pipe[2] = {0,0};
	int parent_to_child_pipe[2] = {0,0};
	int p2c_write, p2c_read, c2p_write, c2p_read;
	char just_read = '\n';
	char to_write = 'W';

	if (pipe(child_to_parent_pipe) || pipe(parent_to_child_pipe)) {
		printf("Error with pipe(): %s\n", strerror(errno));
		return 1;
	}

	c2p_read = child_to_parent_pipe[0];
	c2p_write= child_to_parent_pipe[1];

	p2c_read = parent_to_child_pipe[0];
	p2c_write= parent_to_child_pipe[1];

	if (!(child = fork())) {
		for (int i = 0; i<counter; i++) {
			read(p2c_read, &just_read, 1);
			printf("done with p2c read in child.\n");

			sleep(1);

			write(c2p_write, &to_write, 1);
			printf("done with c2p write in the child.\n");
		}
		return 0;
	}

	for (int i = 0; i<counter; i++) {
		write(p2c_write, &to_write, 1);
		printf("done with p2c write in the parent.\n");
		sleep(1);
		read(c2p_read, &just_read, 1);
		printf("done with c2p read in the parent.\n");
	}

	waitpid(child, NULL, 0);

	return 0;
}
