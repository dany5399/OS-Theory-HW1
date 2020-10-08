/* CS 519, FALL 2020: HW-1 
 * IPC using shared memory to perform matrix multiplication.
 * Feel free to extend or change any code or functions below.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "sem_ops.h"
#include "mat_ops.h"
#include "strassen.h"
#include "shm_ops.h"
#include <signal.h>

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

void handle(int sig){
	printf("SEGV in %d\n", getpid());
	//printf("errno %d\n", errno);
}

int** fork_strassen(int fmats, int** mat1, int** mat2, int size, int fork_bound, int div){

	int hsize = size/2;
	//call regular strassens
	if(size <= fork_bound || div == 3 || size <= 2){
		return strassen(mat1, mat2, size);
	}else{
		
		int hsize = size/2;
		
		int*** split1 = split(mat1, hsize);
		int*** split2 = split(mat2, hsize);
		
		if(fmats){
			free_mat(mat1, size);
			free_mat(mat2, size);
		}

		//for when the child updates with a new shared memory location
		int shm_keys_id = shm_init(getpid(), sizeof(int)*7);
		
		pid_t curr_pid = -1;
		pid_t children[7];
		int split_id;

		for(int i = 0; i < 7; i++){
			if((children[i] = fork()) == -1) return NULL;	//should just return zero mat instead but meh
			if(children[i] == 0){
				curr_pid = children[i];
				split_id = i+1;
				break;
			}
		}
	
		if(curr_pid == 0){
			//shm array that holds keys of shm matrices that children create
		
			signal(SIGSEGV, handle);
			int** res = NULL;
			switch(split_id){
				case 1: {
					int** F_H;
					SD_SPLIT(hsize, 0, F, H, F_H, 0);
					res = fork_strassen(0, A, F_H, hsize, fork_bound, div+1); 
					free_mat(F_H, hsize); break;
				}
				case 2: {
					int** AB;
					SD_SPLIT(hsize, 0, A, B, AB, 1);
					res = fork_strassen(0, AB, H, hsize, fork_bound, div+1);
					free_mat(AB, hsize); break;
				}
				case 3: {
					int** CD;
					SD_SPLIT(hsize, 0, C, D, CD, 1);
					res = fork_strassen(0, CD, E, hsize, fork_bound, div+1);
					free_mat(CD, hsize); break;
				}
				case 4: {
					int** G_E;
					SD_SPLIT(hsize, 0, G, E, G_E, 0);
					res = fork_strassen(0, D, G_E, hsize, fork_bound, div+1);
					free_mat(G_E, hsize); break;
				}
				case 5: {
					int** AD, **EH;
					SD_SPLIT(hsize, 0, A, D, AD, 1);
					SD_SPLIT(hsize, 0, E, H, EH, 1);
					res = fork_strassen(0, AD, EH, hsize, fork_bound, div+1);
					free_mat(AD, hsize); free_mat(EH, hsize); break;
				}
				case 6: {
					int** B_D, **GH;
					SD_SPLIT(hsize, 0, B, D, B_D, 0);
					SD_SPLIT(hsize, 0, G, H, GH, 1);
					res = fork_strassen(0, B_D, GH, hsize, fork_bound, div+1);
					free_mat(B_D, hsize); free_mat(GH, hsize); break;
				}
				case 7: {
					int** A_C, **EF;
					SD_SPLIT(hsize, 0, A, C, A_C, 0);
					SD_SPLIT(hsize, 0, E, F, EF, 1);
					res = fork_strassen(0, A_C, EF, hsize, fork_bound, div+1);
					free_mat(A_C, hsize); free_mat(EF, hsize); break;
				}
			}
			
			//make a shm matrix
			int cid;
			cid = shm_init(getpid(), sizeof(int)*hsize*hsize);
			int* flat_mat = attach(cid, 0);	
			//copy_mat(res, child_shm_mat, hsize); //copy result matrix from previous recusive calls into shm
			flat_mat = copy_to_shm_mat(res, flat_mat, hsize);
		
			detach(flat_mat);

			//put in shared array child's shm matrix key so parent can access it. uses split_id
			//so there aren't any collision between children
			int* shm_child_keys = attach(shm_keys_id, 0);
			//printf("child split %d key %d shared %d\n", split_id, cid, shm_child_keys[split_id-1]);
			shm_child_keys[split_id-1] = cid;
			//printf("after split %d input %d\n", split_id, shm_child_keys[split_id-1]);
			detach(shm_child_keys);

			free_mat(res, hsize);
			CLEAN_SPLITS(split1, split2, hsize);
			exit(0);
		}else{
		
			CLEAN_SPLITS(split1, split2, hsize);

			int** p1 = NULL, **p2 = NULL, **p3 = NULL, **p4 = NULL, **p5 = NULL, **p6 = NULL, **p7 = NULL;
		
			int checked[7] = {0}; //for checking if spot is already consumed
			unsigned int iter = 0; //go back to zero on overflow
			int n = 7;

			//loop through checking for updated keys in shm array of child shm matrix keys.
			//0 in checked indicates no key update in spot, -1 indicates already got shm matrix
			
			//shm array that holds keys of shm matrices that children create
			int* shm_child_keys = attach(shm_keys_id, SHM_RDONLY);
		
			while(n > 0){
				int i = iter%7;
				int check_key = shm_child_keys[i];

				if(check_key > 0 && checked[i] == 0){
					
					//printf("parent split %d key %d\n", i+1, check_key);
					int* ch_array = attach(check_key, 0);
					int** temp_mat = convert_shm_mat(ch_array, hsize);
					//print_mat(temp_mat, hsize);
					//assign based on which shm array spot key was found
					switch(i){
						case 0: p1 = temp_mat; break;
						case 1:	p2 = temp_mat; break;
						case 2: p3 = temp_mat; break;
						case 3: p4 = temp_mat; break;
						case 4: p5 = temp_mat; break;
						case 5: p6 = temp_mat; break;
						case 6: p7 = temp_mat; break;
					}
					detach(ch_array);
					shm_destroy(check_key);
					checked[i] = -1;
					n--;
				}
				iter++;
			}

			int **q1, **q2, **q3, **q4;
			GET_Q_MATS(hsize, 0, q1, q2, q3, q4, p1, p2, p3, p4, p5, p6, p7);
			
			CLEAN_P_MATS(hsize, p1, p2, p3, p4, p5, p6, p7);
			
			int** res = combine(q1, q2, q3, q4, size);

			CLEAN_Q_MATS(q1, q2, q3, q4, hsize);
			detach(shm_child_keys);
			shm_destroy(shm_keys_id);
			
			pid_t wpid;
			while((wpid = wait(NULL)) >= 0){}

			return res;
		}
	}
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
		
		if(benchmark){
			gettimeofday(&t1, NULL);
			res = fork_strassen(1, mat1, mat2, sqsize, nearest_sq(fork_bound), 0);
			gettimeofday(&t2, NULL);
		}else{
			res = fork_strassen(1, mat1, mat2, sqsize, nearest_sq(fork_bound), 0);
		}

		if(res == NULL){
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
		print_mat(check_mat, size);
		print_mat(res, size);
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


