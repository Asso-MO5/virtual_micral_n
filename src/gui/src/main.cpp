#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

class ImGui_SDL_GL_Context
{
public:
    ImGui_SDL_GL_Context()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        {
            printf("Error: %s\n", SDL_GetError());
            exit(1);
        }

        // Setup window
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                                         SDL_WINDOW_ALLOW_HIGHDPI);
        window =
                SDL_CreateWindow("Micral N Emulation", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
        gl_context = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, gl_context);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL2_Init();

        // Load Fonts
        io.Fonts->AddFontDefault();
    }

    ~ImGui_SDL_GL_Context()
    {
        // Cleanup
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void start_imgui_frame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
    }

    bool process_events(void (*function)(const SDL_Event&))
    {
        ImGuiIO& io = ImGui::GetIO();
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

    void render_frame()
    {
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

private:
    bool wants_to_quit(SDL_Event& event)
    {
        bool sdl_quit = event.type == SDL_QUIT;
        bool sdl_window_close = event.type == SDL_WINDOWEVENT &&
                                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                                event.window.windowID == SDL_GetWindowID(window);

        return sdl_quit || sdl_window_close;
    }

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context{};
};

int main(int, char**)
{
    auto context = ImGui_SDL_GL_Context{};

    bool show_demo_window = true;

    bool done = false;
    while (!done)
    {
        done = context.process_events([](const SDL_Event& event) {
            // Handle custom events.
        });

        context.start_imgui_frame();

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            ImGui::Begin("Hello, world!");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::ColorEdit3("clear color", (float*) &context.clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        context.render_frame();
    }

    return 0;
}
