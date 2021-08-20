#ifndef MICRALN_IMGUI_SDL_GL_CONTEXT_H
#define MICRALN_IMGUI_SDL_GL_CONTEXT_H

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
    virtual ~ImGui_SDL_GL_Context();

    void start_imgui_frame();
    bool process_events(void (*function)(const SDL_Event&));
    void render_frame();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

protected:
    void create_window(const SDL_WindowFlags& window_flags);
    static void load_imgui_fonts() ;
    void setup_imgui(bool with_filesystem) const;

private:
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context{};

    bool wants_to_quit(const SDL_Event& event);
    static void init_sdl() ;
    virtual void start_frame() = 0;
    virtual void render_draw_data() = 0;
};

#endif //MICRALN_IMGUI_SDL_GL_CONTEXT_H
