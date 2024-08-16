#ifndef MEMORY_HH
#define MEMORY_HH

#include <cstdlib>

#define KB(x)  (1024 * x)
#define MB(x)  (1024 * KB(x))
#define GB(x)  (1024 * MB(x))

#define ALLOCATE_ARRAY(arena, type, count) (type *) push_value_to_arena(arena, count * sizeof(type), alignof(type))
#define ALLOCATE_STRUCT(arena, type) (type *) push_value_to_arena(arena, sizeof(type), alignof(type))

#define GET_ALIGNMENT_OFFSET(pointer, type)  get_alignment_offset(pointer, sizeof(type), alignof(type))
#define GET_ALIGNMENT_POINTER(pointer ,type) (type *) get_alignment_pointer(pointer, sizeof(type), alignof(type))
#define RESET_ARENA(arena) reset_arena_to_zero(arena);


struct MemoryBlock{
    void * ptr;
    size_t size;
};

struct MemoryArena{
    void * ptr;
    size_t size;
    size_t cur;
};

size_t get_alignment_offset(void * ptr, size_t size, size_t alignment);
void * get_alignment_pointer(void * ptr, size_t size, size_t alignment);

void * push_value_to_arena(MemoryArena * arena, size_t size, size_t  alignment);
void reset_arena_to_zero(MemoryArena * arena);

#endif
