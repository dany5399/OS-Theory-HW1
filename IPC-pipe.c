/* CS 519, FALL 2020: HW-1 
 * IPC using pipes to perform matrix multiplication.
 * Feel free to extend or change any code or functions below.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "mat_ops.h"
#include "sem_ops.h"

//Add all your global variables and definitions here.
#define A split1[0]
#define B split1[1]
#define C split1[2]
#define D split1[3]
#define E split2[0]
#define F split2[1]
#define G split2[2]
#define H split2[3]


/* Time function that calculates time between start and end */
double getdeltatimeofday(struct timeval *begin, struct timeval *end)
{
	return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
		(begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}


/* Stats function that prints the time taken and other statistics that you wish
 * to provide.
 */
void print_stats() {



}

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

#define SD_SPLITS(SIZE)	\
	int** F_H = mat_init(SIZE); SUBTRACT(F, H, F_H, SIZE);	\
	int** AB = mat_init(SIZE); ADD(A, B, AB, SIZE);	\
	int** CD = mat_init(SIZE); ADD(C, D, CD, SIZE);	\
	int** G_E = mat_init(SIZE); SUBTRACT(G, E, G_E, SIZE);	\
	int** AD = mat_init(SIZE); ADD(A, D, AD, SIZE);	\
	int** EH = mat_init(SIZE); ADD(E, H, EH, SIZE);	\
	int** B_D = mat_init(SIZE); SUBTRACT(B, D, B_D, SIZE);	\
	int** GH = mat_init(SIZE); ADD(G, H, GH, SIZE);	\
	int** A_C = mat_init(SIZE); SUBTRACT(A, C, A_C, SIZE);	\
	int** EF = mat_init(SIZE); ADD(E, F, EF, SIZE);	

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

#define GET_Q_MATS(SIZE, Q1, Q2, Q3, Q4, P1, P2, P3, P4, P5, P6, P7)	\
	Q1 = mat_init(SIZE);	\
	ADD(P5, P4, Q1, SIZE);	\
	SUBTRACT(Q1, P2, Q1, SIZE);	\
	ADD(Q1, P6, Q1, SIZE);	\
\
	Q2 = mat_init(SIZE);	\
	ADD(P1, P2, Q2, SIZE);	\
\
	Q3 = mat_init(SIZE);	\
	ADD(P3, P4, Q3, SIZE);	\
\
	Q4 = mat_init(SIZE);	\
	ADD(P1, P5, Q4, SIZE);	\
	SUBTRACT(Q4, P3, Q4, SIZE);	\
	SUBTRACT(Q4, P7, Q4, SIZE);


//do strassen on given quadrant
//can probably be called for just 1 process benchmark
int** strassen(int** mat1, int** mat2, int size){
	
	if(size == 1){
		int** ret = mat_init(size);
		ret[0][0] = mat1[0][0] * mat2[0][0];	//no point in calling matmul for 1x1
		return ret;
	}

	int hsize = size/2;
	//for optimization i guess
	int*** split1 = split(mat1, hsize); //a=0, b=1, c=2, d=3
	int*** split2 = split(mat2, hsize); //e=0, f=1, g=2, h=3

	SD_SPLITS(hsize);
	
	int** p1, **p2, **p3, **p4, **p5, **p6, **p7;
	GET_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);

	//make more room for q matrices
	CLEAN_SPLITS(split1, split2, hsize);
	CLEAN_SD_SPLITS(hsize);
	
	//combine to quadrants
	int** q1, **q2, **q3, **q4;
	GET_Q_MATS(hsize, q1, q2, q3, q4, p1, p2, p3, p4, p5, p6, p7);

	CLEAN_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);

	int** ret = combine(q1, q2, q3, q4, size);
	
	CLEAN_Q_MATS(q1, q2, q3, q4, hsize);
	
	return ret;
}

int** read_pipe(int fd, int* p_mat_id){
	//get p.i matrix and matrix size
	int pmat_and_matsize[2];
	read(fd, (char*)pmat_and_matsize, sizeof(int)*2);
	*p_mat_id = pmat_and_matsize[0];
	int matrix_size = pmat_and_matsize[1];
	
	//read in p_matrix
	int** temp_mat = mat_init(matrix_size);
	for(int i = 0; i < matrix_size; i++){
		read(fd, (char*) (temp_mat[i]), sizeof(int)*matrix_size);
	}

	return temp_mat;
}

