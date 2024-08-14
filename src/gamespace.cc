#include "gamespace.hh"

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include "memory.hh"


#define QUADCOUNT 100

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
"layout (location = 1) in vec3 color;\n\n"
"out vec3 vertcolor;\n"
"void main(){\n"
"   gl_Position = vec4(position, 0.0, 1.0);\n"
"   vertcolor= color;\n"
"}\n";

const char * f2 = ""
"#version 400 core\n"
"out vec4 fragcolor;\n"
"in  vec3 vertcolor;\n"
"void main(){\n"
"   fragcolor = vec4(vertcolor, 1.0);\n"
"}\n";

const char * v3 = ""
"#version 400 core\n"
"layout (location = 0) in vec2 position;\n\n"
"layout (location = 1) in vec3 color;\n\n"
"out vec3 vertcolor;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"   gl_Position = projection * vec4(position, 0.0, 1.0);\n"
"   vertcolor= color;\n"
"}\n";

const char * f3 = ""
"#version 400 core\n"
"out vec4 fragcolor;\n"
"in  vec3 vertcolor;\n"
"void main(){\n"
"   fragcolor = vec4(vertcolor, 1.0);\n"
"}\n";

const char * v4 = ""
"#version 400 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec3 color;\n"
"layout (location = 2) in vec2 uv;\n\n"
"out vec3 vertcolor;\n"
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
"in  vec3 vertcolor;\n"
"in  vec2 vertuv;\n"
"uniform sampler2D spriteTexture;\n"
"void main(){\n"
"   fragcolor = texture(spriteTexture, vertuv) * vec4(vertcolor, 1.0);\n"
"}\n";



