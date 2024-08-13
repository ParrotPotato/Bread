#include "gamespace.hh"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>


#include "platform.hh"



void update(){
    return;
}


void internal_change(){
}

extern "C"
void gamespace_update_function(){
    ImGui::Begin("Test gamespace window");
    if (is_key_down(SDLK_a)){
        ImGui::Text("a button is not up");
    } else if (is_key_down(SDLK_b)){
        ImGui::Text("b button is not up");
    }else {
        ImGui::Text("all buttons are up");
    }


    ImGui::End();
}


extern "C"    
void gamespace_init_function(){
    printf("gamespace init\n");
}

extern "C"
void gamespace_before_unload(){
    printf("gamespace before unload\n");
}

extern "C"
void gamespace_after_load(){
    printf("gamespace after load\n");
}

