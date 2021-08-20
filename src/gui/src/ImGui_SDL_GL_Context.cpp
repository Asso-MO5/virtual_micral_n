#include "ImGui_SDL_GL_Context.h"
#include "imgui_impl_opengl2.h"

#include "imgui_impl_sdl.h"

#ifdef EMSCRIPTEN
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

ImGui_SDL_GL_Context::ImGui_SDL_GL_Context(ImGui_SDL_GL_Context::Config config) { init_sdl(); }

ImGui_SDL_GL_Context::~ImGui_SDL_GL_Context()
{
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool ImGui_SDL_GL_Context::process_events(void (*function)(const SDL_Event&))
{
    const ImGuiIO& io = ImGui::GetIO();
    bool done = false;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        done |= wants_to_quit(event);

        if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
        {
            function(event);
        }
    }

    return done;
}

bool ImGui_SDL_GL_Context::wants_to_quit(const SDL_Event& event)
{
    bool sdl_quit = event.type == SDL_QUIT;
    bool sdl_window_close = event.type == SDL_WINDOWEVENT &&
                            event.window.event == SDL_WINDOWEVENT_CLOSE &&
                            event.window.windowID == SDL_GetWindowID(window);

    return sdl_quit || sdl_window_close;
}

void ImGui_SDL_GL_Context::init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        //printf("Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void ImGui_SDL_GL_Context::create_window(const SDL_WindowFlags& window_flags)
{
    window = SDL_CreateWindow("Dear ImGui Emscripten example", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        exit(2);
    }
    SDL_GL_SetSwapInterval(1); // Enable vsync
}

void ImGui_SDL_GL_Context::setup_imgui(bool with_filesystem) const
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();                             // Setup Dear ImGui style

    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    if (!with_filesystem)
    {
        io.IniFilename = nullptr;
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
}

void ImGui_SDL_GL_Context::load_imgui_fonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
}

void ImGui_SDL_GL_Context::start_imgui_frame()
{
    start_frame();

    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

void ImGui_SDL_GL_Context::render_frame()
{
    ImGui::Render();

    SDL_GL_MakeCurrent(window, gl_context);

    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    render_draw_data();

    SDL_GL_SwapWindow(window);
}
