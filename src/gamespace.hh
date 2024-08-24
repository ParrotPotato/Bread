#ifndef GAMESPACE_HH
#define GAMESPACE_HH

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "memory.hh"

#include "physics.hh"

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

struct SpriteSheet{
    Texture2D * texture;
    unsigned int x_max;
    unsigned int y_max;
};

struct SpriteTextureInfo{
    glm::vec2  pos;
    glm::vec2  dim;
};

struct StaticWorldInformation{
    // if it is -1 that don't draw anything

    int * static_indices;

    unsigned int space_width;
    unsigned int space_height;
};

struct LevelEditor{ 
    StaticWorldInformation world_info;

    // note: this will later become a list of sprite-sheets 
    // for now this is a single one

    SpriteSheet * sprite;

    float per_sprite_width;
    float per_sprite_height;

    int selected_sprite_x;
    int selected_sprite_y;

};


struct Camera2D {
    glm::mat4 projection;
    glm::mat4 inverse;

    float xresolution;
    float yresolution;

    glm::vec2 position;
};

#define TILE_PLACEMENT    1 << 0
#define TILE_SELECTION    1 << 1

struct GameMemory{
    Renderer2D game_renderer;
    Renderer2D static_ui_renderer;
    LevelEditor level_editor;
    Texture2D plain_texture;
    Texture2D tile_texture;

    Camera2D camera;

    glm::mat4 static_ortho_projection;

    SpriteSheet sprite_sheet;

    MemoryArena permanent;
    MemoryArena temporary;

    float xresolution;
    float yresolution;

    int current_ui;
    GLint  p1, p2, p3, p4;

    // physics debiging starts

    bool rotating;

    BoxCollider b1;
    BoxCollider b2;

    // physics debugging ends
};

typedef void (*gamespace_update_function_t)(MemoryBlock * block);
typedef void (*gamespace_init_function_t)(MemoryBlock * block);


#endif // GAMESPACE_HH
