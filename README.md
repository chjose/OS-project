# OS-project

Part 1 Determining L2 cache parameters.

  The program approximates the L2 cache parameters. Uses two methods to find the cached line/block size and cache size.

i) Cache block size

  Method used here is to traverse the char array using different strides which are powers of 2. A large array is used here and traversed once so that it a loaded in the main memory with virtual addresses mapped. Assume the max block size as 2^k Bytes and iterate an outer loop with this limit. When the stride is greater than the cache block size we get a miss in most of the accesses. So this spike in the time gives us the cache block size which will be the stride used for the previous iteration. 

ii) Cache miss penalty

  This can be computed inside the function to find the cache block size. This is the time which is taken to fetch the block from a lower level cache. In program this will be the average access time for the previous iteration minus the present iteration(when the previous was the cache block size).

iii) Cache size

  This can be determined by increasing the array size in each iteration so that finally it exceeds the cache size. Access is made such that we access different blocks in each steps. Use the below the logic to get this done:

  Array[i*BLOCK_SIZE&ARRAY_SIZE] = some random operation; 

The program is filled with comments to increase the readability. Also it writes down the analysis result to a Cache_Analysis.txt file which will created in the same directory when the program is executed.

Next page please find the graphs demonstrating the spike which helped to determine the cache block size and cache size. In the below graph for Cache block size, the block size is 64 Bytes and the cache miss penalty is 7 NS (13 – 6).

NOTE:  The program only gives an approximation of the cache params. During some runs it could give a wrong value. For better results, run this for multiple times and the max occurring value will be answer. On an average in my machine it was having a success rate of 80 % which I feel is quite good as this involves measuring the hardware bounds which is not directly accessible to a program. Also there is web page I had referred to get some enlightenment to the above method (http://igoro.com/archive/gallery-of-processor-cache-effects/), so giving due credits and mention to this.








Part 2 User Level thread library

      The program consists of two files, mypthread.c and mypthread.h. mypthread.c has the function definitions required for the cooperative thread library implementation. The methodology that I have used for this is as per the guidelines in the HW pdf. The following sub sections explain the way this library functions are implemented.

i) Framework and scheduler used

  I have made used of a circular linked list to ensure uniform scheduling. This list will be global and in mypthread.c we access this list and store the thread object in each node. Whenever a thread finishes execution(by means of yield or exit) it transfers control to the next READY thread in the list. The first node in the list is for the main() function, or the first function where a mypthread_yield is called. Once the control is swapped from main thread, it will only return after the completion of the currently active(READY) threads in the list. The program is very much flexible that the scheduling order can be changed using the join function, where the thread which calls the join function will be moved to a WAIT state. The main thread will be present in this list till the program exits.  

ii) mypthread_create

  This functions receives the thread pointer from the user program. This function initializes the ucontext_t structure of the thread object and inserts the thread object into the circular linked list. The status of each thread is made READY in the create function.

Iii) mypthread_yield

  In order to start the threads, this function needs to be called. When its called this function just picks up a READY thread from the list and swaps it with the current executing one. In the case of the first thread execution, it is swapped with the main thread. We also set a current node pointer in this function to keep track of the current thread.

iv) mypthread_exit

  A thread exits or terminates by calling this function. As this program is just a library demo, I have not made it complex by adding a garbage collector(that is freeing the NOT_ACTIVE threads). This can be implemented in the exit function quite easily. And also this program has an upper limit of 100 threads (ACTIVE and NOT ACTIVE) for the sake of simplicity and readability. This function changes the status of current thread to NOT_ACTIVE and calls the yield function to take up the next eligible thread in the list.

v) mypthread_join

  This function actually violates the concept of threads as the user controls the transfer of execution and its not the scheduler. Here the specified thread in the join function will be executed and the caller thread will be put to WAIT state and will be in this state till the other thread exits.

vi) mypthread_t structure

struct mypthread_s
{
  int thread_id;
  int status;
  mypthread_t* wait_for;
  ucontext_t a;
};

thread_id – Each thread got an id which starts from 1 and the main thread has the value 0.

status – There are four possible status: 0) NOT_ACTIVE 1) READY 2) MAIN_THREAD 3) WAIT

wait_for – this field is used to change the status of a WAIT thread to READY if the wait_for thread is in NOT_ACTIVE state(that means it had exited).

a – has the stack and stores the context of the thread.

vi) start and current pointers

  Start holds the head of the circular list and current points to the current thread.

vii) Limitations

  a) Maximum of 100 threads – For the purpose of illustration.
  b) join cannot be executed from main based on this scheduling algorithm. For me it doesn’t seem to be a limitation but just mentioned it here. Once yield is called from main, the control comes back to main only after all the threads in the current list gets executed. So a join call doesn’t make any sense, it just prints the thread is not active as the thread has already finished executing. If this doesn’t looks correct according to the question please give me a chance to correct it.
