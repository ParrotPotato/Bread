#include <iostream>
#include <string>

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assert.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

// Todo add keyboard / mouse / joystick support
// potentially multiple joystick support for couch co-op

#define     VISIBLEKEYS     128
#define     FUNCTIONALKEYS  256
#define     KEYOFFSET       SDLK_CAPSLOCK
#define     KEYCOUNT        (VISIBLEKEYS + FUNCTIONALKEYS)


struct KeyboardState{
    bool cur[KEYCOUNT];
    bool pre[KEYCOUNT];
};
inline int get_key_offset(int keycode){
    return ((keycode >= KEYOFFSET)? (keycode - KEYOFFSET + VISIBLEKEYS) : (keycode));
}


#define     BUTTONCOUNT     6
struct MouseState{
    char cur[BUTTONCOUNT];
    char pre[BUTTONCOUNT];

    float  xpos, ypos;
    float  xrel, yrel;
};


#define STATE_QUIT_REQUESTED                1 << 0
#define STATE_KEYBOARD_KEY_CHANGE_EVENT     1 << 1
#define STATE_KEYBOARD_KEY_REPEATER_EVENT   1 << 2
#define STATE_MOUSE_BUTTON_EVENT            1 << 3
#define STATE_MOUSE_MOTION_EVENT            1 << 4

struct SystemStateHandler {
    SDL_Window * window_handle;
    SDL_GLContext  context_handle;

    KeyboardState key;
    MouseState    mouse;

    int central_state;
} g_state = {};


inline void set_quit_request(){
    g_state.central_state |= STATE_QUIT_REQUESTED;
}

inline bool is_quit_requested(){
    return (g_state.central_state & STATE_QUIT_REQUESTED);
}

inline bool keyboard_key_repeater_event() {
    return g_state.central_state & STATE_KEYBOARD_KEY_REPEATER_EVENT;
}

inline bool keyboard_key_change_event(){
    return g_state.central_state & STATE_KEYBOARD_KEY_CHANGE_EVENT;
}
inline bool mouse_button_event(){
    return g_state.central_state & STATE_MOUSE_BUTTON_EVENT;
}
inline bool mouse_motion_event(){
    return g_state.central_state & STATE_MOUSE_MOTION_EVENT;
}

inline bool is_key_down(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == true;
}

inline bool is_key_up(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == false;
}

inline bool is_key_pressed(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == true && g_state.key.pre[keycode] == false; 
}

inline bool is_key_released(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == false && g_state.key.pre[keycode] == true; 
}


inline bool is_button_down(int buttoncode){
    return g_state.mouse.cur[buttoncode] == true;
}
inline bool is_button_up(int buttoncode){
    return g_state.mouse.cur[buttoncode] == false;
}
inline bool is_button_pressed(int buttoncode){
    return g_state.mouse.cur[buttoncode] == true && g_state.mouse.pre[buttoncode] == false; 
}
inline bool is_button_released(int buttoncode){
    return g_state.mouse.cur[buttoncode] == false && g_state.mouse.pre[buttoncode] == true; 
}

inline glm::vec2 mouse_window_pos(){
    return glm::vec2(g_state.mouse.xpos, g_state.mouse.ypos);
}

inline glm::vec2 mouse_window_motion(){
    return glm::vec2(g_state.mouse.xrel, g_state.mouse.yrel);
}


// Tasks 
// 1. Setting up SDL2 and OPENGL build with cmake 
// 2. Setting up DearIMGUI build with cmake

void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam){
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::string sourceStr;
    switch (source) {
        case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    }

    std::string typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }

    std::string severityStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }

    fprintf(stderr, "GL Debug: Source: %s, Type: %s, ID: %d, Severity: %s, Message: %s\n",
        sourceStr.c_str(), typeStr.c_str(), id, severityStr.c_str(), message);
}

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

// I want to be able to render 2000 rects 
// this means 2000 * 8 floats for vertices
// this means 2000 * 6 unsigned for indices
// this means 2000 * 12 floats for colors

