#include <iostream>

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "platform.hh"
#include "memory.hh"
#include "gamespace.hh"

// Todo add keyboard / mouse / joystick support
// potentially multiple joystick support for couch co-op


// Tasks 
// 1. Setting up SDL2 and OPENGL build with cmake 
// 2. Setting up DearIMGUI build with cmake


char * load_entire_file_in_buffer(const char *filepath){
    // should be later replaced with open call (instead of fopen)
    FILE * fptr = fopen(filepath, "rb");
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    char * string = (char *) malloc(fsize + 1);
    string[fsize] = 0;
    fread(string, fsize, 1, fptr);
    return string;
}

int compile_shader(char * shader_buffer, GLenum type){
    int shader_handle = glCreateShader(type);
    glShaderSource(shader_handle, 1, &shader_buffer, 0);
    glCompileShader(shader_handle);
    GLint status = 0;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE){
        int error_log_length = 0;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &error_log_length);
        char * error_log = (char *) malloc(error_log_length + 1);
        glGetShaderInfoLog(shader_handle, error_log_length, &error_log_length, error_log);
        printf("shader compilation error\n");
        printf("%s\n", error_log);
        free(error_log);
        glDeleteShader(shader_handle);
        shader_handle = 0;
    }
    return shader_handle;
}

int link_program(int * shader_list, int shader_count){
    GLint program = glCreateProgram();
    for(int i = 0; i < shader_count ; i++) glAttachShader(program, shader_list[i]);
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) 
    {
        int error_log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &error_log_length);
        char * error_log = (char *) malloc(error_log_length + 1);
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

struct Texture2D {
    GLuint          id;
    unsigned int    width;
    unsigned int    height;
    unsigned int    components;
};

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

#define  CLEAN         0
#define  UNFLUSHED     1 << 0
#define  QUADCOUNT     8000
struct Renderer2D {
    float * mem_vertex_buffer = 0;
    float * mem_color_buffer = 0;
    unsigned int * mem_index_buffer = 0;

    size_t added_vertices = 0;
    size_t added_indices = 0;
    size_t added_colors = 0;

    size_t total_colors = 0;
    size_t total_vertices = 0;
    size_t total_indices = 0;

    GLuint vbo = 0;
    GLuint cbo = 0;

    GLuint ibo = 0;
    GLuint vao = 0;

    int state = 0;

};

void init_renderer(Renderer2D * renderer, int quad_count = QUADCOUNT){
    renderer->mem_vertex_buffer = (float *) malloc(sizeof(float) * quad_count * 8);
    renderer->mem_color_buffer = (float *) malloc(sizeof(float) * quad_count * 12);
    renderer->mem_index_buffer  = (unsigned int *) malloc(sizeof(unsigned int) * quad_count * 6);

    renderer->total_vertices = quad_count* 8;
    renderer->total_colors = quad_count * 12;
    renderer->total_indices  = quad_count * 6;

    renderer->state = CLEAN;

    glGenBuffers(1, &renderer->vbo);
    glGenBuffers(1, &renderer->ibo);
    glGenBuffers(1, &renderer->cbo);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * renderer->total_vertices, renderer->mem_vertex_buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->cbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * renderer->total_colors , renderer->mem_color_buffer, GL_STATIC_DRAW);
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
    glBindVertexArray(0);
}

void start_rendering(Renderer2D * renderer){
    renderer->added_indices = 0;
    renderer->added_vertices = 0;
    renderer->added_colors = 0;

    renderer->state = CLEAN;
}

void render_quad_rect(Renderer2D * renderer, glm::vec2 pos, glm::vec2 dim, glm::vec3 color){

    if (
            renderer->added_vertices >= renderer->total_vertices 
            || renderer->added_colors >= renderer->total_colors 
            || renderer->added_indices >= renderer->total_indices
       ){
        std::cout << "renderer :: buffer entire full\n";
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * renderer->added_indices, (void *) renderer->mem_index_buffer);
}


