#include "intstr.h"
#include "myio.h"
#include "VirtualAlloc.h"
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h> 

static Allocator* allocator = NULL;
#define MAP_ANONYMOUS 0x20


Allocator* allocator_create(void* memory, const size_t size){
    char* memo;
    if(memory == NULL){
        memo = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(memo == MAP_FAILED){
            perror();
            return NULL;
        }
    }
    else{
        memo = memory;
    }
    allocator = (Allocator*)memo;
    allocator->memory = (void*)((char*)memo + sizeof(Allocator));
    allocator->size = size - sizeof(Allocator);
    allocator->head_of_blocks = (struct Block*)allocator->memory;

    allocator->head_of_blocks->size = allocator->size;
    allocator->head_of_blocks->next = NULL;
    atexit(allocator_destroy_exit);
    return allocator;
} 

void allocator_destroy(const Allocator* allocator){
    if(allocator == NULL){
        return;
    }
    munmap(allocator->memory - sizeof(Allocator), allocator->size + sizeof(Allocator));
    allocator = NULL;
    return;
}

void allocator_destroy_exit(){
    if(allocator == NULL){
        return;
    }
    munmap(allocator->memory - sizeof(Allocator), allocator->size + sizeof(Allocator));
    allocator = NULL;
    return;
}

void* allocator_alloc(const Allocator* allocator, const size_t size){
    if(allocator == NULL){
        return NULL;
    }
    struct Block* empty = allocator->head_of_blocks, *prev = empty;
    int mode = 0;
    size_t allocated = 0, empty_memo_between = 0;

    int res = (char*)empty->next - (char*)empty - sizeof(struct Block);

    if(empty->size == 0 && (char*)empty->next - (char*)empty - sizeof(struct Block) >= size){
        empty->size = size;
        return (void*)((char*)empty + sizeof(struct Block));
    }

    while(empty->next != NULL){
        prev = empty;
        
        allocated += (prev->size + sizeof(struct Block));
        
        empty = empty->next;

        size_t delta = empty - prev - sizeof(struct Block) - prev->size;
        empty_memo_between += delta <= 0? 0: delta; 

        mode = empty->next == NULL ? 0 : 1;
        if(empty != prev && (char*)empty - (char*)prev - sizeof(struct Block) - prev->size >= size){
            break;
        }
    }
    if(mode){
        struct Block *delta_block;
        delta_block = (struct Block*)((char*)prev + prev->size + sizeof(struct Block));
        prev->next = delta_block;
        delta_block->size = size;
        delta_block->next = empty;
        return (void*)((char*)delta_block + sizeof(struct Block));
    }
    else{

        if(allocator->size - allocated - sizeof(struct Block) - empty_memo_between <= size){
            allocator = allocator_resize(allocator, allocator->size * 2 - allocated < size ? allocator->size * 2 + size : allocator->size * 2); 
        }

        struct Block *next_block;
        next_block = (struct Block*)((char*)empty + sizeof(struct Block) + size);
        size_t empty_memory = empty->size;
        empty->size = size;
        empty->next = next_block;
        next_block->size = empty_memory - sizeof(struct Block) - size;
        next_block->next = NULL;
        return (void*)((char*)empty + sizeof(struct Block));
    }
}

void allocator_free(const Allocator* allocator, const void* memory){
    if(allocator == NULL){
        return;
    }
    else if(allocator->head_of_blocks->next == NULL){
        my_write("The memory that should have been deleted not found!\n");
        exit(-1);
        return;
    }
    struct Block* delete_block = (struct Block*)((char*)memory - sizeof(struct Block)), *search = (struct Block*)allocator->head_of_blocks, *prev = search; //а если head_of_blocks освободили уже???
    while(search != delete_block && search != NULL){
        prev = search;
        search = search->next;
    }
    if(search == NULL){
        my_write("The memory that should have been deleted not found!\n");
        exit(-1);
        return;
    }
    else if(search == allocator->head_of_blocks)
    {
        if(search->next != NULL){
            search->size = 0;
            return;
        }else{
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
        allocator = allocator_create(NULL, 1024 * 1024 * 1024 * 1.75);
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