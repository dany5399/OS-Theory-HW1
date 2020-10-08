#include <stdio.h>
#include <stdlib.h>
#include "sem_ops.h"
#include <time.h>

//use split_id + global letter to get unique id?
int semaphore_init(int proj, int sem_count, int sem_num, int init_value){
	key_t key = ftok(".", proj);
	int semid = semget(key, sem_count, IPC_CREAT | 0600 | IPC_EXCL);
	//keep going until different key
	time_t t;
	srand((unsigned) time(&t));
	while(semid == -1){
	
		int r = rand() % 50;
		semid = semget(ftok("IPC-pipe.c", proj*r), sem_count, IPC_CREAT | 0666 | IPC_EXCL);
	}
	int debug = semctl(semid, sem_num, SETVAL, 1);
	//printf("debug init %d\n", debug);
	return semid;
}

void semaphore_release(int sem_id, int semnum, int nops){
	struct sembuf inc = { .sem_num = semnum, .sem_op = 1, .sem_flg = 0};
	int debug = semop(sem_id, &inc, nops);
	//printf("debug release %d errno %d semid %d nops %d\n", debug, errno, sem_id, nops);
}

void semaphore_reserve(int sem_id, int semnum, int nops){
	struct sembuf dec = { .sem_num = semnum, .sem_op = -1, .sem_flg = 0};
	int debug = semop(sem_id, &dec, nops);
	//printf("debug reserve %d errno %d semid %d nops %d\n", debug, errno, sem_id, nops);
}

void semaphore_destroy(int sem_id, int sem_num){
	semctl(sem_id, sem_num, IPC_RMID);
}

