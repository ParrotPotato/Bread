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

// General functions
size_t get_alignment_offset(void * ptr, size_t size, size_t alignment);
void * get_alignment_pointer(void * ptr, size_t size, size_t alignment);

// Arean fucntions
struct MemoryArena{
    void * ptr;
    size_t size;
    size_t cur;
};

void * push_value_to_arena(MemoryArena * arena, size_t size, size_t alignment);
void * pop_value_from_arena(MemoryArena *arena, size_t size, size_t alignment);


// stack based allocation

#define PUSH_IN_STACK(stack, type, count)  (type *) push_in_stack(stack, sizeof(type) * count, alignof(type))
#define POP_FROM_STACK(stack)                       pop_from_stack(stack)

#define MAX_ALLOCATION   128
struct MemoryStackAllocator{
    void * allocations[MAX_ALLOCATION];
    unsigned int allocation_count;

    void * base;
    size_t used;
    size_t size;
};

void init_stack_allocator(MemoryStackAllocator * allocator, void * ptr, size_t size);
void * push_in_stack(MemoryStackAllocator * allocator, size_t size, size_t alignment);
void pop_from_stack(MemoryStackAllocator * allocator);
void reset_stack_allocator( MemoryStackAllocator * allocator);

void reset_arena_to_zero(MemoryArena * arena);


#endif
