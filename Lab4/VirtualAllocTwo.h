#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <sys/mman.h>

#define POWERS_AMOUNT 25

struct Block {
    size_t size;                
    struct Block* next;         
};

struct Header {
    void* memory;               
    size_t power;               
    size_t size;                
    struct Block* head;         
};

typedef struct {
    size_t size_of_block;       
    struct Header* head[POWERS_AMOUNT]; 
} Allocator;


Allocator* allocator_create(const size_t size);

void allocator_destroy(const Allocator* allocator);

void allocator_destroy_exit();

void* allocator_alloc(const Allocator* allocator, const size_t size);

void allocator_free(const Allocator* allocator, const void* memory);

void* allocate(const size_t size);

void deallocate(const void* memory);

#endif 