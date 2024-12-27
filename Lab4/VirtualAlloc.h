#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h> 

struct Block {
    size_t size;       
    struct Block *next; 
};

typedef struct {
    void *memory;               
    size_t size;                
    struct Block *head_of_blocks; 
}Allocator;

Allocator* allocator_create(void* memory, const size_t size);
void allocator_destroy(const Allocator* allocator);

void allocator_destroy_exit();

void* allocator_alloc(const Allocator* allocator, const size_t size);

void allocator_free(const Allocator* allocator, const void* memory);

void* allocate(const size_t size);

void deallocate(const void* memory);

#endif 