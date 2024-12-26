/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cFiles/main.c to edit this template
 */

/* 
 * File:   main.c
 * Author: vvg
 *
 * Created on 20 ноября 2024 г., 14:19
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/sysinfo.h> //for  get_nprocs (void) 

#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>

void swap(int *a, int *b){
    int tmp=*a;
    *a=*b;
    *b=tmp;
}

/*The parameter dir indicates the sorting direction, ASCENDING
   or DESCENDING; if (a[i] > a[j]) agrees with the direction,
   then a[i] and a[j] are interchanged.
   dir=1 - sort in ascending order
 */
void compAndSwap(int a[], int i, int j, int dir)
{
    if (dir==(a[i]>a[j]))
        swap(&a[i],&a[j]);
}
 
/*It recursively sorts a bitonic sequence in ascending order,
  if dir = 1, and in descending order otherwise (means dir=0).
  The sequence to be sorted starts at index position low,
  the parameter cnt is the number of elements to be sorted.*/
void bitonicMerge(int a[], int low, int cnt, int dir)
{
    if (cnt>1)
    {
        int k = cnt/2;
        for (int i=low; i<low+k; i++)
            compAndSwap(a, i, i+k, dir);
        bitonicMerge(a, low, k, dir);
        bitonicMerge(a, low+k, k, dir);
    }
}
 
/* This function first produces a bitonic sequence by recursively
    sorting its two halves in opposite sorting orders, and then
    calls bitonicMerge to make them in the same order */
void bitonicSort(int a[],int low, int cnt, int dir)
{
    if (cnt>1)
    {
        int k = cnt/2;
 
        // sort in ascending order since dir here is 1
        bitonicSort(a, low, k, 1);
 
        // sort in descending order since dir here is 0
        bitonicSort(a, low+k, k, 0);
 
        // Will merge whole sequence in ascending order
        // since dir=1.
        bitonicMerge(a,low, cnt, dir);
    }
}
 
/* Caller of bitonicSort for sorting the entire array of
   length N in ASCENDING order */
void sort(int a[], int N, int up)
{
    bitonicSort(a,0, N, up);
}
///////// end recursive bitonic ///////////////// 

///////// parallel bitonic /////////////////////
struct PTHREAD_DATA{
    int *paddedValues;
    unsigned int threadId;
    unsigned int chunkSize;
    unsigned int mergeStep;
    unsigned int bitonicSequenceSize;
};

void reverse(int* first, int* last)
{
  while (1)
    if( first == last || first == --last )
      return;
    else{
        swap( first, last);
        ++first;
    }
}

void compareAndSwap(int paddedValues[], unsigned int threadId,
                    unsigned int chunkSize, unsigned int mergeStep, unsigned int bitonicSequenceSize)
{
    unsigned int startIndex = threadId * chunkSize;
    unsigned int endIndex = (threadId + 1) * chunkSize;

    // Process the chunk assigned to this thread
    for (unsigned int currentIndex = startIndex; currentIndex < endIndex; currentIndex++)
    {
        // Find the element to compare with
        unsigned int compareIndex = currentIndex ^ mergeStep;

        // Only compare if the compareIndex is greater (to avoid duplicate swaps)
        if (compareIndex > currentIndex)
        {
            bool shouldSwap = false;

            // Determine if we should swap based on the current subarray's sorting direction
            if ((currentIndex & bitonicSequenceSize) == 0)  // First half of subarray (ascending)
            {
                shouldSwap = (paddedValues[currentIndex] > paddedValues[compareIndex]);
            }
            else  // Second half of subarray (descending)
            {
                shouldSwap = (paddedValues[currentIndex] < paddedValues[compareIndex]);
            }

            // Perform the swap if necessary
            if (shouldSwap)
            {
                swap(&paddedValues[currentIndex], &paddedValues[compareIndex]);
            }
        }
    }
} 

void *thread_func(void* arg)
{
  struct PTHREAD_DATA* data=(struct PTHREAD_DATA*)arg;
  if(data){
    compareAndSwap(data->paddedValues, data->threadId, data->chunkSize, data->mergeStep, data->bitonicSequenceSize);
    free(data);
  }
  return NULL;
}

