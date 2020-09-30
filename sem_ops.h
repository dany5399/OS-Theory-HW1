#ifndef SEM_OPS_H
#define SEM_OPS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

typedef struct sembuf sbuf;

void sembuf_init(int, int, int);
int semaphore_init(int, int, int, int);
void semaphore_release(int, int);
void semaphore_reserve(int, int);
void semaphore_destroy(int, int);

#endif