void init_renderer(Renderer2D * renderer){
    renderer->mem_vertex_buffer = (float *) malloc(sizeof(float) * QUADCOUNT * 8);
    renderer->mem_color_buffer = (float *) malloc(sizeof(float) * QUADCOUNT * 12);
    renderer->mem_index_buffer  = (unsigned int *) malloc(sizeof(unsigned int) * QUADCOUNT * 6);

    renderer->total_vertices = QUADCOUNT* 8;
    renderer->total_colors = QUADCOUNT * 12;
    renderer->total_indices  = QUADCOUNT * 6;

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


int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    const char * shader_preprocessor = "#version 400";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window * windowHandle = SDL_CreateWindow("main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    assert(windowHandle != nullptr);
    SDL_GLContext contextHandle = SDL_GL_CreateContext(windowHandle);
    assert(contextHandle != nullptr);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        std::cout << "glew init : failed to create context\n";
        return 0;
    }
    int opengl_context_flag = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &opengl_context_flag);
    if (opengl_context_flag & GL_CONTEXT_FLAG_DEBUG_BIT){
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(opengl_debug_message_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    g_state.window_handle = windowHandle;
    g_state.context_handle = contextHandle;


    // starting imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(windowHandle, contextHandle);
    ImGui_ImplOpenGL3_Init(shader_preprocessor);

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

    GLint shaders [] = {
        vertex_shader,
        fragment_shader
    };
    GLint program = link_program(shaders, 2);

    GLint v2s = compile_shader((char *)v2, GL_VERTEX_SHADER);
    GLint f2s = compile_shader((char *)f2, GL_FRAGMENT_SHADER);

    GLint s2[] = { v2s, f2s };
    GLint p2 = link_program(s2, 2);


    const int box_count = QUADCOUNT;
    Box boxes[box_count];
    
    for(int i = 0 ; i < box_count;i++){
        boxes[i].pos.x = (-100 + rand() % 200) / 100.0f;
        boxes[i].pos.y = (-100 + rand() % 200) / 100.0f;

        boxes[i].dim = glm::vec2(0.007, 0.007);

        boxes[i].color.r = (rand()  % 100 ) / 100.0f;
        boxes[i].color.b = (rand()  % 100 ) / 100.0f;
        boxes[i].color.g = (rand()  % 100 ) / 100.0f;
    }

    Renderer2D renderer = {};
    init_renderer(&renderer);

    while(is_quit_requested() == false){

        // resettign the central event state variable
        g_state.central_state &= ~ (STATE_QUIT_REQUESTED);
        g_state.central_state &= ~ (STATE_KEYBOARD_KEY_CHANGE_EVENT);
        g_state.central_state &= ~ (STATE_KEYBOARD_KEY_REPEATER_EVENT);
        g_state.central_state &= ~ (STATE_MOUSE_BUTTON_EVENT);
        g_state.central_state &= ~ (STATE_MOUSE_MOTION_EVENT);

        // reset keyboard and mouse
        for(int i = 0 ; i < KEYCOUNT ; i++){
            g_state.key.pre[i] = g_state.key.cur[i];
        }

        for(int i = 0 ; i < BUTTONCOUNT ; i++){
            g_state.mouse.pre[i] = g_state.mouse.cur[i];
        }
        g_state.mouse.xrel = g_state.mouse.yrel;

        // Setting local key buffer which can be used to call 
        // upon the callback

        int keyboard_events[256];
        int mouse_events[5];

        SDL_Event sdl_event;
        while(SDL_PollEvent(&sdl_event)){
            ImGui_ImplSDL2_ProcessEvent(&sdl_event);

            // custom state handlers for special elements

            // event state updater
            switch(sdl_event.type){
                case SDL_QUIT:
                    g_state.central_state |= STATE_QUIT_REQUESTED;
                    break;
                case SDL_KEYDOWN:
                    if (g_state.key.cur[get_key_offset(sdl_event.key.keysym.sym)] == false) {
                        g_state.central_state |= STATE_KEYBOARD_KEY_CHANGE_EVENT;
                    }
                    g_state.key.cur[get_key_offset(sdl_event.key.keysym.sym)] =  true;
                    keyboard_events[get_key_offset(sdl_event.key.keysym.sym)] = 0x0 | 1 << (sizeof(bool) * 8) | true;
                    g_state.central_state |= STATE_KEYBOARD_KEY_REPEATER_EVENT;
                    break;
                case SDL_KEYUP:
                    if (g_state.key.cur[get_key_offset(sdl_event.key.keysym.sym)] == true) {
                        g_state.central_state |= STATE_KEYBOARD_KEY_CHANGE_EVENT;
                    }
                    g_state.key.cur[get_key_offset(sdl_event.key.keysym.sym)] =  false;
                    keyboard_events[get_key_offset(sdl_event.key.keysym.sym)] = 0x0 | 1 << (sizeof(bool) * 8)  | false;
                    g_state.central_state |= STATE_KEYBOARD_KEY_REPEATER_EVENT;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    g_state.mouse.cur[sdl_event.button.button] =  true;
                    mouse_events[sdl_event.button.button]  = 0x0 | 1 << (sizeof(bool) * 8) | true;
                    g_state.central_state |= STATE_MOUSE_BUTTON_EVENT;
                    break;
                case SDL_MOUSEBUTTONUP:
                    g_state.mouse.cur[sdl_event.button.button] =  false;
                    mouse_events[sdl_event.button.button]  = 0x0 | 1 << (sizeof(bool) * 8) | false;
                    g_state.central_state |= STATE_MOUSE_BUTTON_EVENT;
                    break;
                case SDL_MOUSEMOTION:
                    g_state.mouse.xrel = sdl_event.motion.xrel;
                    g_state.mouse.yrel = sdl_event.motion.yrel;
                    g_state.mouse.xpos = sdl_event.motion.x;
                    g_state.mouse.ypos = sdl_event.motion.y;
                    g_state.central_state |= STATE_MOUSE_MOTION_EVENT;
                    break;
                    // handle more events here
                default:
                    break;
            }
        }

        for(int i = 0 ; i < box_count; i++){
            boxes[i].pos = boxes[i].pos - glm::vec2(0.00, 0.01);
            if (boxes[i].pos.y <= -1.0f) {
                boxes[i].pos.x = (-100 + rand() % 200) / 100.0f;
                boxes[i].pos.y = 1.0f;
            }
            boxes[i].color.r = (rand()  % 100 ) / 100.0f;
            boxes[i].color.b = (rand()  % 100 ) / 100.0f;
            boxes[i].color.g = (rand()  % 100 ) / 100.0f;
        }


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        glUseProgram(program);
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glUseProgram(p2);
        start_rendering(&renderer);
        for(int i = 0 ; i < box_count; i++){
            render_quad_rect(&renderer, boxes[i].pos, boxes[i].dim, boxes[i].color);
        }
        end_rendering(&renderer);
        draw(&renderer);

        ImGui::Begin("Main internal window");
        ImGui::Checkbox("Boolean property", &bvalue);
        if (ImGui::Button("Reset property")) {
            bvalue = false;
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(windowHandle);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(contextHandle);
    SDL_DestroyWindow(windowHandle);

    return 0;
}
