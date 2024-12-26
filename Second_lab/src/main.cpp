#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <random>
#include <math.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>

#include "intstr.h"
#include "bitSort.h"
#include "myio.h"
#include "multiThread.h"




int get_random_array(int*& array, int num, int lb, int rb){
    int amount = (1 << num);
    array = (int*)malloc(sizeof(int) * amount);
    if(array == NULL){
        return 0;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(lb, rb);
    for (int i = 0; i < amount; i++)
    {
        array[i] = dis(gen);
    }
    return 1;
}

//Format: ./<function> <lengthOfArray(the degree of two)> <lb> <rb>
int main(int argc, char* argv[]){
    if(argc != 4){
        my_write("Wrong amount of arguments!\n");
        return -1;
    }
    else if(!hasOnlyNums(argv[1]) || !hasOnlyNums(argv[2]) || !hasOnlyNums(argv[3])){
        my_write("Wrong type of argument! All arguments must be integer!\n");
        return -1;
    }
    int amount = str_to_int(argv[1]), lb = str_to_int(argv[2]), rb = str_to_int(argv[3]);
    if(lb > rb || amount <= 0){
        my_write("Border | amount error!\n");
        return -1;
    }

    int *arrayForRec, *arrayForLin;

    get_random_array(arrayForRec, amount, lb, rb);
    get_random_array(arrayForLin, amount, lb, rb);

    amount = (1 << amount);
    my_write("How many threads do you want to use?\n");

    int numCPU = sysconf(_SC_NPROCESSORS_ONLN);

    my_write("Currently the amount of CPUs that are avaluable is ");
    char* chr;
    my_write(int_to_str(numCPU, chr));
    free(chr);
    my_write("\n");
    char buf[BUFSIZ];
    my_read(buf);
    if(hasOnlyNums(buf) && str_to_int(buf) > 0){
        //for recursion
        {
            struct timespec start, stop;
            clock_gettime(CLOCK_REALTIME, &start);
            bitsort(arrayForRec, amount, 1);
            clock_gettime(CLOCK_REALTIME, &stop);
            char *str;
            str = double_to_str((stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3, str);
            
            my_write("Time for recursion: ");
            my_write(str);
            my_write("\n");
            free(str);
        }
        //for multi-thread
        {
            struct timespec start, stop;
            clock_gettime(CLOCK_REALTIME, &start);

            bitonicSort2(arrayForLin, amount, str_to_int(buf), 1);

            clock_gettime(CLOCK_REALTIME, &stop);
            char *str;
            str = double_to_str((stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3, str);
            
            my_write("Time for multi-thread: ");
            my_write(str);
            my_write("\n");
            free(str);
        }

        char fileName1[100], fileName2[100];
        strcpy(fileName1, "recursion.txt");
        strcpy(fileName2, "multiThread.txt");

        int descriptor1, descriptor2;
        descriptor1 = file_open(fileName1);
        for (int i = 0; i < amount; i++)
        {
            char* str = int_to_str(arrayForRec[i], str);
            write(descriptor1, str, strlen(str));
            if(i % 15 == 0){
                write(descriptor1, "\n", 1);
            }
            else{
                write(descriptor1, " ", 1);
            }
            free(str);
        }
        file_close(descriptor1);

        descriptor2 = file_open(fileName2);
        for (int i = 0; i < amount; i++)
        {
            char* str = int_to_str(arrayForLin[i], str);
            write(descriptor2, str, strlen(str));
            if(i != 0 && i % 15 == 0){
                write(descriptor2, "\n", 1);
            }
            else{
                write(descriptor2, " ", 1);
            }
            free(str);
        }
        file_close(descriptor2);

        my_write("\nOK\n");
        free(arrayForLin);
        free(arrayForRec);
        return 0;
    }
    else{
        my_write("Wrong amount of CPUs\n");
        return -1;
    }
    free(arrayForLin);
    free(arrayForRec);
    return 0;
}