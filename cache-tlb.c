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
#include <sys/time.h>


#define ITER   100
#define MAX_N 64*1024*1024 
#define MB    (1024*1024)

// Max Last-level cache size parameters assumed
#define START_SIZE 1*MB
#define STOP_SIZE  32*MB

char array[MAX_N];



/////////////////////////////////////////////////////////
// Provides elapsed Time between t1 and t2 in nanoseconds
/////////////////////////////////////////////////////////
double getdetlatimeofday(struct timeval *begin, struct timeval *end)
{
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
           (begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

double DummyTest(void)
{    
  struct timeval t1, t2;
  int ii, iterid;

  // start timer
  gettimeofday(&t1, NULL);

  for(iterid=0;iterid<ITER;iterid++){
    for(ii=0; ii< MAX_N; ii++){
      array[ii] += rand();
    }
  }

  // stop timer
  gettimeofday(&t2, NULL);
 
  return getdetlatimeofday(&t1, &t2);
}



/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double CacheLineSizeTest(void)
{    
  double retval;

  return retval; 
}


/////////////////////////////////////////////////////////
// Change this, including input parameters. Calculate L1, L2, LLC size
/////////////////////////////////////////////////////////

double CacheSizeTest(void)
{    
  double retval;

  return retval; 
}


/////////////////////////////////////////////////////////
// Change this, including input parameters. Calculate TLB size
/////////////////////////////////////////////////////////

double TLBSizeTest(void)
{    
  double retval;

  return retval; 
}



/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double MemoryTimingTest(void)
{    
  double retval;

  return retval; 
}


/////////////////////////////////////////////////////////
// Print L1, L2, LLC, and TLB size as well as memory latency
/////////////////////////////////////////////////////////
void print_stats() {



} 

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int main(){
  
  // Add your code here, and comment above
  

  print_stats();

}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

