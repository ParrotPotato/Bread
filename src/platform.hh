#ifndef PLATFORM_HH
#define PLATFORM_HH


#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>


#define     VISIBLEKEYS     128
#define     FUNCTIONALKEYS  256
#define     KEYOFFSET       SDLK_CAPSLOCK
#define     KEYCOUNT        (VISIBLEKEYS + FUNCTIONALKEYS)


struct KeyboardState{
    bool cur[KEYCOUNT];
    bool pre[KEYCOUNT];
};


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
};

const SDL_Window * get_window_handle();
const SDL_GLContext get_context_handle();

void set_quit_requested();
void set_quit_request();
bool is_quit_requested();
bool keyboard_key_repeater_event();
bool keyboard_key_change_event();
bool mouse_button_event();
bool mouse_motion_event();
bool is_key_down(int keycode);
bool is_key_up(int keycode);
bool is_key_pressed(int keycode);
bool is_key_released(int keycode);
bool is_button_down(int buttoncode);
bool is_button_up(int buttoncode);
bool is_button_pressed(int buttoncode);
bool is_button_released(int buttoncode);
glm::vec2 mouse_window_pos();
glm::vec2 mouse_window_motion();

void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam);

void platform_init(const char * window, unsigned int width, unsigned int height, unsigned int flags);
void platform_begin_rendering();
void platform_end_rendering();
void platform_update_input_state();
void platform_delete_all_data();

#endif
