#include <stdio.h>
#include <stdlib.h>
#include "mat_ops.h"

void print_mat(int** mat, int size){
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			printf("%d\t", mat[i][j]);
		}
		printf("\n");
	}
}

void free_mat(int** mat, int size){
	for(int i = 0; i < size; i++){
		free(mat[i]);	
	}
	free(mat);
}

int** mat_init(int size, int pad){
	int** mat = malloc(sizeof(int*)*size);
	for(int i=0; i < size; i++){
		mat[i] = malloc(sizeof(int)*size);
		if(pad) memset(mat[i], 0, sizeof(int)*size);
	}
	return mat;
}

int** matmul(int** mat1, int** mat2, int size){
	int** res = mat_init(size, 0);
	for(int i=0; i < size; i++){
		for(int j = 0; j < size; j++){
			res[i][j] = 0;
			for(int k=0; k < size; k++){
				res[i][j] += mat1[i][k] * mat2[k][j];
			}
		}
	}
	return res;
}

void write_check_mat(int** mat1, int** mat2, int size, char* file){
	int** check_mat = matmul(mat1, mat2, size);
	int fd = open(file, O_CREAT | O_RDWR, 0640);

	for(int i = 0; i < size; i++){
		write(fd, (char*)(check_mat[i]), sizeof(int)*size);
	}
	free_mat(check_mat, size);
	close(fd);
}

int** read_check_mat(int size, char* file){
	int** check_mat = mat_init(size, 0);
	int fd = open(file, O_RDWR);

	for(int i=0; i < size; i++){
		read(fd, (char*)(check_mat[i]), sizeof(int)*size);
	}
	close(fd);
	return check_mat;
}

int check_mul(int** check, int** res, int size){
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			if(check[i][j] != res[i][j]) return -1;
		}
	}
	return 0;
}

void generate_sq_matrix(int** mat, int simple, int size){
	
	srand(time(0));
	
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			if(simple) mat[i][j] = (i+1)*(j+1);
			else mat[i][j] = (rand() % 10) + 1;
		}
	}
}


//rq, cq = 0 or 1 depending on quadrant
void _split(int** mat, int** s, int rq, int cq, int size){
	int si = 0;
	for(int i = rq * size; i < size * (rq+1); i++){
		int sj = 0;
		for(int j = cq * size; j < size * (cq+1); j++){
			s[si][sj] = mat[i][j];
			sj++;
		}
		si++;
	}
}

int*** split(int** mat, int size){
	
	int*** splits = malloc(sizeof(int**)*4);
	int g = 0, h = 0;
	for(int k=0; k < 4; k++){
		splits[k] = mat_init(size, 0);
		g = k/2; //needs to flip every other iter. only works because only 4 splits
		_split(mat, splits[k], g, h, size);
		h ^= 1;
	}
	return splits;
}

int** combine(int** q1, int** q2, int** q3, int** q4, int size){
	int** ret = mat_init(size, 0);
	int hsize = size/2;
	int msize = sizeof(int)*hsize;
	for(int i = 0; i < hsize; i++){
		memcpy(ret[i], q1[i], msize);
		memcpy(ret[i] + hsize, q2[i], msize);
		memcpy(ret[i + hsize], q3[i], msize);
		memcpy(ret[i + hsize] + hsize, q4[i], msize);
	}
	return ret;
}

//less malloc calls here
void mat_add_sub(int flip, int** mat1, int** mat2, int** ret, int size){
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			ret[i][j] = mat1[i][j] + ((!flip) ? mat2[i][j] : mat2[i][j] * -1);
		}
	}
}

void copy_mat(int** src, int** dest, int size){

	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			dest[i][j] = src[i][j];
		}
	}
}
