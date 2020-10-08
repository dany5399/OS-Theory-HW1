/* CS 519, FALL 2020: HW-1 
 * Measuring the caches size, TLB size, and memory latency
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
#include <time.h>


#define MAX_N 64*1024*1024 
#define MB    (1024*1024)


#define ITER   4096

/////////////////////////////////////////////////////////
// Print L1, L2, LLC, and TLB size as well as memory latency
/////////////////////////////////////////////////////////
void print_stats() {



} 
double delta_time(struct timespec b, struct timespec e){
	return (double) (e.tv_sec * 1000000000.0 + e.tv_nsec) - (b.tv_sec * 1000000000.0 + b.tv_nsec);
}

#define MB32 32*MB
int larg_af_array[64*MB];

unsigned long memory(){
	
	time_t t;
	struct timespec b, e;
	srand((unsigned) time(&t));
	int access = 16*MB; //start at accessing 16 bytes per line

	int sum = 0;
	int strides[ITER];
	
	for(int k = 0; k < ITER; k++){
		strides[k] = rand() % access;
	}
	int r = 0;
	clock_gettime(CLOCK_REALTIME, &b);
	for(int j = 0; j < ITER; j++){
		r = strides[j];
		sum += larg_af_array[(r+sum)];	//trigger page fault
	}
	clock_gettime(CLOCK_REALTIME, &e);
	
	long time = (e.tv_nsec - b.tv_nsec)/(ITER);

	printf("Memory Timing: %ldns\n\n", time);
}

void tlb(){
	
	time_t t;
	srand((unsigned) time(&t));
	int access = 16*MB; //start at accessing 16 bytes per line

	int sum = 0;
	int strides[ITER];
	
	for(int k = 0; k < ITER; k++){
		strides[k] = rand() % access;
	}
	
	long last_avg = 0;

	for(int i = 0; i < ITER; i++){
		
		struct timespec b, b1, e, e1;

		sum += larg_af_array[(strides[i]+sum)];	//should trigger pagefault
		
		//strides[i] += sum;	//store location
		int x = 0;
			
		clock_gettime(CLOCK_REALTIME, &b);
		for(int j = 0; j <= i; j++){
			x += larg_af_array[strides[j] + x];
			//reaccess old stuff randomly and check for multiple page faults	
		}
		clock_gettime(CLOCK_REALTIME, &e);

		long new_avg = 0;
		if(i > 128){	//calculate once we've gotten decently far
			new_avg = (e.tv_nsec - b.tv_nsec)/i;
		}	
		
		//printf("size %d; last avg %ld; new avg %ld\n", i, last_avg, new_avg);
		long delta = new_avg - last_avg;
		printf("Size %d; Delta avg: %ld\n", i, (delta < 0) ? delta*-1:delta);
		last_avg = new_avg;
	}
		
		
}

#define KB 1024
void caches(){

	int n_step = 16;
	int repeats = 10.0;
	int accesses = 16*MB;
	time_t t;
	srand((unsigned) time(&t));

	int pos_sizes[17] = {2*KB, 4*KB, 8*KB, 16*KB, 32*KB, 64*KB, 128*KB, 256*KB, 512*KB, MB, 2*MB, 4*MB, 8*MB, 12*MB, 16*MB, 32*MB, 64*MB};

	double last = 0;
	int L1 = 0, L2 = 0, L3 = 0;
	double overhead = 2.0; //assumed overhead in nanoseconds;

	for(int i = 0; i < 17; i++){
		int curr = (pos_sizes[i]-1)/sizeof(int);
		
		double time_to_test = 0.0;
		struct timespec b, e;
		for(int j = 0; j < repeats; j++){
			int* rands = malloc(sizeof(int)*accesses);
			for(int r = 0; r < accesses; r++){
				rands[r] = rand()%pos_sizes[i];
			}
			int sum =0;
			clock_gettime(CLOCK_REALTIME, &b);
			for(int k = 0; k < accesses; k++){
				sum += larg_af_array[((rands[k] + sum) * n_step) & curr];
			}
			clock_gettime(CLOCK_REALTIME, &e);
			time_to_test += (delta_time(b, e)/accesses);
			free(rands);
		}
		double time = (time_to_test/repeats) - overhead;
		printf("Size Tested: %d\t Time: %f\n", pos_sizes[i], time);
		if(i == 0) last = time;
		double diff = (time - last < 0) ? (time-last)*-1:(time-last);
		if(diff > .2 && L1 == 0){
			printf("L1 size: %d\n", pos_sizes[i]);
			L1 = 1;
		}else if(time > 6 && L1 == 1 && L2 == 0){
			printf("L2 size: %d\n", pos_sizes[i-1]);
			L2 = 1;
		}else if(diff > 8.0 && L3 == 0){
			printf("L3 size: %d\n", pos_sizes[i]);
			L3 = 1;
			
		}
		last = time;
	}
}

void cache_line(){

	int repeats = 10.0;
	int accesses = 32*MB;
	time_t t;
	srand((unsigned) time(&t));

	double last = 1.50;
	int L1 = 0, L2 = 0, L3 = 0;
	double overhead = 2.0; //assumed overhead in nanoseconds;

	int mod = MB32 - 1;
	
	int found = 0;

	for(int i = 1; i <= 256; i<<=1){
		double time_to_test = 0.0;
		struct timespec b, e;

		for(int j = 0; j < repeats; j++){	
			int s = i/4;
			int sum =0;
			clock_gettime(CLOCK_REALTIME, &b);
			for(int k = 0; k < accesses; k++){
				larg_af_array[(k*i) & mod]++;
			}
			clock_gettime(CLOCK_REALTIME, &e);
			time_to_test += (delta_time(b, e)/accesses);
		}
		double time = (time_to_test/repeats);
		double diff = time - last;
		diff = (diff < 0) ? diff*-1:diff;
		printf("Cache Line: %d\t Time: %f\n", i*4, time);
		if(diff > 1.6 && found == 0){
			printf("CACHE LINE FOUND %d\n", i*4);
			found = 1;
		}
	}

}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int main(int argc, char** argv){
	for(int i = 0; i < 64*MB; i++){
		larg_af_array[i] = rand() % 50;
	}

	switch(atoi(argv[1])){
		case 0: cache_line(); break;
		case 1: caches(); break;
		case 2: tlb(); break;
		case 3: memory(); break;
	}

}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

