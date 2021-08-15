#ifndef MICRALN_IMGUISDLGLCONTEXT_H
#define MICRALN_IMGUISDLGLCONTEXT_H

#include "imgui.h"

#include <SDL.h>

class ImGui_SDL_GL_Context
{
public:
    struct Config
    {
        int width;
        int height;
    };

    explicit ImGui_SDL_GL_Context(Config config);

    ~ImGui_SDL_GL_Context();

    void start_imgui_frame();
    bool process_events(void (*function)(const SDL_Event&));
    void render_frame();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

private:
    bool wants_to_quit(const SDL_Event& event);

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context{};
};

#endif //MICRALN_IMGUISDLGLCONTEXT_H
