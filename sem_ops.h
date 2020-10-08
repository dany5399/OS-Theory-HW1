#ifndef SEM_OPS_H
#define SEM_OPS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
typedef struct sembuf sbuf;

int semaphore_init(int, int, int, int);
void semaphore_release(int, int, int);
void semaphore_reserve(int, int, int);
void semaphore_destroy(int, int);

#endif