void write_pipe(int fd, int p_mat_id, int size, int** mat, int sem_id, int sem_num){
	//lock around entire thing to guarantee sending thru the entire matrix, otherwise other child processes
	//will mess up the message transfer. if matrix data size was guaranteed less than 4K, it could be sent atomically in
	//one message but that it isn't guaranteed to be less than 4K (ie. N = 1024, and sizeof(int)*2 just above 4K)
	int split_and_matsize[2] = {p_mat_id, size};

	semaphore_reserve(sem_id, sem_num);
	write(fd, (char*)(split_and_matsize), sizeof(int)*2);	//save on one read and one write per process
	for(int i = 0; i < size; i++){
		write(fd, (char*)(mat[i]), sizeof(int)*size);
	}
	semaphore_release(sem_id, sem_num);
}

//split, fork, stitch
int fork_strassen(int fmats, int** mat1, int** mat2, int*** res, int size, int fork_bound){
	
	int hsize = size/2;
	
	//split matrices before fork so they copy over
	//because i dont feel like piping them to each child
	int*** split1 = split(mat1, hsize);	//a-d
	int*** split2 = split(mat2, hsize);	//e-h

	//free before forking so that mat1 and mat2 don't get
	//copied to children. fmats == 1 only on initial call otherwise we lose
	//SD_MATS when recursively calling strassen
	if(fmats){
		free_mat(mat1, size);
		free_mat(mat2, size);
	}
	
	SD_SPLITS(hsize);

	int fd[2];

	int** p1 = NULL, **p2 = NULL, **p3 = NULL, **p4 = NULL, **p5 = NULL, **p6 = NULL, **p7 = NULL;
	
	//call regular strassens
	if(size <= fork_bound || size == 2){
		GET_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);

		CLEAN_SPLITS(split1, split2, hsize);
		CLEAN_SD_SPLITS(hsize);

		int **q1, **q2, **q3, **q4;
		GET_Q_MATS(hsize, q1, q2, q3, q4, p1, p2, p3, p4, p5, p6, p7);

		CLEAN_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);
		
		(*res) = combine(q1, q2, q3, q4, size);

		CLEAN_Q_MATS(q1, q2, q3, q4, hsize);

		return 0;
	}else{
		
		//sem init here?
		int proj = getpid();
		int semid = semaphore_init(proj, 1, 0, 1);
		//pipe init
		if(pipe(fd) == -1) return -1;
		
		pid_t curr_pid = -1;	//tell apart parent from children
		pid_t children[7];	//gets copied over unfortunately
		int split_id; //need this for knowing which child does which splits

		//parent makes 7 child processes, child processes just skip the rest of the loop
		for(int i=0; i < 7; i++){
			if((children[i] = fork()) == -1) return -1;
			if(children[i] == 0){

				close(fd[0]);	//close read pipe which comes from parent
				curr_pid = children[i];
				split_id = i+1;
				break;
			}
		}
		
		//each child process gets its own strassen call where it forks more if needed
		if(curr_pid == 0){
			switch(split_id){
				case 1: fork_strassen(0, A, F_H, res, hsize, fork_bound); break;
				case 2: fork_strassen(0, AB, H, res, hsize, fork_bound); break;
				case 3: fork_strassen(0, CD, E, res, hsize, fork_bound); break;
				case 4: fork_strassen(0, D, G_E, res, hsize, fork_bound); break;
				case 5: fork_strassen(0, AD, EH, res, hsize, fork_bound); break;
				case 6: fork_strassen(0, B_D, GH, res, hsize, fork_bound); break;
				case 7: fork_strassen(0, A_C, EF, res, hsize, fork_bound); break;
			}

			CLEAN_SPLITS(split1, split2, hsize);
			CLEAN_SD_SPLITS(hsize);

			//child write to pipe that goes to parent
			write_pipe(fd[1], split_id, hsize, *res, semid, 0);	//locking done in function
			close(fd[1]);

			free_mat((*res), hsize);
			exit(0);
		}else{
			
			//parent reads from each child process
			for(int i = 0; i < 7; i++){
				int p_mat_id;
				int** temp_mat = read_pipe(fd[0], &p_mat_id);
				
				switch(p_mat_id){
					case 1: p1 = temp_mat; break;
					case 2: p2 = temp_mat; break;
					case 3: p3 = temp_mat; break;
					case 4: p4 = temp_mat; break;
					case 5: p5 = temp_mat; break;
					case 6: p6 = temp_mat; break;
					case 7: p7 = temp_mat; break;
				}
			}	
			close(fd[0]);	//close reading pipe that comes from child
			close(fd[1]);	//close write end of this pipe which goes to child
			
			CLEAN_SPLITS(split1, split2, hsize);
			CLEAN_SD_SPLITS(hsize);

			int **q1, **q2, **q3, **q4;
			GET_Q_MATS(hsize, q1, q2, q3, q4, p1, p2, p3, p4, p5, p6, p7);

			CLEAN_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);

			//reassemble
			(*res) = combine(q1, q2, q3, q4, size);
			
			CLEAN_Q_MATS(q1, q2, q3, q4, hsize);
			//two thought processes. wait on child here so memory is freed as soon as possible,
			//or wait on children at the end so that parent can go back to reading the pipe asap
			for(int i=0; i < 7; i++){
				waitpid(children[i], NULL, 0);
			}
				

			semaphore_destroy(semid, 0);
		}
	}
	return 0;
}


