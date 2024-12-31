#include "myio.h"
#include "intstr.h"
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include "VirtualAllocTwo.h"

#include <stdlib.h>

#define POWERS_AMOUNT 25


static Allocator* allocator = NULL;

#define MP_ANONYMOUS 0x20
Allocator* allocator_create(const size_t size ){
    Allocator* allocator;
    for (size_t i = 0; i < POWERS_AMOUNT; i++)
    {
        void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MP_ANONYMOUS, -1, 0);
        if(memory == MAP_FAILED){
            for (size_t j = i; j >= 0; j--)
            {
                munmap(allocator->head[i]->memory, size);
            }
            return NULL;
        }

        if(i == 0){ 

            allocator = (Allocator*)memory;

            allocator->head[i] = (struct Header*)((char*)memory + sizeof(Allocator));

            allocator->head[i]->memory = (void*)((char*)memory + sizeof(Allocator) + sizeof(struct Header));

            allocator->head[i]->size = size - sizeof(struct Header);
            allocator->head[i]->power = i;

            struct Block* block = (struct Block*)allocator->head[i]->memory; 
            block->next = NULL;
            block->size = allocator->head[i]->size - sizeof(struct Block);
            allocator->head[i]->head = block;
            continue;
        }

        allocator->head[i] = (struct Header*)memory;
        allocator->head[i]->memory = (void*)((char*)memory + sizeof(struct Header));
        allocator->head[i]->size = size - sizeof(struct Header);
        allocator->head[i]->power = i;

        struct Block* block = (struct Block*)allocator->head[i]->memory;
        block->next = NULL;
        block->size = allocator->head[i]->size - sizeof(struct Block);
        allocator->head[i]->head = block;
    }
    allocator->size_of_block = size; 
    atexit(allocator_destroy_exit);
    return allocator;
}

void allocator_destroy(const Allocator* allocator){
    for (int j = POWERS_AMOUNT - 1; j >= 0; j--)
    {
        if(j == 0){
            munmap((void*)((char*)allocator->head[j]->memory - sizeof(struct Header) - sizeof(Allocator)), allocator->size_of_block);
            continue;
        }
        munmap((void*)((char*)allocator->head[j]->memory - sizeof(struct Header)), allocator->size_of_block);
    }
    allocator = NULL;
    return;
}

void allocator_destroy_exit(){
    for (int j = POWERS_AMOUNT - 1; j >= 0; j--)
    {
        if(j == 0){
            munmap((void*)((char*)allocator->head[j]->memory - sizeof(struct Header) - sizeof(Allocator)), allocator->size_of_block);
            continue;
        }
        munmap((void*)((char*)allocator->head[j]->memory - sizeof(struct Header)), allocator->size_of_block);
    }
    allocator = NULL;
    return;
}



void* allocator_alloc(const Allocator* allocator, const size_t size){
    if(allocator == NULL){
        return NULL;
    }
    size_t power = (size_t)ceil(log2(size));


    struct Block* empty = allocator->head[power]->head, *prev = empty;
    int mode = 0;
    size_t allocated = 0, empty_memo_between = 0;

    if(empty->size == 0 && (char*)empty->next - (char*)empty - sizeof(struct Block) >= 1 << power){
        empty->size = 1 << (int)ceil(log2(size));
        return (void*)((char*)empty + sizeof(struct Block));
    }

    while(empty->next != NULL){
        prev = empty;
        
        allocated += (prev->size + sizeof(struct Block));
        
        empty = empty->next;

        size_t delta = empty - prev - sizeof(struct Block) - prev->size;
        empty_memo_between += delta <= 0? 0: delta; 

        mode = empty->next == NULL ? 0 : 1;
        if(empty != prev && (char*)empty - (char*)prev - sizeof(struct Block) - prev->size >= 1 << power){
            break;
        }
    }
    if(mode){
        struct Block *delta_block;
        delta_block = (struct Block*)((char*)prev + prev->size + sizeof(struct Block));
        prev->next = delta_block;
        delta_block->size = 1 << power;
        delta_block->next = empty;
        return (void*)((char*)delta_block + sizeof(struct Block));
    }
    else{

        struct Block *next_block;
        next_block = (struct Block*)((char*)empty + sizeof(struct Block) + (1 << power));
        size_t empty_memory = empty->size;
        empty->size = 1 << power;
        empty->next = next_block;
        next_block->size = empty_memory - sizeof(struct Block) - (1 << power);
        next_block->next = NULL;
        return (void*)((char*)empty + sizeof(struct Block));
    }

}

void allocator_free(const Allocator* allocator, const void* memory){
    struct Block* block = (struct Block*)((char*)memory - sizeof(struct Block));
    
    if(allocator == NULL){
        return;
    }

    size_t power = (size_t)ceil(log2(block->size));

    struct Block* delete_block = (struct Block*)((char*)memory - sizeof(struct Block)), *search = (struct Block*)allocator->head[power]->head/* allocator->head_of_blocks */, *prev = search; //а если head_of_blocks освободили уже???
    if(search->next == NULL){
        my_write("The memory that should have been deleted not found!\n");
        exit(-1);
        return;
    }
    while(search != delete_block && search != NULL){
        prev = search;
        search = search->next;
    }
    if(search == NULL){
        my_write("The memory that should have been deleted not found!\n");
        exit(-1);
        return;
    }
    else if(search == allocator->head[power]->head)
    {
        if(search->next != NULL){
            search->size = 0;
            return;
        }else{
            my_write("The memory that should have been deleted not found!\n");
            exit(-1);
            return;
        }
    }
    else{
        prev->next = search->next;
    }
    return;
}

void* allocate(const size_t size){
    if(allocator == NULL){
        allocator = allocator_create(1024 * 1024 * 1024);
        if(allocator == NULL){
            return NULL;
        }
    }
    void* ret_memory = allocator_alloc(allocator, size);
    if(ret_memory == NULL){
        return NULL;
    }
    return ret_memory;
}

void deallocate(const void* memory){
    if(allocator == NULL){
        return;
    }
    allocator_free(allocator, memory);
    return;
}

