#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "platform.hh"
#include "memory.hh"
#include "gamespace.hh"

/* Platform specific code */
#include <dlfcn.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/select.h>



// Tasks 
// 1. Setting up SDL2 and OPENGL build with cmake 
// 2. Setting up DearIMGUI build with cmake


struct GamespaceLibrary{
    void * handle = 0;

    gamespace_init_function_t   gspace_init_func            = 0 ;
    gamespace_update_function_t gspace_update_func          = 0 ;
};


int load_library(GamespaceLibrary * lib) {
    lib->handle = dlopen("./libgamespace.so", RTLD_NOW);
    if(!lib->handle) { 
        printf("unable to load library ./libgamespace.so : %s\n", dlerror());
        return -1; 
    }

    lib->gspace_init_func = (gamespace_init_function_t) dlsym(lib->handle, "gamespace_init_function");
    if(!lib->gspace_init_func){
        printf("unable to load function gamespace_init_function: %s\n", dlerror());
        return -1;
    }

    lib->gspace_update_func= (gamespace_update_function_t) dlsym(lib->handle, "gamespace_update_function");
    if(!lib->gspace_update_func){ 
        printf("unable to load function gamespace_update_function: %s\n", dlerror()); 
        return -1; 
    }
    return 0;
}

int unload_library(GamespaceLibrary * lib) {
    if(dlclose(lib->handle)){
        printf("error occurred while unloading gamespace library: %s\n", dlerror());
        return -1;
    }
    *lib = {0};
    return 0;
}

int reload_library(GamespaceLibrary * lib){
    if (unload_library(lib)){
        printf("error whiel reload_library\n");
        return -1;
    }
    if (load_library(lib)){
        printf("error while reload_library\n");
        return -1;
    }
    return 0;
}


int main(int argc, char ** argv) {


    printf("Memory stack allocator test starts\n");

    MemoryStackAllocator sa =  {};
    void * buffer = 0;
    init_stack_allocator(&sa, buffer, 10);
    void * first = push_in_stack(&sa, 2, 1);
    assert(first == (void *) 0x00);
    void * second = push_in_stack(&sa, 4, 4);
    assert(second == (void *) 0x04);
    void * third = push_in_stack(&sa, 2, 1);
    assert(third == (void *) 0x08);
    pop_from_stack(&sa);
    void * fourth = push_in_stack(&sa, 3, 1);
    assert(fourth == nullptr);

    printf("Memory stack allocator test ends\n");


    printf("Memory arena allocator test starts\n");
    // @todo(nitesh) : adds the arena based allocator tests in herre as well 
    printf("Memory arena allocator test ends\n");

            
    // end mem test
    platform_init("main window", 1200, 900, SDL_WINDOW_OPENGL);

    GamespaceLibrary lib = {0};
    if (load_library(&lib)) {
        printf("error occurred while loading library for first time\n");
        return -1;
    }

    MemoryBlock gspace_mem = {0};
    gspace_mem.ptr = malloc(MB(50));
    gspace_mem.size = MB(50);

    lib.gspace_init_func(&gspace_mem);

    bool bvalue = false;
    glClearDepth(1.0f);

    unsigned int b = get_ticks_since_start();
    unsigned int delta = 0;
    while(is_quit_requested() == false){
        platform_update_input_state();
        platform_begin_rendering();

        lib.gspace_update_func(&gspace_mem);

        platform_end_rendering();
        if (is_key_pressed(SDLK_r)){
            unsigned int ticks   = get_ticks_since_start();
            unsigned int seconds  = ticks/1000;
            unsigned int minutes = seconds/60;
            unsigned int hours   = minutes/60;
            printf("[%02u:%02u:%02u] reloading library instance\n", hours, minutes, seconds);
            reload_library(&lib);
        }
    }
    platform_delete_all_data();
    return 0;
}
