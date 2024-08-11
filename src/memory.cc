#include "memory.hh"


// TODO: how to handle memory clearing

void * push_value_to_arena(Arena * arena, size_t size, size_t alignment){
    void * result = 0;

    char * base_ptr= (char *) arena->ptr;
    char * present_ptr = base_ptr + arena->cur;

    size_t offset =  0;
    if ((size_t) present_ptr % alignment){
        offset = (((size_t) present_ptr + alignment - 1) & !(alignment - 1)) - (size_t) present_ptr;
    }

    result = (void *) ((size_t) present_ptr + offset);
    arena->cur = arena->cur + (size_t) result - (size_t) present_ptr + size;
    return result;
}
