#ifndef GAMESPACE_HH
#define GAMESPACE_HH

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "memory.hh"

struct Renderer2D {
    float * mem_vertex_buffer = 0;
    float * mem_color_buffer = 0;
    float * mem_uv_coord_buffer = 0;
    unsigned int * mem_index_buffer = 0;

    size_t added_vertices = 0;
    size_t added_indices = 0;
    size_t added_colors = 0;
    size_t added_uv_coords = 0;

    size_t total_colors = 0;
    size_t total_vertices = 0;
    size_t total_indices = 0;
    size_t total_uv_coords = 0;


    GLuint vbo = 0;
    GLuint cbo = 0;
    GLuint uvo = 0;

    GLuint ibo = 0;
    GLuint vao = 0;

    int state = 0;
};

struct Texture2D{
    GLuint          id;
    unsigned int    width;
    unsigned int    height;
    unsigned int    components;
};


struct GameMemory{
    Renderer2D renderer;
    glm::mat4 ortho_projection;
    Texture2D tile_texture;

    MemoryArena permanent;
    MemoryArena temporary;

    float xresolution;
    float yresolution;

    int counter;
    GLint  p1, p2, p3, p4;
};

typedef void (*gamespace_update_function_t)(MemoryBlock * block);
typedef void (*gamespace_init_function_t)(MemoryBlock * block);


#endif // GAMESPACE_HH
