#include <stdlib.h>
#include <stdio.h>
#include "shm_ops.h"
#include <errno.h>

int shm_init(int proj, int size){
	key_t key = ftok(".", proj);
	int debug = shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL);
	time_t t;
	srand((unsigned) time(&t));
	
	while(debug == -1){

		int r = rand() % 50;
		key = ftok("IPC-shmem.c", proj*r);
		debug = shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL);
	}
	//printf("pid %d: key %d errno %d id %d\n", getpid(), key, errno, debug);
	return debug;
}

int* attach(int id, int flag){
	return shmat(id, 0, 0);
}

int detach(void* addr){
	return shmdt(addr);
}

int shm_destroy(int id){
	return shmctl(id, IPC_RMID, NULL);
}

int** shm_mat_init(int* id, int proj, int size){
	*id = shm_init(proj, sizeof(int)*size*size);
	return convert_shm_mat(attach(*id, 0), size);
}

int** convert_shm_mat(int* src, int size){
	int** mat = mat_init(size, 0);
	for(int i = 0; i < size; i++){
		memcpy(mat[i], src+(i*size), sizeof(int)*size);
	}
	return mat;
}

int* copy_to_shm_mat(int** src, int* dest, int size){
	for(int i = 0; i < size; i++){
		memcpy(dest+(i*size), src[i], sizeof(int)*size);
	}
	return dest;
}
