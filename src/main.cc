#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

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


#define STATE_QUIT_REQUESTED      1 << 0
#define STATE_KEYBOARD_KEY_CHANGE_EVENT  1 << 1
#define STATE_KEYBOARD_KEY_REPEATER_EVENT 1 << 2
#define STATE_MOUSE_BUTTON_EVENT  1 << 3
#define STATE_MOUSE_MOTION_EVENT  1 << 4

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

void initialize_central_state(){
}


int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    // NOTE: there is not error handling here, please add this in the future

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    const char * shader_preprocessor = "#version 400";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);


    auto windowHandle = SDL_CreateWindow("main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    assert(windowHandle != nullptr);

    auto contextHandle = SDL_GL_CreateContext(windowHandle);
    assert(contextHandle != nullptr);

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        std::cout << "failed to create context\n";
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


        if (keyboard_key_change_event()){
            std::cout << "keyboard key change event detected" << std::endl;
        }
        if (mouse_button_event()){
            std::cout << "mouse button event detected" << std::endl;
        }
        if (mouse_motion_event()){
            std::cout << "mouse motion event detected" << std::endl;
        }


        // all the update code will happen here in the main game loop

        // all the update code will end here

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // all the rendering code will happen here in the main game loop 

        ImGui::Begin("Main Internal window");
        ImGui::Checkbox("Boolean property", &bvalue);
        if (ImGui::Button("Reset property")){
            bvalue = false;
        }
        ImGui::End();

        // this is where all the rendering code will end

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
