#include "memory.hh"

#include <cstdio>
#include <cstring>
#include <cstdlib>

///////////// AREAN BASED PERMANENT ALLOCATOR ///////////////////////////

// TODO: getting offset of the pointer from this type
size_t get_alignment_offset(void * ptr, size_t size, size_t alignment){
    size_t offset = 0;
    char * base_ptr = (char *) ptr;
    if ((size_t) base_ptr % alignment){
        offset = (((size_t) base_ptr + alignment - 1) & ~(alignment - 1)) - (size_t) base_ptr;
    }
    return offset;
}

// TODO: getting pointer which is aligned to this type
void * get_alignment_pointer(void * ptr, size_t size, size_t alignment){
    char * base_ptr = (char *) ptr;
    base_ptr = base_ptr + get_alignment_offset(ptr, size, alignment);
    return base_ptr;
}

// TODO: how to handle memory clearing
void * push_value_to_arena(MemoryArena * arena, size_t size, size_t alignment){
    void * result = 0;

    char * base_ptr= (char *) arena->ptr;
    char * present_ptr = base_ptr + arena->cur;

    size_t offset =  0;
    if ((size_t) present_ptr % alignment){
        offset = (((size_t) present_ptr + alignment - 1) & ~(alignment - 1)) - (size_t) present_ptr;
    }

    // RETURNING FROM THIS POINT BECAUSE ARENA CAPACITY REACHED
    if (offset + arena->cur + size > arena->size){
        return NULL;
    }

    result = (void *) ((size_t) present_ptr + offset);
    arena->cur = arena->cur + (size_t) result - (size_t) present_ptr + size;
    return result;
}

// Stack based allocator

void init_stack_allocator(MemoryStackAllocator * allocator, void * ptr, size_t size){
    for(unsigned int i = 0 ; i < MAX_ALLOCATION ; i++){
        allocator->allocations[i] = nullptr;
    }
    allocator->allocation_count = 0;

    allocator->base = ptr;
    allocator->size = size;
    allocator->used = 0;
}

void * push_in_stack(MemoryStackAllocator * allocator, size_t size, size_t alignment){
    // 1. check if there is space left for this allocation :: mem
    // 2. check if there is slot left for this allocation  :: stack
    
    void * result = nullptr;
    void * current = (void *) ((char *) allocator->base + allocator->used);
    size_t adjusted_size = size +  get_alignment_offset(current, size, alignment);

    if (adjusted_size > allocator->size - allocator->used){
        printf("ERROR: allocation failed, insufficient space in allocator\n");
        return nullptr;
    }

    if (allocator->allocation_count == MAX_ALLOCATION){
        printf("ERROR: allocation failed, insufficient slots in allocator\n");
        return nullptr;
    }

    allocator->allocations[allocator->allocation_count] = current;
    allocator->allocation_count += 1;

    result = get_alignment_pointer(current, size, alignment);
    allocator->used += adjusted_size;

    return result;
}


// @note: check if we can do memset here as well, for data sanity

void pop_from_stack(MemoryStackAllocator * allocator){

    if (allocator->allocation_count == 0){
        printf("ERROR : deallocation called before allocation\n");
        return;
    }

    void * top = allocator->allocations[allocator->allocation_count - 1];
    allocator->used = (char *) top - (char *) allocator->base;
    allocator->allocation_count -= 1;
}

void reset_stack_allocator( MemoryStackAllocator * allocator){
    memset(allocator->allocations,  0, sizeof(allocator->allocations[0]) * MAX_ALLOCATION);
    allocator->allocation_count = 0;
    allocator->used = 0;
}




void reset_arena_to_zero(MemoryArena * arena){
    // question : should we reset the memory 
    //  to 0 as well for mem reset
    arena->cur = 0;
}
