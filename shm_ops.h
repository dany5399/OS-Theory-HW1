#ifndef SHM_OPS_H
#define SHM_OPS_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "mat_ops.h"

int shm_init(int, int);
int* attach(int, int);
int detach(void*);
int shm_destroy(int);
int** shm_mat_init(int*, int, int);
int** convert_shm_mat(int*, int);
int* copy_to_shm_mat(int**, int*, int);

#endif
