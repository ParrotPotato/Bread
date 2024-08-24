/*
 * Filename     : gamespace.cc
 * Author       : Nitesh Meena (niteshmeena698@gmail.com)
 * Description  : 
 * This file in the entry point for the game logic and state-management
 * It exposes 2 function 
 * - gamespace_init_function : called for initializing the game memory
 * - gamespace_update_function : called as the main game update loop
 * 
 * Tasks : 
 * 1.    Basic OpenGL setup and rendering - done
 * 2.    Basic Texture rendering          - done
 * 3.    ImGui integration for debug gui  - done 
 * 4.    Hot reloading game code          - done
 * 5.    Sprite sheet instancing and load - done
 *          1. Add depth based rendering because we have multiple layeres 
 *             of transparent and translucent stuff being drawn as part of the 
 *             gui for texture updates
 * 6.    Level editor and world generator - done
 *          1. This needs some improvement in the basic world editing and
 *             where should shoul be to run the game, pause the game, edit 
 *             the world and then resume the world again
 * 7.    Basic collisions                 - todo
 * 8.    Player movement                  - todo
 * 9.    Enemy movement                   - todo
 * 10.   Enemy AI                         - todo
 * 11.   Sprite sample animation          - todo
 * 12.   Proper memory boundary tests     - todo
 * 13.   (Platform) Audio system          - todo
 *
 * Maintainace Tasks :
 * 1. Evaluate the effects of coordinate system 
 *      - update the coordinate system based on what feels nicer
 *
 * Description of tashs which are not clear by themselves. 
 *
 * 1. Sprite sample animation 
 *      animation of sprite hands with the circular dots which are 
 *      present in the sprite pack
 *      animation of the wepon being held
 *
 * 2. Memory boundary test 
 *      setting a dummy values at the end of allocator
 *      boundaries and then checking if they have been modified 
 *      to check memory leaks
 *
 * 3. Audio system 
 *      Playing audio clips and modulating their amplitudes
 *      based on distance and may occlusion (the occlusion is 
 *      not required right now) 
 */

#include "gamespace.hh"

#include "platform.hh"

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include "memory.hh"


#define QUADCOUNT 20000

const char * v1 = ""
"#version 400 core\n"
"layout (location = 0) in vec3 position;\n\n"
"void main(){\n"
"   gl_Position = vec4(position, 1.0);\n"
"}\n";

const char * f1 = ""
"#version 400 core\n"
"out vec4 fragcolor;\n"
"void main(){\n"
"   fragcolor = vec4(0.8, 0.2, 0.3, 1.0);\n"
"}\n";

const char * v2 = ""
"#version 400 core\n"
"layout (location = 0) in vec2 position;\n\n"
"layout (location = 1) in vec4 color;\n\n"
"out vec4 vertcolor;\n"
"void main(){\n"
"   gl_Position = vec4(position, 0.0, 1.0);\n"
"   vertcolor= color;\n"
"}\n";

const char * f2 = ""
"#version 400 core\n"
"out vec4 fragcolor;\n"
"in  vec4 vertcolor;\n"
"void main(){\n"
"   fragcolor = vertcolor;\n"
"}\n";

const char * v3 = ""
"#version 400 core\n"
"layout (location = 0) in vec2 position;\n\n"
"layout (location = 1) in vec4 color;\n\n"
"out vec4 vertcolor;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"   gl_Position = projection * vec4(position, 0.0, 1.0);\n"
"   vertcolor= color;\n"
"}\n";

const char * f3 = ""
"#version 400 core\n"
"out vec4 fragcolor;\n"
"in  vec4 vertcolor;\n"
"void main(){\n"
"   fragcolor = vertcolor;\n"
"}\n";

const char * v4 = ""
"#version 400 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec4 color;\n"
"layout (location = 2) in vec2 uv;\n\n"
"out vec4 vertcolor;\n"
"out vec2 vertuv;\n\n"
"uniform mat4 projection;\n\n"
"void main(){\n"
"   gl_Position = projection * vec4(position, 0.0, 1.0);\n"
"   vertcolor   = color;\n"
"   vertuv      = uv;\n"
"}\n";

const char * f4 = ""
"#version 400 core\n"
"out vec4 fragcolor;\n"
"in  vec4 vertcolor;\n"
"in  vec2 vertuv;\n"
"uniform sampler2D spriteTexture;\n"
"void main(){\n"
"   fragcolor = texture(spriteTexture, vertuv) * vertcolor;\n"
"}\n";

/////////// math functions  //////////////////////

