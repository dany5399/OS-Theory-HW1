all: ipc_pipe ipc_shm caches

caches: 
	gcc -g cache-tlb.c -o caches

ipc_shm:
	gcc -g -std=c99 IPC-shmem.c mat_ops.c shm_ops.c strassen.c -o ipc_shm

ipc_pipe:
	gcc -g -std=c99 -O0 IPC-pipe.c mat_ops.c sem_ops.c strassen.c -o ipc_pipe

clean:
	rm ipc_pipe ipc_shm check_mat.txt caches