char * load_entire_file_in_temp_buffer(GameMemory * memory, const char *filepath){
    // should be later replaced with open call (instead of fopen)
    FILE * fptr = fopen(filepath, "rb");
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    char * string = ALLOCATE_ARRAY(&memory->temporary, char, fsize + 1);
    string[fsize] = 0;
    fread(string, fsize, 1, fptr);
    return string;
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
    renderer->mem_color_buffer   = ALLOCATE_ARRAY(&memory->permanent, float,        quad_count * 12);
    renderer->mem_index_buffer   = ALLOCATE_ARRAY(&memory->permanent, unsigned int, quad_count * 6);

    renderer->total_vertices = quad_count* 8;
    renderer->total_uv_coords = quad_count * 8;
    renderer->total_colors = quad_count * 12;
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
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

void render_quad_rect_tex(
        Renderer2D * renderer, 
        glm::vec2 pos, 
        glm::vec2 dim, 
        glm::vec3 color = glm::vec3(1.0f),
        glm::vec2 uv_pos = {0.5, 0.5}, 
        glm::vec2 uv_dim = {1.0, 1.0}
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

    rect[0] = glm::vec2(pos.x, pos.y - dim.y);
    rect[1] = glm::vec2(pos.x, pos.y);
    rect[2] = glm::vec2(pos.x + dim.x, pos.y);
    rect[3] = glm::vec2(pos.x + dim.x, pos.y - dim.y);

    glm::vec3 colors[4] = { color,color,color,color };
    
    glm::vec2 uv[4];
    uv[0] = glm::vec2(uv_pos.x - uv_dim.x * 0.5, uv_pos.y - uv_dim.y * 0.5); 
    uv[1] = glm::vec2(uv_pos.x - uv_dim.x * 0.5, uv_pos.y + uv_dim.y * 0.5); 
    uv[2] = glm::vec2(uv_pos.x + uv_dim.x * 0.5, uv_pos.y + uv_dim.y * 0.5); 
    uv[3] = glm::vec2(uv_pos.x + uv_dim.x * 0.5, uv_pos.y - uv_dim.y * 0.5); 

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

    memcpy(renderer->mem_color_buffer + renderer->added_colors, colors, sizeof(float) * 3 * 4);
    renderer->added_colors += 3 * 4;

    memcpy(renderer->mem_uv_coord_buffer + renderer->added_uv_coords, uv, sizeof(float) * 2 * 4);
    renderer->added_uv_coords += 2 * 4;

    memcpy(renderer->mem_index_buffer + renderer->added_indices, indices, sizeof(unsigned int) * 6);
    renderer->added_indices += 6;
}

void render_quad_rect(Renderer2D * renderer, glm::vec2 pos, glm::vec2 dim, glm::vec3 color){

    if (
            renderer->added_vertices >= renderer->total_vertices 
            || renderer->added_colors >= renderer->total_colors 
            || renderer->added_indices >= renderer->total_indices
       ){
        printf("renderer; buffer entirely full\n");
        return;
    } 

    glm::vec2 rect[4];
    rect[0] = glm::vec2(pos.x - dim.x * 0.5, pos.y - dim.y * 0.5);
    rect[1] = glm::vec2(pos.x - dim.x * 0.5, pos.y + dim.y * 0.5);
    rect[2] = glm::vec2(pos.x + dim.x * 0.5, pos.y + dim.y * 0.5);
    rect[3] = glm::vec2(pos.x + dim.x * 0.5, pos.y - dim.y * 0.5);

    glm::vec3 colors[4] = { color,color,color,color };

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

    memcpy(renderer->mem_color_buffer + renderer->added_colors, colors, sizeof(float) * 3 * 4);
    renderer->added_colors += 3 * 4;

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
    game_mem->counter = 0;

    size_t remaining_size = gspace_mem->size - game_memory_offset - sizeof(GameMemory);

    game_mem->permanent.ptr =  (char *) game_mem + sizeof(GameMemory);
    game_mem->permanent.size =  remaining_size - remaining_size / 2;
    game_mem->permanent.cur = 0;

    game_mem->temporary.ptr = (char *) game_mem + sizeof(GameMemory) + game_mem->permanent.size;
    game_mem->temporary.size = remaining_size / 2;
    game_mem->temporary.cur = 0;

    // Resource loading

    game_mem->renderer = {0};
    init_renderer(game_mem, &game_mem->renderer);

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

    const char * texture_path = "/home/nitesh/work/projects/active/beta-one/data/kenney_scribble-platformer/Spritesheet/spritesheet_default.png";

    Texture2D sprite_sheet_texture = {};
    if(load_texture_memory( &sprite_sheet_texture, texture_path)) {
        printf("failed to load texture %s\n", texture_path);
    }
    
    game_mem->tile_texture = sprite_sheet_texture;

    const float xresolution = 800;
    const float yresolution = 600;

    game_mem->xresolution = 800;
    game_mem->yresolution = 600;

    game_mem->ortho_projection = glm::ortho(0.0f, xresolution, 0.0f, yresolution, 0.0f, 1000.0f);

    glClearColor(0.1f, 0.4f, 0.03f, 1.0f);
    glClearDepth(1.0f);
}


extern "C"
void gamespace_update_function(MemoryBlock * gspace_mem){
    GameMemory * pointer = GET_ALIGNMENT_POINTER(gspace_mem->ptr, GameMemory);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLint projectionLoadtion = glGetUniformLocation(pointer->p4,  "projection");
    glUseProgram(pointer->p4);
    glUniformMatrix4fv(projectionLoadtion, 1, GL_FALSE, glm::value_ptr(pointer->ortho_projection));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pointer->tile_texture.id);
    glUniform1i(glGetUniformLocation(pointer->p4, "spriteTexture"), 0);
    start_rendering(&pointer->renderer);
    render_quad_rect_tex(&pointer->renderer, glm::vec2(pointer->xresolution / 2.0, pointer->yresolution), glm::vec2(pointer->yresolution * 0.5), glm::vec3(1.0), glm::vec2(0.5), glm::vec2(1.0));
    end_rendering(&pointer->renderer);
    draw(&pointer->renderer);
    glDisable(GL_BLEND);

    ImGui::Begin("Game window");
    if (ImGui::Button("increase")){
        pointer->counter++;
    } 
    if (ImGui::Button("decrease")){
        pointer->counter--;
    }
    ImGui::Text("counter : %d", pointer->counter);
    ImGui::Text("after the first update on the build command");
    ImGui::Text("after the hot reloading bug fix in build command");
    ImGui::Text("after the hot reloading bug fix in build command");
    ImGui::End();
}