int clamp_int(int value, int min, int max){
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int clamp_float(float value, float min, float max){
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int compile_shader(GameMemory * memory, char * shader_buffer, GLenum type){
    int shader_handle = glCreateShader(type);
    glShaderSource(shader_handle, 1, &shader_buffer, 0);
    glCompileShader(shader_handle);
    GLint status = 0;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE){
        int error_log_length = 0;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &error_log_length);
        char * error_log = ALLOCATE_ARRAY(&memory->temporary, char, error_log_length + 1);
        glGetShaderInfoLog(shader_handle, error_log_length, &error_log_length, error_log);
        printf("shader compilation error\n");
        printf("%s\n", error_log);
        free(error_log);
        glDeleteShader(shader_handle);
        shader_handle = 0;
    }
    return shader_handle;
}

int link_program(GameMemory * memory, int * shader_list, int shader_count){
    GLint program = glCreateProgram();
    for(int i = 0; i < shader_count ; i++) glAttachShader(program, shader_list[i]);
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) 
    {
        int error_log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &error_log_length);
        char * error_log = ALLOCATE_ARRAY(&memory->temporary, char, error_log_length  + 1);
        glGetProgramInfoLog(program, error_log_length, &error_log_length, error_log);
        printf("program linking failed\n");
        printf("%s\n", error_log);
        free(error_log);
        for(int i = 0; i < shader_count ; i++) glDetachShader(program, shader_list[i]);
        glDeleteProgram(program);
        program = 0;
    }
    return program;
}

int load_plain_texture(Texture2D * texture_ref){
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char buffer[4] = { (unsigned int) (0xff) , (unsigned int) (0xff) , (unsigned int) (0xff) , (unsigned int) (0xff) };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    texture_ref->id = texture_id;
    texture_ref->width = 1;
    texture_ref->height = 1;
    texture_ref->components = 4;
    return 0;
}

int load_texture_memory(Texture2D* texture_ref, const char * filepath)
{
    stbi_set_flip_vertically_on_load(true);
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    int width, height;
    int component;
    // all the file loading and unloading should happen from the same function
    unsigned char * image = stbi_load(filepath, &width, &height, &component, 0);

    if (!image) {
        printf("failed to load parse image from specified buffer\n");
        return -1;
    }
    
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (component == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);

    texture_ref->id= texture_id;
    texture_ref->width = width;
    texture_ref->height = height;
    texture_ref->components = component;

    return 0;
}


void init_renderer(GameMemory * memory, Renderer2D * renderer, int quad_count = QUADCOUNT){
    renderer->mem_vertex_buffer  = ALLOCATE_ARRAY(&memory->permanent, float,        quad_count * 8);
    renderer->mem_uv_coord_buffer= ALLOCATE_ARRAY(&memory->permanent, float,        quad_count * 8);
    renderer->mem_color_buffer   = ALLOCATE_ARRAY(&memory->permanent, float,        quad_count * 16);
    renderer->mem_index_buffer   = ALLOCATE_ARRAY(&memory->permanent, unsigned int, quad_count * 6);

    renderer->total_vertices = quad_count* 8;
    renderer->total_uv_coords = quad_count * 8;
    renderer->total_colors = quad_count * 16;
    renderer->total_indices  = quad_count * 6;

    glGenBuffers(1, &renderer->vbo);
    glGenBuffers(1, &renderer->ibo);
    glGenBuffers(1, &renderer->uvo);
    glGenBuffers(1, &renderer->cbo);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * renderer->total_vertices, renderer->mem_vertex_buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->cbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * renderer->total_colors , renderer->mem_color_buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->uvo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * renderer->total_uv_coords, renderer->mem_uv_coord_buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * renderer->total_indices, renderer->mem_index_buffer, GL_STATIC_DRAW);

    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->cbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->uvo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float) , (void *)0);
    glBindVertexArray(0);
}

void start_rendering(Renderer2D * renderer){
    renderer->added_indices = 0;
    renderer->added_vertices = 0;
    renderer->added_uv_coords = 0;
    renderer->added_colors = 0;
}


