#ifndef MICRALN_IMGUI_SDL_GL2_CONTEXT_H
#define MICRALN_IMGUI_SDL_GL2_CONTEXT_H

#include "ImGui_SDL_GL_Context.h"

class ImGui_SDL_GL2_Context : public ImGui_SDL_GL_Context
{
public:
    explicit ImGui_SDL_GL2_Context(const Config& config);
    ~ImGui_SDL_GL2_Context() override;

private:
    static void SetGLAttributes() ;
    void start_frame() override;
    void render_draw_data() override;
};

#endif //MICRALN_IMGUI_SDL_GL2_CONTEXT_H