void draw(Renderer2D * renderer){
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glDrawElements(GL_TRIANGLES, renderer->added_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

struct Box{
    glm::vec2 pos, dim;
    glm::vec3 color;
};


#define NONE                      0
#define SPRITE_FORMATTER_SCREEN   1

struct GameStateData{

    struct SpriteFormatterData{
    };

    int state = 0;
};

int main() {

    // Code for testing game dynamic loading

    enter_game_space(0);
    before_reset();
    after_reset(0);

    // Generating required memory buffers

    // following is the test code for the memory allocator 
    SuperArena mainstaticarena = {0};
    mainstaticarena.ptr = malloc(1024);
    mainstaticarena.size = 1024;

    Arena perframearena = {0};
    perframearena.ptr = mainstaticarena.ptr;
    perframearena.cur = 0;
    perframearena.size = mainstaticarena.size / 2;

    int * values = ALLOCATE_ARRAY(&perframearena, 10, int);
    values[0] = 0;
    values[1] = 1;
    values[2] = 2;
    values[3] = 3;

    unsigned long long * unsigned_values = ALLOCATE_ARRAY(&perframearena, 10,unsigned long long);
    unsigned_values[0] = 0;
    unsigned_values[1] = 1;
    unsigned_values[2] = 2;
    unsigned_values[3] = 3;

    // generation ends

    platform_init("main window", 800, 600, SDL_WINDOW_OPENGL);

    // starting imgui

    bool bvalue = false;
    glClearColor(0.1, 0.2, 0.3, 1.0);
    glClearDepth(1.0f);


    GLuint vbo;
    GLuint ibo;

    float vertices [] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2
    };

    // Following are the baby shaders which will be later converted to big boy shaders
    // keeping the book of shadeds next to them so that they can learn and grow stronger
    // https://thebookofshaders.com/

    const char * vertexshader = ""
        "#version 400 core\n"
        "layout (location = 0) in vec3 position;\n\n"
        "void main(){\n"
        "   gl_Position = vec4(position, 1.0);\n"
        "}\n";
    const char * fragmentshader = ""
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

    const float xresolution = 800;
    const float yresolution = 600;
    glm::mat4 ortho_projection = glm::ortho(0.0f, xresolution, 0.0f, yresolution, 0.0f, 1000.0f);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    GLint vertex_shader = compile_shader((char *)vertexshader, GL_VERTEX_SHADER);
    GLint fragment_shader = compile_shader((char *)fragmentshader, GL_FRAGMENT_SHADER);
    GLint shaders [] = { vertex_shader, fragment_shader };
    GLint program = link_program(shaders, 2);

    GLint v2s = compile_shader((char *)v2, GL_VERTEX_SHADER);
    GLint f2s = compile_shader((char *)f2, GL_FRAGMENT_SHADER);
    GLint s2[] = { v2s, f2s };
    GLint p2 = link_program(s2, 2);

    GLint v3s = compile_shader((char *)v3, GL_VERTEX_SHADER);
    GLint f3s = compile_shader((char *)f3, GL_FRAGMENT_SHADER);
    GLint s3[] = {v3s, f3s};
    GLint p3 = link_program(s3, 2);

    GLint v4s = compile_shader((char *)v4, GL_VERTEX_SHADER);
    GLint f4s = compile_shader((char *)f4, GL_FRAGMENT_SHADER);
    GLint s4[] = {v4s, f4s};
    GLint p4 = link_program(s4, 2);

    const int spark_count = QUADCOUNT;
    Box sparks[spark_count];
    for(int i = 0 ; i < spark_count;i++){
        sparks[i].pos.x = (-100 + rand() % 200) / 100.0f;
        sparks[i].pos.y = (-100 + rand() % 200) / 100.0f;

        sparks[i].dim = glm::vec2(0.007, 0.007);

        sparks[i].color.r = (rand()  % 100 ) / 100.0f;
        sparks[i].color.b = (rand()  % 100 ) / 100.0f;
        sparks[i].color.g = (rand()  % 100 ) / 100.0f;
    }

    Renderer2D spark_renderer = {};
    init_renderer(&spark_renderer);

    Renderer2D sprite_renderer = {};
    init_renderer(&sprite_renderer);

    Box sprite = {};
    sprite.pos = glm::vec2(xresolution / 2.0, yresolution / 2.0);
    sprite.dim = glm::vec2(100, 100);
    sprite.color = glm::vec3(0.3, 0.8, 0.1);

    while(is_quit_requested() == false){
        platform_update_input_state();

        for(int i = 0 ; i < spark_count; i++){
            sparks[i].pos = sparks[i].pos - glm::vec2(0.00, 0.01);
            if (sparks[i].pos.y <= -1.0f) {
                sparks[i].pos.x = (-100 + rand() % 200) / 100.0f;
                sparks[i].pos.y = 1.0f;
            }
            sparks[i].color.r = (rand()  % 100 ) / 100.0f;
            sparks[i].color.b = (rand()  % 100 ) / 100.0f;
            sparks[i].color.g = (rand()  % 100 ) / 100.0f;
        }
        platform_begin_rendering();

        glUseProgram(program);
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glUseProgram(p2);
        start_rendering(&spark_renderer);
        for(int i = 0 ; i < spark_count; i++){
            render_quad_rect(&spark_renderer, sparks[i].pos, sparks[i].dim, sparks[i].color);
        }
        end_rendering(&spark_renderer);
        draw(&spark_renderer);

        GLint projectionLocation = glGetUniformLocation(p3, "projection");
        glUseProgram(p3);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(ortho_projection));
        start_rendering(&sprite_renderer);
        render_quad_rect(&sprite_renderer, sprite.pos, sprite.dim, sprite.color);
        end_rendering(&sprite_renderer);
        draw(&sprite_renderer);


        ImGui::NewFrame();
        ImGui::Begin("Main internal window");
        ImGui::Checkbox("Boolean property", &bvalue);
        if (ImGui::Button("Reset property")) {
            bvalue = false;
        }
        ImGui::End();

        platform_end_rendering();
    }
    platform_delete_all_data();
    return 0;
}