void render_quad_rect_tex_rot(
        Renderer2D * renderer,
        glm::vec2 pos,
        glm::vec2 dim,
        glm::vec4 color,
        glm::vec2 uv_pos,
        glm::vec2 uv_dim,
        glm::vec2 center,
        float rot){
    if (
            renderer->added_vertices >= renderer->total_vertices 
            || renderer->added_colors >= renderer->total_colors 
            || renderer->added_indices >= renderer->total_indices
       ){
        printf("renderer :: buffer entirly full\n");
        return;
    } 

    glm::vec2 rect[4];
    rect[0] = glm::vec2(pos.x, pos.y);
    rect[1] = glm::vec2(pos.x, pos.y + dim.y);
    rect[2] = glm::vec2(pos.x + dim.x, pos.y + dim.y);
    rect[3] = glm::vec2(pos.x + dim.x, pos.y);

    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0.0f, 0.0f, 1.0f));

    for(unsigned int i = 0; i < 4 ; i++){
        rect[i] = center + glm::vec2(rotation_matrix *  glm::vec4((rect[i] - center), 0.0, 1.0));
    }

    glm::vec4 colors[4] = { color,color,color,color };
    
    glm::vec2 uv[4];
    uv[0] = glm::vec2(uv_pos.x, uv_pos.y);
    uv[1] = glm::vec2(uv_pos.x, uv_pos.y + uv_dim.y);
    uv[2] = glm::vec2(uv_pos.x + uv_dim.x, uv_pos.y + uv_dim.y);
    uv[3] = glm::vec2(uv_pos.x + uv_dim.x, uv_pos.y);

    unsigned int indices[6] = {
        0 + (unsigned int) renderer->added_vertices / 2, 
        1 + (unsigned int) renderer->added_vertices / 2,
        2 + (unsigned int) renderer->added_vertices / 2,
        0 + (unsigned int) renderer->added_vertices / 2,
        2 + (unsigned int) renderer->added_vertices / 2,
        3 + (unsigned int) renderer->added_vertices / 2,
    };


    memcpy(renderer->mem_vertex_buffer + renderer->added_vertices, rect,  sizeof(float) * 2 * 4);
    renderer->added_vertices += 2 * 4;

    memcpy(renderer->mem_color_buffer + renderer->added_colors, colors, sizeof(float) * 4 * 4);
    renderer->added_colors += 4 * 4;

    memcpy(renderer->mem_uv_coord_buffer + renderer->added_uv_coords, uv, sizeof(float) * 2 * 4);
    renderer->added_uv_coords += 2 * 4;

    memcpy(renderer->mem_index_buffer + renderer->added_indices, indices, sizeof(unsigned int) * 6);
    renderer->added_indices += 6;
}



// TODO: removce the default arguments in the uv coordinates
void render_quad_rect_tex(
        Renderer2D * renderer, 
        glm::vec2 pos, 
        glm::vec2 dim, 
        glm::vec4 color,
        glm::vec2 uv_pos, 
        glm::vec2 uv_dim
        ){
    if (
            renderer->added_vertices >= renderer->total_vertices 
            || renderer->added_colors >= renderer->total_colors 
            || renderer->added_indices >= renderer->total_indices
       ){
        printf("renderer :: buffer entirly full\n");
        return;
    } 

    glm::vec2 rect[4];
    rect[0] = glm::vec2(pos.x, pos.y);
    rect[1] = glm::vec2(pos.x, pos.y + dim.y);
    rect[2] = glm::vec2(pos.x + dim.x, pos.y + dim.y);
    rect[3] = glm::vec2(pos.x + dim.x, pos.y);

    glm::vec4 colors[4] = { color,color,color,color };
    
    glm::vec2 uv[4];
    uv[0] = glm::vec2(uv_pos.x, uv_pos.y);
    uv[1] = glm::vec2(uv_pos.x, uv_pos.y + uv_dim.y);
    uv[2] = glm::vec2(uv_pos.x + uv_dim.x, uv_pos.y + uv_dim.y);
    uv[3] = glm::vec2(uv_pos.x + uv_dim.x, uv_pos.y);

    unsigned int indices[6] = {
        0 + (unsigned int) renderer->added_vertices / 2, 
        1 + (unsigned int) renderer->added_vertices / 2,
        2 + (unsigned int) renderer->added_vertices / 2,
        0 + (unsigned int) renderer->added_vertices / 2,
        2 + (unsigned int) renderer->added_vertices / 2,
        3 + (unsigned int) renderer->added_vertices / 2,
    };


    memcpy(renderer->mem_vertex_buffer + renderer->added_vertices, rect,  sizeof(float) * 2 * 4);
    renderer->added_vertices += 2 * 4;

    memcpy(renderer->mem_color_buffer + renderer->added_colors, colors, sizeof(float) * 4 * 4);
    renderer->added_colors += 4 * 4;

    memcpy(renderer->mem_uv_coord_buffer + renderer->added_uv_coords, uv, sizeof(float) * 2 * 4);
    renderer->added_uv_coords += 2 * 4;

    memcpy(renderer->mem_index_buffer + renderer->added_indices, indices, sizeof(unsigned int) * 6);
    renderer->added_indices += 6;
}

void render_quad_rect(Renderer2D * renderer, glm::vec2 pos, glm::vec2 dim, glm::vec4 color){

    if (
            renderer->added_vertices >= renderer->total_vertices 
            || renderer->added_colors >= renderer->total_colors 
            || renderer->added_indices >= renderer->total_indices
       ){
        printf("renderer; buffer entirely full\n");
        return;
    } 

    glm::vec2 rect[4];
    rect[0] = glm::vec2(pos.x, pos.y);
    rect[1] = glm::vec2(pos.x, pos.y + dim.y);
    rect[2] = glm::vec2(pos.x + dim.x, pos.y + dim.y);
    rect[3] = glm::vec2(pos.x + dim.x, pos.y);

    glm::vec4 colors[4] = { color,color,color,color };

    unsigned int indices[6] = {
        0 + (unsigned int) renderer->added_vertices / 2, 
        1 + (unsigned int) renderer->added_vertices / 2,
        2 + (unsigned int) renderer->added_vertices / 2,
        0 + (unsigned int) renderer->added_vertices / 2,
        2 + (unsigned int) renderer->added_vertices / 2,
        3 + (unsigned int) renderer->added_vertices / 2,
    };

    memcpy(renderer->mem_vertex_buffer + renderer->added_vertices, rect,  sizeof(float) * 2 * 4);
    renderer->added_vertices += 2 * 4;

    memcpy(renderer->mem_color_buffer + renderer->added_colors, colors, sizeof(float) * 4 * 4);
    renderer->added_colors += 4 * 4;

    memcpy(renderer->mem_index_buffer + renderer->added_indices, indices, sizeof(unsigned int) * 6);
    renderer->added_indices += 6;
}

