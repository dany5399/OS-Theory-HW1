all: ipc_pipe

ipc_pipe:
	gcc -g -std=c99 IPC-pipe.c mat_ops.c sem_ops.c -o ipc_pipe

clean:
	rm ipc_pipe