int main(int argc, char const *argv[])
{

	if(argc < 6){
		printf("Insufficient arguments\n");
		return 0;
	}
	int size = atoi(argv[1]);
	int type = atoi(argv[2]);
	int benchmark = atoi(argv[3]);
	int fork = atoi(argv[4]);
	int fork_bound = atoi(argv[5]);  //the lower limit to when forking should stop and just let one cpu take over the rest

	//init matrices	
	int** mat1 = mat_init(size);
	int** mat2 = mat_init(size);
	
	printf("Generating m1 and m2 of type ");
	if(type == 1) printf("basic\n");
	else printf("random\n");
	//generate values
	generate_sq_matrix(mat1, type, size);
	generate_sq_matrix(mat2, type, size);

	printf("Starting Strassen ");
	if(benchmark == 1) printf("with benchmarking\n");
	else printf("without benchmarking\n");
	
	//write check matrix to file so that it doesn't copy over when doing forking
	//mat1 and mat2 get freed on first iteration of fork_strassen to reduce unncessary copies
	char* file = "check_mat.txt";
	write_check_mat(mat1, mat2, size, file);

	//result matrix
	//init this in strassen so it doesnt unnecessarily copy over
	//an empty array. also this makes it easier to time from main().
	//might cause a bit of latency on initial pipe from first child from time to initialize
	int** res;	
	int check_child_proc;	//if the returning fork strassen is child process then dont print stats
	if(fork){

		//get correct matrix so some space can be saved from freeing
		//mat1 and mat2 in strassen. Unfortunately check_mat gets copied
		//but not much can be done about that.
		
		sembuf_init(0, 1, -1);	//semop sembuf structs

		//1 = child proc
		if((check_child_proc = fork_strassen(1, mat1, mat2, &res, size, fork_bound)) == -1){
			printf("Strassen failed\n");
			return 0;
		}

		printf("Finished IPC Strassen\n");

	}else{
		
		res = strassen(mat1, mat2, size);
		
		printf("Finished basic Strassens\n");
		free_mat(mat1, size);
		free_mat(mat2, size);
	}

	//verify result
	int** check_mat = read_check_mat(size, file);
	printf("Checking Correctness\n");
	if(check_mul(check_mat, res, size) == -1){
		printf("Strassen incorrect\n");
		free_mat(check_mat, size);
		free_mat(res, size);
		return 0;
	}
	printf("Strassen correct\n");
	free_mat(check_mat, size);

	print_stats(); //dont print if child proc
	//if a child process finishes fork_strassen, it needs to clean up res
	//(not really because OS will probably clean up but might as well)
	free_mat(res, size);

	return 0;
}