void end_rendering(Renderer2D * renderer){
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * renderer->added_vertices, (void *) renderer->mem_vertex_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->cbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * renderer->added_colors, (void *) renderer->mem_color_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->uvo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * renderer->added_uv_coords, (void *) renderer->mem_uv_coord_buffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * renderer->added_indices, (void *) renderer->mem_index_buffer);
}


void draw(Renderer2D * renderer){
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glDrawElements(GL_TRIANGLES, renderer->added_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void generate_camera_matrix(Camera2D * camera){
    glm::mat4 ortho = glm::ortho(0.0f, camera->xresolution, 0.0f, camera->yresolution, 0.0f, 1000.0f);
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(camera->position, 0.0f));


    glm::ivec2 window_size = get_window_size();
    camera->projection = ortho * translate;

    glm::mat4 inverse_translation = glm::translate(glm::mat4(1.0f), -glm::vec3(camera->position, 0.0f));
    glm::vec3 scale_factor = glm::vec3(((float) camera->xresolution) / ((float) window_size.x), ((float) camera->yresolution) / ((float) window_size.y), 1.0f);
    camera->inverse = glm::scale(inverse_translation, scale_factor);
}

extern "C"    
void gamespace_init_function(MemoryBlock * gspace_mem){

    // Memory configuration

    size_t game_memory_offset = GET_ALIGNMENT_OFFSET(gspace_mem->ptr, GameMemory);
    printf("offset : %lu alignment : %lu pointer : %lu currentDiff : %lu  \n",
            game_memory_offset,
            alignof(GameMemory),
            (size_t) gspace_mem->ptr,
            (size_t) gspace_mem->ptr % alignof(GameMemory));
    GameMemory * game_mem = GET_ALIGNMENT_POINTER(gspace_mem->ptr, GameMemory);

    size_t remaining_size = gspace_mem->size - game_memory_offset - sizeof(GameMemory);

    game_mem->permanent.ptr =  (char *) game_mem + sizeof(GameMemory);
    game_mem->permanent.size =  remaining_size - remaining_size / 2;
    game_mem->permanent.cur = 0;

    game_mem->temporary.ptr = (char *) game_mem + sizeof(GameMemory) + game_mem->permanent.size;
    game_mem->temporary.size = remaining_size / 2;
    game_mem->temporary.cur = 0;

    // Resource loading
    game_mem->game_renderer = {0};
    init_renderer(game_mem, &game_mem->game_renderer);

    game_mem->static_ui_renderer = {0};
    init_renderer(game_mem, &game_mem->static_ui_renderer);

    // Shader compilation

    GLint v1s = compile_shader(game_mem, (char *)v1, GL_VERTEX_SHADER);
    GLint v2s = compile_shader(game_mem, (char *)v2, GL_VERTEX_SHADER);
    GLint v3s = compile_shader(game_mem, (char *)v3, GL_VERTEX_SHADER);
    GLint v4s = compile_shader(game_mem, (char *)v4, GL_VERTEX_SHADER);

    GLint f1s = compile_shader(game_mem, (char *)f1, GL_FRAGMENT_SHADER);
    GLint f2s = compile_shader(game_mem, (char *)f2, GL_FRAGMENT_SHADER);
    GLint f3s = compile_shader(game_mem, (char *)f3, GL_FRAGMENT_SHADER);
    GLint f4s = compile_shader(game_mem, (char *)f4, GL_FRAGMENT_SHADER);

    GLint s1[] = {v1s, f1s};
    GLint s2[] = {v2s, f2s};
    GLint s3[] = {v3s, f3s};
    GLint s4[] = {v4s, f4s};

    GLint p1 = link_program(game_mem, s1, 2);
    GLint p2 = link_program(game_mem, s2, 2);
    GLint p3 = link_program(game_mem, s3, 2);
    GLint p4 = link_program(game_mem, s4, 2);

    game_mem->p1 = p1;
    game_mem->p2 = p2;
    game_mem->p3 = p3;
    game_mem->p4 = p4;

    // game specific code
    
    Texture2D plain_texture = {};
    if(load_plain_texture(&plain_texture)){
        printf("failed to load plain texture\n");
    }
    const char * texture_path = "/home/nitesh/work/projects/active/beta-one/data/kenney_scribble-platformer/Spritesheet/spritesheet_retina.png";
    Texture2D sprite_sheet_texture = {};
    if(load_texture_memory( &sprite_sheet_texture, texture_path)) {
        printf("failed to load texture %s\n", texture_path);
    }

    printf("texture loaded: width = %u, height %u\n", sprite_sheet_texture.width, sprite_sheet_texture.height);

    
    game_mem->tile_texture  = sprite_sheet_texture;
    game_mem->plain_texture = plain_texture;

    game_mem->sprite_sheet = {0};
    game_mem->sprite_sheet.texture = &game_mem->tile_texture;
    game_mem->sprite_sheet.x_max = 11;
    game_mem->sprite_sheet.y_max = 11;

    const float xresolution = 800;
    const float yresolution = 600;

    game_mem->xresolution = 800;
    game_mem->yresolution = 600;

    Camera2D * camera = &game_mem->camera;
    camera->position = glm::vec2(0.0f);
    camera->xresolution = game_mem->xresolution;
    camera->yresolution = game_mem->yresolution;
    camera->projection = glm::mat4(1.0);
    generate_camera_matrix(&game_mem->camera);

    game_mem->static_ortho_projection =  glm::ortho(0.0f, xresolution, 0.0f, yresolution, 0.0f, 1000.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    StaticWorldInformation * world = &game_mem->level_editor.world_info;
    world->space_width = 120;
    world->space_height= 80;
    world->static_indices = ALLOCATE_ARRAY(&game_mem->permanent, int, world->space_width * world->space_height);

    // FIXME : this does not feel right but this can be done
    for(unsigned int i = 0 ; i < world->space_width * world->space_height ; i++){
        world->static_indices[i] = -1;
    }

    LevelEditor * editor = &game_mem->level_editor;
    editor->selected_sprite_x = 0;
    editor->selected_sprite_y = 0;
    editor->per_sprite_width = 50;
    editor->per_sprite_height= 50;
    editor->sprite = &game_mem->sprite_sheet;
    

    // initailize currnet game state
    game_mem->current_ui = 0;

    game_mem->b1 = {};
    game_mem->b2 = {};

    game_mem->b1.pos = glm::vec2(250, 250);
    game_mem->b1.dim = glm::vec2(100, 100);
    game_mem->b1.center = (game_mem->b1.pos);
    game_mem->b1.rot = 0.0f;

    game_mem->b2.pos = glm::vec2(0, 0);
    game_mem->b2.dim = glm::vec2(100, 100);
    game_mem->b2.center = (game_mem->b2.pos);
    game_mem->b1.rot = 0.0f;

    game_mem->rotating = false;

    RESET_ARENA(&game_mem->temporary);
}

glm::vec2 window_to_world_pos(GameMemory * pointer, glm::vec2 pos){
    glm::vec2 worldmousepos = glm::vec2(0.0);
    pos.y = get_window_size().y - pos.y;
    glm::vec4 _worldmousesposition = pointer->camera.inverse * glm::vec4(pos, 0.0f, 1.0f);
    worldmousepos = _worldmousesposition;
    return worldmousepos;
}

void render_static_world(GameMemory * pointer){
    printf("render_static_world :: functionality not immplemented\n");
}

void render_world(GameMemory * pointer){
    StaticWorldInformation * world =  &pointer->level_editor.world_info;
    LevelEditor * editor = &pointer->level_editor;

    unsigned int sprite_x_max = editor->sprite->x_max;
    unsigned int sprite_y_max = editor->sprite->y_max;

    GLint projectionLoadtion = glGetUniformLocation(pointer->p4,  "projection");
    glUseProgram(pointer->p4);
    glUniformMatrix4fv(projectionLoadtion, 1, GL_FALSE, glm::value_ptr(pointer->camera.projection));
    // rendering the world 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->tile_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);
    start_rendering(&pointer->game_renderer);
    for(unsigned int i = 0 ; i < world->space_width * world->space_height ; i++){
        if (world->static_indices[i] == -1) continue;

        int pos_x_offset = i % world->space_width;
        int pos_y_offset = i / world->space_width;

        glm::vec2 target_pos = glm::vec2(pos_x_offset * editor->per_sprite_width, pos_y_offset * editor->per_sprite_height);
        glm::vec2 target_size= glm::vec2(editor->per_sprite_width, editor->per_sprite_height);


        int value = world->static_indices[i];
        unsigned int tex_x_offset = value % sprite_x_max;
        unsigned int tex_y_offset = value / sprite_x_max;
        glm::vec2 target_uv_pos = glm::vec2( ((float) tex_x_offset) / sprite_x_max, ((float) tex_y_offset) / sprite_y_max);
        glm::vec2 target_uv_size = glm::vec2( 1.0f / sprite_x_max , 1.0f/ sprite_y_max);

        render_quad_rect_tex(&pointer->game_renderer,
                target_pos,
                target_size,
                glm::vec4(1.0),
                target_uv_pos,
                target_uv_size);
    }
    end_rendering(&pointer->game_renderer);
    draw(&pointer->game_renderer);
}

void render_collision_test(GameMemory * pointer){
    // update the positions

    BoxCollider * b1 = &pointer->b1;
    BoxCollider * b2 = &pointer->b2;

    b2->pos = window_to_world_pos(pointer, mouse_window_pos());
    b2->center = window_to_world_pos(pointer, mouse_window_pos());

    if (pointer->rotating){
        b2->rot = 0.5 * 3.14 * get_ticks_since_start() / 1000;
    }

    
    // b2->rot = 0.0f;

    bool collided = check_collision(b1, b2);

    // render the positions

    GLint projectionLoadtion = glGetUniformLocation(pointer->p4,  "projection");
    glUseProgram(pointer->p4);

    glUniformMatrix4fv(projectionLoadtion, 1, GL_FALSE, glm::value_ptr(pointer->camera.projection));

    // rendering the world 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->plain_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);

    start_rendering(&pointer->game_renderer);

    {
        render_quad_rect_tex_rot(
                &pointer->game_renderer,
                b1->pos - 0.5f * b1->dim,
                b1->dim,
                glm::vec4(1.0, 1.0, 1.0, 0.5),
                glm::vec2(0.0),
                glm::vec2(1.0),
                b1->pos,
                b1->rot
        );
    }

    {
        glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 0.5);
        if (collided){
            color = glm::vec4(1.0, 1.0, 0.0, 0.5);
        }
        render_quad_rect_tex_rot(
                &pointer->game_renderer,
                b2->pos - 0.5f * b2->dim,
                b2->dim,
                color,
                glm::vec2(0.0),
                glm::vec2(1.0),
                b2->pos,
                b2->rot
        );
    }
    
    end_rendering(&pointer->game_renderer);
    draw(&pointer->game_renderer);
}

void render_tile_placement_gui(GameMemory * pointer){

    // processing 

    LevelEditor * editor = &pointer->level_editor;
    StaticWorldInformation * world = &pointer->level_editor.world_info;

    glm::vec2 target_uv_dim = glm::vec2(0.0f);
    glm::vec2 target_uv_pos = glm::vec2(0.0f);
    glm::ivec2 tile_indices = glm::ivec2(0);
    {
        unsigned int x_idx = pointer->level_editor.selected_sprite_x;
        unsigned int y_idx = pointer->level_editor.selected_sprite_y;
        unsigned int x_max = pointer->level_editor.sprite->x_max;
        unsigned int y_max = pointer->level_editor.sprite->y_max;

        target_uv_pos = glm::vec2(
                1.0f / x_max * (x_idx % x_max),
                1.0f / y_max * (y_idx % y_max)
                );
        target_uv_dim = glm::vec2(1.0/ x_max, 1.0 / y_max);
        tile_indices = glm::ivec2(x_idx, y_idx);
    }

    glm::vec2 mouse_target_size = glm::vec2(editor->per_sprite_width, editor->per_sprite_height);
    glm::vec2 mouse_target_pos = glm::vec2(0.0f);
    glm::ivec2 world_indices = glm::ivec2(0);
    {
        glm::vec2 world_mouse_pos = window_to_world_pos(pointer, mouse_window_pos());
        int x_pos = (int) (world_mouse_pos.x / editor->per_sprite_width) - (world_mouse_pos.x < 0 ? 1: 0);
        int y_pos = (int) (world_mouse_pos.y / editor->per_sprite_height)- (world_mouse_pos.y < 0 ? 1: 0);
        mouse_target_pos = glm::vec2(x_pos  * editor->per_sprite_width, y_pos * editor->per_sprite_height);
        world_indices = glm::ivec2(x_pos, y_pos);
    }

    // check if there has been update

    ImGui::Begin("Debug");
    ImGui::Text("world indices :: %d, %d", world_indices.x, world_indices.y);
    ImGui::Text("tile indices :: %d, %d", tile_indices.x, tile_indices.y);
    if (is_button_down(SDL_BUTTON_LEFT)){
        int world_offset = world_indices.x + world_indices.y * world->space_width;
        int tile_offset = tile_indices.x  + tile_indices.y * editor->sprite->x_max;
        if(
                world_indices.x >= world->space_width || world_indices.x < 0 || 
                world_indices.y >= world->space_height || world_indices.y < 0
          ){
            printf("world offset outside bounds skipping adding tile to world map\n");
        } else if(
                tile_indices.x >= editor->sprite->x_max || tile_indices.x < 0 || 
                tile_indices.y >= editor->sprite->y_max || tile_indices.y < 0
          ){
            printf("tile offset outside bounds skipping adding tile to world map\n"); 
        } else {
            world->static_indices[world_offset] = tile_offset;
        }
    } 
    if (is_button_down(SDL_BUTTON_RIGHT)){
        int world_offset = world_indices.x + world_indices.y * world->space_width;
        if(
                world_indices.x >= world->space_width || world_indices.x < 0 || 
                world_indices.y >= world->space_height || world_indices.y < 0
          ){
            printf("world offset outside bounds skipping adding tile to world map\n");
        } else {
            world->static_indices[world_offset] = -1;
        }
    }
    ImGui::End();

    // rendering

    GLint projectionLoadtion = glGetUniformLocation(pointer->p4,  "projection");
    glUseProgram(pointer->p4);

    glUniformMatrix4fv(projectionLoadtion, 1, GL_FALSE, glm::value_ptr(pointer->camera.projection));

    // rendering the world 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->plain_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);

    start_rendering(&pointer->game_renderer);
    
    for(unsigned int i = 0 ; i < world->space_width * world->space_height ; i++){
        if(world->static_indices[i] != -1) continue;

        int x_offset = i % world->space_width;
        int y_offset = i / world->space_width;

        glm::vec2 target_pos = glm::vec2(x_offset * editor->per_sprite_width, y_offset * editor->per_sprite_height);
        glm::vec2 target_size= glm::vec2(editor->per_sprite_width, editor->per_sprite_height);

        render_quad_rect_tex(&pointer->game_renderer,
                target_pos,
                target_size,
                glm::vec4(0.396, 0.408, 0.62, 0.4),
                glm::vec2(0.0), glm::vec2(1.0));
    }
    end_rendering(&pointer->game_renderer);
    draw(&pointer->game_renderer);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, editor->sprite->texture->id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);

    start_rendering(&pointer->game_renderer);

    unsigned int sprite_x_max = editor->sprite->x_max;
    unsigned int sprite_y_max = editor->sprite->y_max;

    ImGui::Begin("Level Render debug");

    for(unsigned int i = 0 ; i < world->space_width * world->space_height ; i++){
        if (world->static_indices[i] == -1) continue;

        int pos_x_offset = i % world->space_width;
        int pos_y_offset = i / world->space_width;

        glm::vec2 target_pos = glm::vec2(pos_x_offset * editor->per_sprite_width, pos_y_offset * editor->per_sprite_height);
        glm::vec2 target_size= glm::vec2(editor->per_sprite_width, editor->per_sprite_height);


        int value = world->static_indices[i];
        unsigned int tex_x_offset = value % sprite_x_max;
        unsigned int tex_y_offset = value / sprite_x_max;
        glm::vec2 target_uv_pos = glm::vec2( ((float) tex_x_offset) / sprite_x_max, ((float) tex_y_offset) / sprite_y_max);
        glm::vec2 target_uv_size = glm::vec2( 1.0f / sprite_x_max , 1.0f/ sprite_y_max);

        ImGui::Text("[%d] target_pos : (%f, %f), target_size: (%f, %f)", i, target_pos.x, target_pos.y, target_size.x, target_size.y);

        render_quad_rect_tex(&pointer->game_renderer,
                target_pos,
                target_size,
                glm::vec4(1.0),
                target_uv_pos,
                target_uv_size);
    }
    ImGui::End();

    render_quad_rect_tex(&pointer->game_renderer, 
            mouse_target_pos, 
            mouse_target_size,
            glm::vec4(1.0, 1.0, 0.0, 1.0),
            target_uv_pos,
            target_uv_dim
            );
    end_rendering(&pointer->game_renderer);
    draw(&pointer->game_renderer);
}


