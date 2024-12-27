#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>

#include "myio.h"
#include "intstr.h"

void* (*allocate)(const size_t size);

void (*deallocate)(const void* memory);


void* init_lstlib(){
    void *hdl = dlopen("./lib/liblstalloc.so", RTLD_LAZY);
    if(hdl == NULL){
        return NULL;
    }
    allocate = (void* (*)(const size_t))dlsym(hdl, "allocate");
    if(allocate == NULL){
        return NULL;
    }
    deallocate = (void (*)(const void*))dlsym(hdl, "deallocate");
    if(deallocate == NULL){
        return NULL;
    }
    return hdl;
}

void* init_tbllib(){
    void *hdl = dlopen("./lib/libtblalloc.so", RTLD_LAZY);
    if(hdl == NULL){
        return NULL;
    }
    allocate = (void* (*)(const size_t))dlsym(hdl, "allocate");
    if(allocate == NULL){
        return NULL;
    }
    deallocate = (void (*)(const void*))dlsym(hdl, "deallocate");
    if(deallocate == NULL){
        return NULL;
    }
    return hdl;
}

void* init_stdlib(){
    void *hdl = dlopen("./lib/libstd.so", RTLD_LAZY);
    if(hdl == NULL){
        return NULL;
    }
    allocate = (void* (*)(const size_t))dlsym(hdl, "custom_malloc");
    if(allocate == NULL){
        return NULL;
    }
    deallocate = (void (*)(const void*))dlsym(hdl, "custom_free");
    if(deallocate == NULL){
        return NULL;
    }
    return hdl;
}

int main(int argc, char const *argv[])
{
    void* hdl;
    char allocator_name[100];
    if(argc != 2){
        if((hdl = init_stdlib()) == NULL){
            my_write("Library not found!\n");
            return -1;
        }
        strcpy(allocator_name, "Standart");
    }
    else{
        if(!strcmp("List", argv[1])){
            if((hdl = init_lstlib()) == NULL){
                my_write("Library not found!\n");
                return -1;
            }
        }
        else if(!strcmp("Table", argv[1])){
            if((hdl = init_tbllib()) == NULL){
                my_write("Library not found!\n");
                return -1;
            }
        }
        else{
            my_write("Unknown allocator <"); my_write(argv[1]); my_write(">. Choose between <List> or <Table> allocators...\n");
            return -1;
        }
        strcpy(allocator_name, argv[1]);
    }

    clock_t start = clock();
    int** array = (int**)allocate(sizeof(int*) * 50000);
    int iter = 0;

    
    for (size_t i = 1; i <= 10; i++)
    {
        for (size_t j = 0; j < 5000; j++)
        {
            array[iter++] = allocate(sizeof(int) * i);
            if(array[iter - 1] == NULL){
                for (size_t k = 0; k < iter; k++)
                {
                    deallocate(array[k]);
                }
                deallocate(array);
                my_write("Error while allocating!\n");
                return -1;
            }
        }
    }
    clock_t end = clock();
    my_write("Memory allocating with library <"); my_write(allocator_name); my_write("> lasts: "); print_int((end - start) / 1000); my_write(" ms\n");
    print_int(50000);my_write(" int* allocated.\n");

/* 
    for (size_t i = 0; i < iter; i++)
    {
        array[i][0] = i;
    }
 */
    start = clock();
    for (int i = iter - 1; i >= 0; i--)
    {
        deallocate(array[i]);
    }
    deallocate(array);
    end = clock();
    my_write("Memory deallocating with library <"); my_write(allocator_name); my_write("> lasts: "); print_int((end - start) / 1000); my_write(" ms\n");
    print_int(50000);my_write(" int* deallocated.\n");

    dlclose(hdl);

    return 0;
}
