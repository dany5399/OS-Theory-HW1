#ifndef STRASSEN_H
#define STRASSEN_H

#include "mat_ops.h"
#include "sem_ops.h"

#define A split1[0]
#define B split1[1]
#define C split1[2]
#define D split1[3]
#define E split2[0]
#define F split2[1]
#define G split2[2]
#define H split2[3]

//Realistically if this entire thing was written in macros and no functions
//I could probably save on a ton of malloc/free calls and stack calls from functions
//but debugging is already hard enough.

//I just don't want to retype these for the forked version of strassen

#define CLEAN_SPLITS(SPLIT1, SPLIT2, SIZE)	\
	for(int i=0; i < 4; i++){	\
		free_mat(SPLIT1[i], SIZE);	\
		free_mat(SPLIT2[i], SIZE);	\
	}	\
	free(SPLIT1);	\
	free(SPLIT2);

#define CLEAN_SD_SPLITS(SIZE)	\
	free_mat(F_H, SIZE);	\
	free_mat(AB, SIZE);	\
	free_mat(CD, SIZE);	\
	free_mat(G_E, SIZE);	\
	free_mat(AD, SIZE);		\
	free_mat(EH, SIZE);	\
	free_mat(B_D, SIZE);	\
	free_mat(GH, SIZE);	\
	free_mat(A_C, SIZE);	\
	free_mat(EF, SIZE);

#define CLEAN_Q_MATS(Q1, Q2, Q3, Q4, SIZE)	\
	free_mat(Q1, SIZE);	\
	free_mat(Q2, SIZE);	\
	free_mat(Q3, SIZE);	\
	free_mat(Q4, SIZE);

//only one sd split
#define SD_SPLIT(SIZE, PAD, SPLIT1, SPLIT2, RES, DO_ADD)	\
	RES = mat_init(SIZE, PAD);	\
	if(DO_ADD) ADD(SPLIT1, SPLIT2, RES, SIZE);	\
	else SUBTRACT(SPLIT1, SPLIT2, RES, SIZE);

#define SD_SPLITS(SIZE, PAD)	\
	int** F_H = mat_init(SIZE, PAD); SUBTRACT(F, H, F_H, SIZE);	\
	int** AB = mat_init(SIZE, PAD); ADD(A, B, AB, SIZE);	\
	int** CD = mat_init(SIZE, PAD); ADD(C, D, CD, SIZE);	\
	int** G_E = mat_init(SIZE, PAD); SUBTRACT(G, E, G_E, SIZE);	\
	int** AD = mat_init(SIZE, PAD); ADD(A, D, AD, SIZE);	\
	int** EH = mat_init(SIZE, PAD); ADD(E, H, EH, SIZE);	\
	int** B_D = mat_init(SIZE, PAD); SUBTRACT(B, D, B_D, SIZE);	\
	int** GH = mat_init(SIZE, PAD); ADD(G, H, GH, SIZE);	\
	int** A_C = mat_init(SIZE, PAD); SUBTRACT(A, C, A_C, SIZE);	\
	int** EF = mat_init(SIZE, PAD); ADD(E, F, EF, SIZE);	

#define CLEAN_P_MATS(SIZE, P1, P2, P3, P4, P5, P6, P7)	\
	free_mat(P1, SIZE);	\
	free_mat(P2, SIZE);	\
	free_mat(P3, SIZE);	\
	free_mat(P4, SIZE);	\
	free_mat(P5, SIZE);	\
	free_mat(P6, SIZE);	\
	free_mat(P7, SIZE);	

#define GET_P_MATS(SIZE, P1, P2, P3, P4, P5, P6, P7)	\
	P1 = strassen(A, F_H, SIZE);	\
	P2 = strassen(AB, H, SIZE);	\
	P3 = strassen(CD, E, SIZE);	\
	P4 = strassen(D, G_E, SIZE);	\
	P5 = strassen(AD, EH, SIZE);	\
	P6 = strassen(B_D, GH, SIZE);	\
	P7 = strassen(A_C, EF, SIZE);	

#define GET_Q_MATS(SIZE, PAD, Q1, Q2, Q3, Q4, P1, P2, P3, P4, P5, P6, P7)	\
	Q1 = mat_init(SIZE, PAD);	\
	ADD(P5, P4, Q1, SIZE);	\
	SUBTRACT(Q1, P2, Q1, SIZE);	\
	ADD(Q1, P6, Q1, SIZE);	\
\
	Q2 = mat_init(SIZE, PAD);	\
	ADD(P1, P2, Q2, SIZE);	\
\
	Q3 = mat_init(SIZE, PAD);	\
	ADD(P3, P4, Q3, SIZE);	\
\
	Q4 = mat_init(SIZE, PAD);	\
	ADD(P1, P5, Q4, SIZE);	\
	SUBTRACT(Q4, P3, Q4, SIZE);	\
	SUBTRACT(Q4, P7, Q4, SIZE);

int** strassen(int** mat1, int** mat2, int size);
#endif
