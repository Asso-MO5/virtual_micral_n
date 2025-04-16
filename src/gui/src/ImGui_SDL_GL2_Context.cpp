#include "ImGui_SDL_GL2_Context.h"

#include "ImGui_SDL_GL_Context.h"
#include "imgui_impl_opengl2.h"

#include <imgui_impl_sdl2.h>

ImGui_SDL_GL2_Context::ImGui_SDL_GL2_Context(const ImGui_SDL_GL_Context::Config& config)
    : ImGui_SDL_GL_Context(config)
{
    SetGLAttributes();

    auto window_flags =
            static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                         SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
    create_window(window_flags);

    const bool with_filesystem = true;
    setup_imgui(with_filesystem);

    ImGui_ImplOpenGL2_Init();

    load_imgui_fonts();
}

ImGui_SDL_GL2_Context::~ImGui_SDL_GL2_Context()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}


void ImGui_SDL_GL2_Context::SetGLAttributes()
{
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
}

void ImGui_SDL_GL2_Context::start_frame()
{
    ImGui_ImplOpenGL2_NewFrame();
}

void ImGui_SDL_GL2_Context::render_draw_data() {
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}
