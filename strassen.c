#include <stdio.h>
#include <stdlib.h>
#include "strassen.h"

//do strassen on given quadrant
//can probably be called for just 1 process benchmark
int** strassen(int** mat1, int** mat2, int size){
	
	if(size == 1){
		int** ret = mat_init(size, 0);
		ret[0][0] = mat1[0][0] * mat2[0][0];	//no point in calling matmul for 1x1
		return ret;
	}

	int hsize = size/2;
	//for optimization i guess
	int*** split1 = split(mat1, hsize); //a=0, b=1, c=2, d=3
	int*** split2 = split(mat2, hsize); //e=0, f=1, g=2, h=3

	SD_SPLITS(hsize, 0);
	
	int** p1, **p2, **p3, **p4, **p5, **p6, **p7;
	GET_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);
	//make more room for q matrices
	CLEAN_SPLITS(split1, split2, hsize);
	CLEAN_SD_SPLITS(hsize);
	
	//combine to quadrants
	int** q1, **q2, **q3, **q4;
	GET_Q_MATS(hsize, 0, q1, q2, q3, q4, p1, p2, p3, p4, p5, p6, p7);

	CLEAN_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);

	int** ret = combine(q1, q2, q3, q4, size);
	
	CLEAN_Q_MATS(q1, q2, q3, q4, hsize);
	
	return ret;
}


