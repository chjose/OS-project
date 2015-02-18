#include <stdio.h> /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h> /* for clock_gettime */
#include "analyzecache.h"
 
int find_L2_block_size(float* miss_p, FILE *fp)
{
  int i = 0;
  int factor = 1;
  struct timespec start, end;
  long long unsigned int prev = 0;
  long long unsigned int diff = 0;
  int miss_penalty = 0;
  int block_size;

  for (i=0;i<=CACHE_MAX_SIZE;i++)
  {
    buffer[i]=2*i;
  }

  while(factor <= 512)
  {
    clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */
    /* Each time below loop is executed, stride is increased  by a power of 2 
     * When the miss penalty increases drastically, it marks we have crossed the 
     * cache block */
    for (i=0;i<CACHE_MAX_SIZE;i=i+factor)
    {
      buffer[i]++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    /* COndition to exclude the first iteration */
    if(factor != 1)
    {
      miss_penalty = (diff/(CACHE_MAX_SIZE/factor)) - prev;
    }

    if (miss_penalty > 3)
    {
      block_size = factor/2;
      break;
    }

    prev = diff/(CACHE_MAX_SIZE/factor);
    factor*=2;
  }
  /* Convert the miss penalty to micro seconds */
  *miss_p = (float)miss_penalty/1000;

  return block_size;
}

int find_L2_cache_size(FILE *fp, int block_size)
{
  int i = 0;
  int array_size = 1*1024;  // Start Array size from 1 KB
  struct timespec start, end;
  long long unsigned int diff = 0;
  int miss_penalty = 0;
  int memory[12],time[12],count = 0;
  int percent_change[11];
  int l2_size;
  
  for (i=0;i<CACHE_MAX_SIZE_EXT;i++)
  {
    buffer_ext[i]=(2*i);
  }

  while(array_size <= CACHE_MAX_SIZE_EXT)
  {
    clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */
    for (i=0;i<CACHE_MAX_SIZE_EXT;i++)
    {
      /* Here accessing elements such that each are from different blocks
       * '&' operation restricts the index in the array_size */
      buffer_ext[(i * block_size) & (array_size-1)] *= 10;
      buffer_ext[(i * block_size) & (array_size-1)] /= 10;
    }
    clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    diff =(long long unsigned int)(diff/CACHE_MAX_SIZE_EXT)/(BILLION*10);

    memory[count] = array_size/1024;
    time[count] = diff;
    count++;

    fprintf(fp,"\nARRAY SIZE = %d KB ", (array_size/1024));
    fprintf(fp,"\nACCESS TIME = %llu units\n", diff);
    array_size*=2;
  }

  for (i=0;i<11;i++)
  {
    percent_change[i] = ((float)(time[i+1]-time[i])/(float)time[i])*100;
    fprintf(fp,"\nPERCENT CHANGE = %d %% FROM MEMORY = %d KB to %d KB Array size\n",percent_change[i],memory[i],memory[i+1]);
  }
 
  /* Below loop starts from 6 so as to ignore L1 cache spike
   * Assuming here that the L1 is upto 32 KB */
  for (i=6;i<11;i++)
  {
    if(percent_change[i]>0)   
    /* If percent_change is non zero, this indicates a spike and the memory 
     * could be L2 cache size */
    {
      l2_size = memory[i];
    }

    /* Below if checks if after getting a spike if the change is constant 
     * that is zero, one or negative one*/
    if(l2_size!=0 && (percent_change[i] == 0))
      break;
  }
  
  return l2_size;
}

int main()
{
  int block_size, cache_size;
  float miss_penalty;
  FILE *fp;

  /* File Cache_Analysis.txt records important concepts used
   * Also it has Cache performance under different metrics
   * (Array size, Stride etc) and how the values are reached */
  fp = fopen("Cache_Analysis.txt","w");
  
  fprintf(fp,"\n Method adopted to find the cache size:\n");
  fprintf(fp,"\n --------------------------------------\n");
  fprintf(fp,"\n Access the array as blocks with increasing block size till we get a spike, after this closely monitor the percentage change, when it reaches constant that is zero, the memory associated with the previous non zero change will be the L2 size\n");
  block_size = find_L2_block_size(&miss_penalty, fp);
  cache_size = find_L2_cache_size(fp, block_size);

  printf("Cache Block/Line Size: %d B\n", block_size);
  printf("Cache Size: %d KB\n", cache_size);
  printf("Cache Miss Penalty: %.3f us\n", miss_penalty);
  
  return 0;
}

