#include <stdio.h>
#include <stdlib.h>
#include "sem_ops.h"

typedef struct sembuf sbuf;
sbuf dec;
sbuf inc;

void sembuf_init(int sem_num, int inc_op, int dec_op){
	inc.sem_num = sem_num;
	inc.sem_op = 1;
	inc.sem_flg = 0;
	dec.sem_num = sem_num;
	dec.sem_op = -1;
	dec.sem_flg = 0;
}

//use split_id + global letter to get unique id?
int semaphore_init(int proj, int sem_count, int sem_num, int init_value){
	key_t key = ftok("/tmp", proj);
	int semid = semget(key, sem_count, IPC_CREAT | 0600);
	semctl(semid, sem_num, SETVAL, 1);
	return semid;
}

void semaphore_release(int sem_id, int sem_num){
	semop(sem_id, &inc, sem_num);
}

void semaphore_reserve(int sem_id, int sem_num){
	semop(sem_id, &dec, sem_num);
}

void semaphore_destroy(int sem_id, int sem_num){
	semctl(sem_id, sem_num, IPC_RMID);
}

