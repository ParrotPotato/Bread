#include "memory.hh"



// TODO: getting offset of the pointer from this type
size_t get_alignment_offset(void * ptr, size_t size, size_t alignment){
    size_t offset = 0;
    char * base_ptr = (char *) ptr;
    if ((size_t) base_ptr % alignment){
        offset = (((size_t) base_ptr + alignment - 1) & !(alignment - 1)) - (size_t) base_ptr;
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
        offset = (((size_t) present_ptr + alignment - 1) & !(alignment - 1)) - (size_t) present_ptr;
    }

    // RETURNING FROM THIS POINT BECAUSE ARENA CAPACITY REACHED
    if (offset + arena->cur + size > arena->size){
        return NULL;
    }

    result = (void *) ((size_t) present_ptr + offset);
    arena->cur = arena->cur + (size_t) result - (size_t) present_ptr + size;
    return result;
}

void reset_arena_to_zero(MemoryArena * arena){
    // question : should we reset the memory 
    //  to 0 as well for mem reset
    arena->cur = 0;
}
