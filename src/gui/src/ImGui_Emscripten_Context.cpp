#include "ImGui_Empscripten_Context.h"

#include "ImGui_SDL_GL_Context.h"
#include "imgui_impl_opengl3.h"

ImGui_SDL_GL3_Context::ImGui_SDL_GL3_Context(const ImGui_SDL_GL_Context::Config& config)
    : ImGui_SDL_GL_Context(config)
{
    SetGLAttributes();

    auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                                     SDL_WINDOW_ALLOW_HIGHDPI);
    create_window(window_flags);

    const bool with_filesystem = false;
    setup_imgui(with_filesystem);

    const char* glsl_version = "#version 100";
    //const char* glsl_version = "#version 300 es";
    ImGui_ImplOpenGL3_Init(glsl_version);

    load_imgui_fonts();
}

ImGui_SDL_GL3_Context::~ImGui_SDL_GL3_Context() {}

void ImGui_SDL_GL3_Context::SetGLAttributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

void ImGui_SDL_GL3_Context::start_frame() { ImGui_ImplOpenGL3_NewFrame(); }

void ImGui_SDL_GL3_Context::render_draw_data()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
