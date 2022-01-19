#include <tuple>
#include "AppHost.h"
#include "Game.h"
#include "SDL.h"
#include "SDLAppHost.h"

namespace cpp_conv::apphost
{
    cpp_conv::apphost::SdlContext App;
}

std::tuple<int, int> cpp_conv::apphost::getAppDimensions()
{
    return std::make_tuple(1, 1);
}

void createWindow()
{
    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_VULKAN;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    cpp_conv::apphost::App.window = SDL_CreateWindow("Cpp Conveyor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 1000, windowFlags);

    if (!cpp_conv::apphost::App.window)
    {
        printf("Failed to open %d x %d window: %s\n", 800, 600, SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

    cpp_conv::apphost::App.renderer = SDL_CreateRenderer(cpp_conv::apphost::App.window, -1, rendererFlags);

    if (!cpp_conv::apphost::App.renderer)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }
} 

#ifdef __cplusplus
extern "C"
{
#endif
    int SDL_main(int argc, char* argv[])
    {
        createWindow();
        cpp_conv::game::run();

        SDL_Quit();
        return 0;
    }
#ifdef __cplusplus
}
#endif
