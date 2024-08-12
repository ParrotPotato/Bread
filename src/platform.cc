#include "platform.hh"

#include <SDL2/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <string>

SystemStateHandler g_state = {0};

const SDL_Window * get_window_handle(){
    return g_state.window_handle;
}
const SDL_GLContext get_context_handle() {
    return g_state.context_handle;
}

int get_key_offset(int keycode){
    return ((keycode >= KEYOFFSET)? (keycode - KEYOFFSET + VISIBLEKEYS) : (keycode));
}

void set_quit_request(){
    g_state.central_state |= STATE_QUIT_REQUESTED;
}

bool is_quit_requested(){
    return (g_state.central_state & STATE_QUIT_REQUESTED);
}

bool keyboard_key_repeater_event() {
    return g_state.central_state & STATE_KEYBOARD_KEY_REPEATER_EVENT;
}

bool keyboard_key_change_event(){
    return g_state.central_state & STATE_KEYBOARD_KEY_CHANGE_EVENT;
}
bool mouse_button_event(){
    return g_state.central_state & STATE_MOUSE_BUTTON_EVENT;
}
bool mouse_motion_event(){
    return g_state.central_state & STATE_MOUSE_MOTION_EVENT;
}

bool is_key_down(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == true;
}

bool is_key_up(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == false;
}

bool is_key_pressed(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == true && g_state.key.pre[keycode] == false; 
}

bool is_key_released(int keycode){
    keycode = get_key_offset(keycode);
    return g_state.key.cur[keycode] == false && g_state.key.pre[keycode] == true; 
}


bool is_button_down(int buttoncode){
    return g_state.mouse.cur[buttoncode] == true;
}
bool is_button_up(int buttoncode){
    return g_state.mouse.cur[buttoncode] == false;
}
bool is_button_pressed(int buttoncode){
    return g_state.mouse.cur[buttoncode] == true && g_state.mouse.pre[buttoncode] == false; 
}
bool is_button_released(int buttoncode){
    return g_state.mouse.cur[buttoncode] == false && g_state.mouse.pre[buttoncode] == true; 
}

glm::vec2 mouse_window_pos(){
    return glm::vec2(g_state.mouse.xpos, g_state.mouse.ypos);
}

glm::vec2 mouse_window_motion(){
    return glm::vec2(g_state.mouse.xrel, g_state.mouse.yrel);
}

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

void platform_init(const char * window, unsigned int width, unsigned int height, unsigned int flags){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    const char * shader_preprocessor = "#version 400";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_Window * windowHandle = SDL_CreateWindow(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    assert(windowHandle != nullptr);
    SDL_GLContext contextHandle = SDL_GL_CreateContext(windowHandle);
    assert(contextHandle != nullptr);
    glewExperimental = GL_TRUE;
    assert(glewInit() == GLEW_OK);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(windowHandle, contextHandle);
    ImGui_ImplOpenGL3_Init(shader_preprocessor);
}



void platform_update_input_state(){
    
    
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
}

void platform_begin_rendering(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void platform_end_rendering(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_state.window_handle);
}

void platform_delete_all_data(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(g_state.context_handle);
    SDL_DestroyWindow(g_state.window_handle);
}
