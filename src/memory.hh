#ifndef MEMORY_HH
#define MEMORY_HH

#include <cstdlib>


#define ALLOCATE_ARRAY(arena, count, type) (type *) push_value_to_arena(arena, count * sizeof(type), alignof(type))

struct SuperArena{
    void * ptr;
    size_t size;
};

struct Arena{
    void * ptr;
    size_t size;
    size_t cur;
};

void * push_value_to_arena(Arena * arena, size_t size, size_t  alignment);

#endif
