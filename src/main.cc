#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <assert.h>

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    auto windowHandle = SDL_CreateWindow("main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL);
    assert(windowHandle != nullptr);
    auto contextHandle = SDL_GL_CreateContext(windowHandle);
    assert(contextHandle != nullptr);
    if(glewInit() != GLEW_OK){
        std::cout << "failed to create context\n";
        return 0;
    }
    glClearColor(0.1, 0.2, 0.3, 1.0);
    glClearDepth(1.0f);
    while(1){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:
                    std::cout << "termination signal received by window\n";
                    return 0;
                default:
                    break;
            }
        }

        SDL_GL_SwapWindow(windowHandle);
    }

    std::cout << "Hello world!\n";
    return 0;
}
