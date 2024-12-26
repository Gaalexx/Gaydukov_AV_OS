#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <random>
#include <math.h>

#include "myio.h"
#include "intstr.h"

struct PTHREAD_DATA{
    int *paddedValues;
    unsigned int threadId;
    unsigned int chunkSize;
    unsigned int mergeStep;
    unsigned int bitonicSequenceSize;
};

void swap(int *a, int *b){
    int tmp=*a;
    *a=*b;
    *b=tmp;
}

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

    for (unsigned int currentIndex = startIndex; currentIndex < endIndex; currentIndex++)
    {
        unsigned int compareIndex = currentIndex ^ mergeStep;

        if (compareIndex > currentIndex) //this prevents data race
        {
            bool shouldSwap = false;

            if ((currentIndex & bitonicSequenceSize) == 0) 
            {
                shouldSwap = (paddedValues[currentIndex] > paddedValues[compareIndex]);
            }
            else  
            {
                shouldSwap = (paddedValues[currentIndex] < paddedValues[compareIndex]);
            }

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
    unsigned int paddedLength = arrayLength; //(already padded according to the main, the length is a degree of two)
    
    unsigned int chunkSize = paddedLength / numThreads;
    int *paddedValues=values;
    // Outer loop: controls the size of bitonic sequences
    for (unsigned int bitonicSequenceSize = 2; bitonicSequenceSize <= paddedLength; bitonicSequenceSize *= 2)
    {
        for (unsigned int mergeStep = bitonicSequenceSize / 2; mergeStep > 0; mergeStep /= 2)
        {
            pthread_t *p_thread = (pthread_t*)malloc(numThreads*sizeof(pthread_t));

            if(p_thread!=NULL){

                for (unsigned int threadId = 0; threadId < numThreads; threadId++)
                {
                    struct PTHREAD_DATA *pdt = (struct PTHREAD_DATA *)malloc(sizeof(struct PTHREAD_DATA)); //free in thread
                    if(pdt!=NULL){
                        pdt->paddedValues=paddedValues;
                        pdt->threadId=threadId;
                        pdt->chunkSize=chunkSize;
                        pdt->mergeStep=mergeStep;
                        pdt->bitonicSequenceSize=bitonicSequenceSize;
                        pthread_create(&p_thread[threadId], NULL,thread_func,(void*)pdt);
                    }
                    else{
                        my_write("malloc error for PTHREAD_DATA in bitonicSort\n");
                        free(p_thread);
                        if(paddedValues!=values)
                            free(paddedValues);
                        return;
                    }
                }

                // Wait for all threads to complete this stage
                for(unsigned int k=0;k<numThreads;k++) {
                    pthread_join(p_thread[k], NULL);
                }

                free(p_thread);
            }
            else{
                my_write("malloc error for paddedValues in bitonicSort\n");
                if(paddedValues!=values)
                    free(paddedValues);
                return;
            }
        }
    }
    
    
    if(paddedValues!=values){
        memcpy(values, paddedValues, arrayLength * sizeof(int));
        free(paddedValues);
    }
        

    if (sortOrder == 0){
        reverse(values, values + arrayLength);
    }
}
