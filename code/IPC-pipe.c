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
#include "strassen.h"

//Add all your global variables and definitions here.

/* Time function that calculates time between start and end */
double getdeltatimeofday(struct timeval *begin, struct timeval *end)
{
	return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
		(begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}


/* Stats function that prints the time taken and other statistics that you wish
 * to provide.
 */
void print_stats(struct timeval* t1, struct timeval* t2, int type, int fork, int size, int fork_bound) {

	printf("\nStats for ");
	if(fork) printf("forked Strassen's.");
	else printf("regular Strassen's." );
	printf("\nType: ");
	if(type) printf("simple\n");
	else printf("random\n");
	printf("Size: %d\n", size);
	int procs = 1;
	while(size > fork_bound){
		size >>= 1;
		procs *= 7;
	}
	printf("Processes: %d\n", procs);
	printf("Time: %f\n", getdeltatimeofday(t1, t2));

}

int** read_pipe(int fd, int* p_mat_id){
	
	int n = 0; //for debugging

	//get p.i matrix and matrix size
	int pmat_and_matsize[2];
	n += read(fd, (char*)pmat_and_matsize, sizeof(int)*2);
	*p_mat_id = pmat_and_matsize[0];
	int matrix_size = pmat_and_matsize[1];
	
	//read in p_matrix
	int** temp_mat = mat_init(matrix_size, 0);
	for(int i = 0; i < matrix_size; i++){
		n += read(fd, (temp_mat[i]), sizeof(int)*matrix_size);
	}
	//print_mat(temp_mat, matrix_size);
	//for debugging
	//printf("read %d at %d split %d with expected size %d", n, getpid(), *p_mat_id, matrix_size);
	//sleep(1);
	return temp_mat;
}

void write_pipe(int fd, int p_mat_id, int size, int** mat, int sem_id){
	//lock around entire thing to guarantee sending thru the entire matrix, otherwise other child processes
	//will mess up the message transfer. if matrix data size was guaranteed less than 4K, it could be sent atomically in
	//one message but that isnt a guarantee
	
	semaphore_reserve(sem_id, 0, 1);
	int n = 0; //for debugging
	int split_and_matsize[2] = {p_mat_id, size};
	write(fd, split_and_matsize, sizeof(int)*2);	//save on one read and one write per process
	for(int i = 0; i < size; i++){
		n += write(fd, mat[i], sizeof(int)*size);
		//printf("split %d size %d errno %d\n", p_mat_id, size, errno);
	}
	//for debugging
	//print_mat(mat, size);
	//printf("wrote %d from %d with expected size %d\n", n/4, getpid(), size);
	semaphore_release(sem_id, 0, 1);
}

//split, fork, stitch
int fork_strassen(int fmats, int** mat1, int** mat2, int*** res, int size, int fork_bound, int div){
	
	//call regular strassens
	if(size <= fork_bound || div == 3){
		(*res) = strassen(mat1, mat2, size);
		return 0;
	}else{
		
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
		
		int fd[2];

		
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
		//it should calculate the extra splits it needs on its own
		if(curr_pid == 0){
			switch(split_id){
				case 1: {
					int** F_H;
					SD_SPLIT(hsize, 0, F, H, F_H, 0);
					fork_strassen(0, A, F_H, res, hsize, fork_bound, div+1); 
					free_mat(F_H, hsize); break;
				}
				case 2: {
					int** AB;
					SD_SPLIT(hsize, 0, A, B, AB, 1);
					fork_strassen(0, AB, H, res, hsize, fork_bound, div+1);
					free_mat(AB, hsize); break;
				}
				case 3: {
					int** CD;
					SD_SPLIT(hsize, 0, C, D, CD, 1);
					fork_strassen(0, CD, E, res, hsize, fork_bound, div+1);
					free_mat(CD, hsize); break;
				}
				case 4: {
					int** G_E;
					SD_SPLIT(hsize, 0, G, E, G_E, 0);
					fork_strassen(0, D, G_E, res, hsize, fork_bound, div+1);
					free_mat(G_E, hsize); break;
				}
				case 5: {
					int** AD, **EH;
					SD_SPLIT(hsize, 0, A, D, AD, 1);
					SD_SPLIT(hsize, 0, E, H, EH, 1);
					fork_strassen(0, AD, EH, res, hsize, fork_bound, div+1);
					free_mat(AD, hsize); free_mat(EH, hsize); break;
				}
				case 6: {
					int** B_D, **GH;
					SD_SPLIT(hsize, 0, B, D, B_D, 0);
					SD_SPLIT(hsize, 0, G, H, GH, 1);
					fork_strassen(0, B_D, GH, res, hsize, fork_bound, div+1);
					free_mat(B_D, hsize); free_mat(GH, hsize); break;
				}
				case 7: {
					int** A_C, **EF;
					SD_SPLIT(hsize, 0, A, C, A_C, 0);
					SD_SPLIT(hsize, 0, E, F, EF, 1);
					fork_strassen(0, A_C, EF, res, hsize, fork_bound, div+1);
					free_mat(A_C, hsize); free_mat(EF, hsize); break;
				}
			}

			//child write to pipe that goes to parent
			write_pipe(fd[1], split_id, hsize, *res, semid);	//locking done in function
			close(fd[1]);

			CLEAN_SPLITS(split1, split2, hsize);
			free_mat((*res), hsize);
			exit(0);
		}else{
		
			CLEAN_SPLITS(split1, split2, hsize);

			int** p1 = NULL, **p2 = NULL, **p3 = NULL, **p4 = NULL, **p5 = NULL, **p6 = NULL, **p7 = NULL;
			
			close(fd[1]);	//close write end of this pipe which goes to child
			//parent reads from each child process
			for(int i = 0; i < 7; i++){
				int p_mat_id;
				int** temp_mat = read_pipe(fd[0], &p_mat_id);
			
				int status;
				//waitpid(children[i], &status, 0);
				//debug
				//printf(" status %d child %d\n", status, children[i]);

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
			
			int **q1, **q2, **q3, **q4;
			GET_Q_MATS(hsize, 0, q1, q2, q3, q4, p1, p2, p3, p4, p5, p6, p7);
			
			CLEAN_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);
			
			//reassemble
			(*res) = combine(q1, q2, q3, q4, size);
			
			CLEAN_Q_MATS(q1, q2, q3, q4, hsize);
			
			pid_t wpid;
			while((wpid = wait(NULL)) >= 0){}

			semaphore_destroy(semid, 0);
		}
	}
	return 0;
}

int nearest_sq(int n){
	int ret = 1;
	while(n > ret){
		ret <<= 1;
	}
	return ret;
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

	int sqsize = nearest_sq(size);
	//init matrices	
	int** mat1 = mat_init(sqsize, 1);
	int** mat2 = mat_init(sqsize, 1);
	

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

	struct timeval t1, t2;

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
		int pass;
		if(benchmark){
			gettimeofday(&t1, NULL);
			pass = fork_strassen(1, mat1, mat2, &res, sqsize, nearest_sq(fork_bound), 0);
			gettimeofday(&t2, NULL);
		}else{
			pass = fork_strassen(1, mat1, mat2, &res, sqsize, nearest_sq(fork_bound), 0);
		}

		if(pass == -1){
			printf("Strassen failed\n");
			return 0;
		}

		printf("Finished IPC Strassen\n");

	}else{
		
		gettimeofday(&t1, NULL);
		res = strassen(mat1, mat2, sqsize);
		gettimeofday(&t2, NULL);
		
		printf("Finished basic Strassens\n");
		free_mat(mat1, sqsize);
		free_mat(mat2, sqsize);
	}

	//verify result
	int** check_mat = read_check_mat(size, file);
	printf("Checking Correctness\n");
	if(check_mul(check_mat, res, size) == -1){
		printf("Strassen incorrect\n");
		free_mat(check_mat, size);
		free_mat(res, sqsize);
		return 0;
	}
	printf("Strassen correct\n");
	free_mat(check_mat, size);
	free_mat(res, sqsize);

	if(benchmark) print_stats(&t1, &t2, type, fork, size, fork_bound);
	return 0;
}