void bitonicSort2(int values[], unsigned int arrayLength, unsigned int numThreads, int sortOrder)
{
    // Step 1: Pad the array to the next power of 2
    unsigned int paddedLength = 1 << (int)(ceil(log2(arrayLength)));
    int *paddedValues=NULL;
    if(paddedLength>arrayLength){
        paddedValues=(int*)malloc(paddedLength*sizeof(int));
        if(paddedValues==NULL){
            printf("malloc error for paddedValues in bitonicSort\n");
            return;
        }

        for(int i=0; i<paddedLength; i++){
            paddedValues[i]=INT_MAX;
        }
        memcpy(paddedValues,values,arrayLength*sizeof(int));
    }
    else{
        paddedValues=values;
    }
    // Step 2: Determine chunk size for each thread
    unsigned int chunkSize = paddedLength / numThreads;
    
    // Step 3: Iteratively build and merge bitonic sequences
    // Outer loop: controls the size of bitonic sequences
    for (unsigned int bitonicSequenceSize = 2; bitonicSequenceSize <= paddedLength; bitonicSequenceSize *= 2)
    {
        // Middle loop: controls the size of sub-sequences being merged
        for (unsigned int mergeStep = bitonicSequenceSize / 2; mergeStep > 0; mergeStep /= 2)
        {
            // Step 4: Use multiple threads to compare and swap elements in parallel
            pthread_t *p_thread=malloc(numThreads*sizeof(pthread_t));

            if(p_thread!=NULL){
                
//                pthread_attr_t attr;
//                pthread_attr_init(&attr);
//                size_t stacksize = 500000; //500 000 bytes
//                pthread_attr_setstacksize (&attr, stacksize);
//                pthread_attr_getstacksize (&attr, &stacksize);
                
                // Thread creation loop
                for (unsigned int threadId = 0; threadId < numThreads; threadId++)
                {
                    struct PTHREAD_DATA *pdt = (struct PTHREAD_DATA *)malloc(sizeof(struct PTHREAD_DATA)); //free in thread
                    if(pdt!=NULL){
                        pdt->paddedValues=paddedValues;
                        pdt->threadId=threadId;
                        pdt->chunkSize=chunkSize;
                        pdt->mergeStep=mergeStep;
                        pdt->bitonicSequenceSize=bitonicSequenceSize;
                        pthread_create(&p_thread[threadId],/*&attr*/ NULL,thread_func,(void*)pdt);
                    }
                    else{
                        printf("malloc error for PTHREAD_DATA in bitonicSort\n");
                        free(p_thread);
                        if(paddedValues!=values)
                            free(paddedValues);
                        return;
                    }
                }

//                pthread_attr_destroy(&attr);
                
                // Wait for all threads to complete this stage
                for(unsigned int k=0;k<numThreads;k++) {
                    pthread_join(p_thread[k], NULL);
                //    printf("Completed join with thread %d\n",k);
                }

                free(p_thread);
            }
            else{
                printf("malloc error for paddedValues in bitonicSort\n");
                if(paddedValues!=values)
                    free(paddedValues);
                return;
            }
        }
    }
    
    // Step 5: Copy back the sorted values
    
    if(paddedValues!=values){
        memcpy(values, paddedValues, arrayLength*sizeof(int));
        free(paddedValues);
    }
        

    // Step 6: If descending order is required, reverse the array
    if (sortOrder == 0){
        reverse(values, values + arrayLength);
    }
}

void generate_random_arr(int arr[], int size) {
	int i, swap_index;
	int swap;

	// Generate the sequence from zero to 'size'
	for (i = 0; i < size; i++)
		arr[i] = i;

	// Shuffle the array from end to beginning
	for (i = size-1; i > 0; i--) {
		swap_index = rand() % i;
		swap = arr[i];
		arr[i] = arr[swap_index];
		arr[swap_index] = swap;
	}
}
///////// end parallel bitonic /////////////////
/*
 * 
 */
int main(int argc, char** argv) 
{
    int alg=2;
    int N=4096;
    if(argc>1){
        N=strtol(argv[1],NULL,10);
    }
    //variant 1
    int numCPU = sysconf(_SC_NPROCESSORS_ONLN); // The return value really represents the current number of the system currently available
    //variant 2
    int numCPU2 = get_nprocs(); // Real reflects the current number of available cores 
    
    printf("CPU number=%d (%d)\n",numCPU, numCPU2);
    
    if(argc>2){
        int cpu=strtol(argv[2],NULL,10);
        if(cpu<numCPU&& cpu>0)
            numCPU=cpu;
    }
    printf("CPU used=%d\n",numCPU);
    
    if(argc>3){
        alg=strtol(argv[3],NULL,10);
        if(alg<1 || alg>2)
            alg=1;
    }
    
    
    int *a= (int*)malloc(N*sizeof(int)); //{3, 7, 4, 8, 6, 2, 1, 5};
    //int N = sizeof(a)/sizeof(a[0]);
    if(a){
        struct timespec start, stop;
        generate_random_arr( a, N);

        /* printf("UnSorted array: \n");
        for (int i=0; i<N; i++)
            printf("%d ", a[i]);

        printf("\n----------------------\n"); */
        int up = 1;   // means sort in ascending order
        gettimeofday(&start, NULL);
        //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        ////classical///
        if(alg==1){
            const rlim_t kStackSize = 32 * 1024 * 1024;   // min stack size = 32 MB
            struct rlimit rl;
            int result;
            result = getrlimit(RLIMIT_STACK, &rl);
            if (result == 0){
                if (rl.rlim_cur < kStackSize){
                    rl.rlim_cur = kStackSize;
                    result = setrlimit(RLIMIT_STACK, &rl);
                    if (result != 0){
                        fprintf(stderr, "setrlimit returned result = %d\n", result);
                    }
                }
            }
            printf("Recursive Bitonic sort\n");
            sort(a, N, up);
        }
        else{
            printf("Parallel Bitonic sort\n");
            bitonicSort2(a, N, numCPU, up);
        }
        ////////////////
        //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        gettimeofday(&stop, NULL);
        
        /* printf("\n\nSorted array: \n");
        for (int i=0; i<N; i++)
            printf("%d ", a[i]); */
        ////////////////
        double result = (stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3;    // in microseconds
        printf("\n\nElapse time %g microseconds\n",result);
        sleep(1);
        free(a);
    }
    
    return (EXIT_SUCCESS);
}