void render_tile_selection_gui(GameMemory * pointer){
    // setup
    GLint projectionLoadtion = glGetUniformLocation(pointer->p4,  "projection");
    LevelEditor * editor = &pointer->level_editor;

    glm::vec2 tile_render_size = glm::vec2(pointer->yresolution * 0.5);

    unsigned int x_idx = pointer->level_editor.selected_sprite_x;
    unsigned int y_idx = pointer->level_editor.selected_sprite_y;

    unsigned int x_max = pointer->level_editor.sprite->x_max;
    unsigned int y_max = pointer->level_editor.sprite->y_max;

    ImGui::Begin("Sprite selector");
    ImGui::Text("selected xsplits : %u\n", editor->selected_sprite_x);
    ImGui::Text("selected ysplits : %u\n", editor->selected_sprite_y);
    ImGui::Text("xsplits          : %u\n", x_max);
    ImGui::Text("ysplits          : %u\n", y_max);
    ImGui::Text("ticks count      : %u\n", get_ticks_since_start());
    ImGui::Text("fps              : %f\n", ImGui::GetIO().Framerate);
    ImGui::End();


    if (is_key_pressed(SDLK_RIGHT)) x_idx += 1;
    if (is_key_pressed(SDLK_LEFT))  x_idx -= 1;
    if (is_key_pressed(SDLK_DOWN))  y_idx -= 1;
    if (is_key_pressed(SDLK_UP))    y_idx += 1;
   
    x_idx = (x_max + x_idx) % x_max;
    y_idx = (y_max + y_idx) % y_max;

    editor->selected_sprite_x = x_idx;
    editor->selected_sprite_y = y_idx;

    glm::vec2 selected_sheet_offset = glm::vec2(
            tile_render_size.x / (x_max) * (x_idx % x_max), 
            tile_render_size.y / (y_max) * (y_idx % y_max)
        );

    glm::vec2 selected_sheet_size = glm::vec2(tile_render_size.x / x_max, tile_render_size.y / y_max);

    // render
    glUseProgram(pointer->p4);

    glUniformMatrix4fv(projectionLoadtion, 1, GL_FALSE, glm::value_ptr(pointer->static_ortho_projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->plain_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);

    // this does not require texture to render stuff
    start_rendering(&pointer->static_ui_renderer);
    render_quad_rect_tex(
            &pointer->static_ui_renderer, 
            glm::vec2(0.0, 0.0), 
            glm::vec2(pointer->yresolution * 0.5 + 10), 
            glm::vec4(0.2, 0.4, 0.6, 0.5), 
            glm::vec2(0.0f), 
            glm::vec2(1.0f)
            ); // because teh default uv coordinated are 0 0 
    end_rendering(&pointer->static_ui_renderer);
    draw(&pointer->static_ui_renderer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->tile_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);

    start_rendering(&pointer->static_ui_renderer);
    render_quad_rect_tex(
            &pointer->static_ui_renderer, 
            glm::vec2(5, 5), 
            glm::vec2(pointer->yresolution * 0.5, pointer->yresolution * 0.5), 
            glm::vec4(1.0), 
            glm::vec2(0.0), glm::vec2(1.0f)
            );
    end_rendering(&pointer->static_ui_renderer);
    draw(&pointer->static_ui_renderer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->plain_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);

    start_rendering(&pointer->static_ui_renderer);
    
    render_quad_rect_tex(
            &pointer->static_ui_renderer,
            glm::vec2(5, 5) + selected_sheet_offset,
            selected_sheet_size,
            glm::vec4(0.8, 0.2, 0.9, 0.5),
            glm::vec2(0.0), glm::vec2(1.0)
            );
    end_rendering(&pointer->static_ui_renderer);
    draw(&pointer->static_ui_renderer);
}

extern "C"
void gamespace_update_function(MemoryBlock * gspace_mem){
    
    GameMemory * pointer = GET_ALIGNMENT_POINTER(gspace_mem->ptr, GameMemory);
    RESET_ARENA(&pointer->temporary);

    // process input 

    if (is_key_pressed(SDLK_ESCAPE)){
        set_quit_request();
    }

    // process stuff


    ImGui::Begin("General Information");
    ImGui::Text("ticks count      : %u\n", get_ticks_since_start());
    ImGui::Text("fps              : %f\n", ImGui::GetIO().Framerate);
    ImGui::End();


    if (is_key_pressed(SDLK_1)){
        if (pointer->current_ui & TILE_PLACEMENT){
            pointer->current_ui &= ~(TILE_PLACEMENT);
            pointer->current_ui &= ~(TILE_SELECTION);
        } else {
            pointer->current_ui |= TILE_PLACEMENT;
        }
    }

    if (is_key_pressed(SDLK_2) && pointer->current_ui & TILE_PLACEMENT){
        if (pointer->current_ui & TILE_SELECTION) {
            pointer->current_ui &= ~(TILE_SELECTION);
        } else {
            pointer->current_ui |= TILE_SELECTION;
        }
    }

    if (is_key_pressed(SDLK_SPACE)){
        pointer->rotating = !pointer->rotating;
    }
    

    { // camera update
       if (is_key_down(SDLK_d)) pointer->camera.position += glm::vec2(-20.0f, 0.0f);
       if (is_key_down(SDLK_a)) pointer->camera.position += glm::vec2(20.0f, 0.0f);
       if (is_key_down(SDLK_w)) pointer->camera.position += glm::vec2(0.0f, -20.0f);
       if (is_key_down(SDLK_s)) pointer->camera.position += glm::vec2(0.0f, 20.0f);

       generate_camera_matrix(&pointer->camera);
    }
    glm::vec2 worldmousepos = window_to_world_pos(pointer, mouse_window_pos());
    // rendering

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_collision_test(pointer);

    if (pointer->current_ui == 0){
        render_world(pointer);
    } 
    if (pointer->current_ui & TILE_PLACEMENT){
        render_tile_placement_gui(pointer);
    }
    if (pointer->current_ui & TILE_SELECTION){
        render_tile_selection_gui(pointer);
    }

    glDisable(GL_BLEND);
}
