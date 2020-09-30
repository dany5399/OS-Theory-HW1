#ifndef MAT_OPS_H
#define MAT_OPS_H

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ADD(X, Y, RET, SIZE) mat_add_sub(0, X, Y, RET, SIZE)
#define SUBTRACT(X, Y, RET, SIZE) mat_add_sub(1, X, Y, RET, SIZE)

void print_mat(int**, int);
void free_mat(int**, int);
int** mat_init(int);
int** matmul(int**, int**, int);
int check_mul(int**, int**, int);
void generate_sq_matrix(int**, int, int);
int*** split(int**, int);
int** combine(int**, int**, int**, int**, int);
void mat_add_sub(int, int**, int**, int**, int);
void write_check_mat(int**, int**, int, char*);
int** read_check_mat(int, char*);

#endif
