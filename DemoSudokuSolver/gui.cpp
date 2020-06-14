
// dear imgui: standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

#include "imgui-master/imgui.h"
#include "imgui-master/examples/imgui_impl_glfw.h"
#include "imgui-master/examples/imgui_impl_opengl2.h"

#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#include "sudoku.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

SudokuInstance gSudoku;
std::string gFilename = "session.txt";

void sudoku_window()
{
    ImGui::SetNextWindowPos({ 16, 16 });
    ImGui::Begin("Sudoku", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoMove |
        0
    );

    ImGui::SetWindowFontScale(5);
    ImGui::PushItemWidth(32);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4, 4 });
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    for(int i = 0; i < 9; ++i)
    {
        for(int j = 0; j < 9; ++j)
        {
            ImGui::PushStyleVar(
                ImGuiStyleVar_ItemSpacing,
                { j % 3 == 2 ? 16.f : 4.f, i % 3 == 2 ? 16.f : 4.f }
            );

            const Position pos(i, j);
            auto &num = gSudoku.cells[i][j];
            const auto candidates = gSudoku.candidates(pos);
            const auto locked = gSudoku.is_locked(pos);
            if(locked)
            {
                ImGui::PushStyleColor(
                    ImGuiCol_Button,
                    { 0, 0, 0, 0 }
                );
                ImGui::PushStyleColor(
                    ImGuiCol_Border,
                    { 0, 0, 0, 0 }
                );
            }
            else // color according to the number of candidates
            {
                const auto num_candidates = candidates.count();
                if(num_candidates == 0 && num == 0)
                {
                    ImGui::PushStyleColor(
                        ImGuiCol_Button,
                        { 1.f, 0.f, 0.f, 0.5f }
                    );
                }
                else
                {
                    const auto col = num_candidates * 0.1f;
                    ImGui::PushStyleColor(
                        ImGuiCol_Button,
                        { col, col, col, 0.5f }
                    );
                }
                ImGui::PushStyleColor(
                    ImGuiCol_Border,
                    { 1.f, 1.f, 1.f, 0.5f }
                );
            }

            const auto btn_name = num
                ? fmt::format("{}##{}-{}", num, i, j)
                : fmt::format("##{}-{}", i, j);
            const auto popup_name = fmt::format("cell_fill_popup##{}-{}", i, j);

            if(ImGui::Button(btn_name.c_str(), { 88 , 88 }) && !locked)
                ImGui::OpenPopup(popup_name.c_str());
            if(ImGui::IsItemClicked(1) && !locked)
                gSudoku.fill(pos, 0);
            if(ImGui::IsItemClicked(2))
                gSudoku.toggle_locked(pos);

            if(ImGui::BeginPopup(popup_name.c_str()))
            {
                for(int c = 0; c < 10; ++c)
                {
                    if(!candidates[c]) continue;
                    if(ImGui::Selectable(fmt::format("{}", c).c_str()))
                    {
                        gSudoku.fill(pos, c);
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::PopStyleColor(2);
            if(j != 8) ImGui::SameLine();
            ImGui::PopStyleVar(1);
            // if(j % 3 == 2) putchar(' ');

        // if(i % 3 == 2) putchar('\n');
        // putchar('\n');
        }
    }
    ImGui::PopStyleVar(2);
    ImGui::PopItemWidth();

    if(ImGui::Button("Solve"))
    {
        fmt::print("Trying to solve the puzzle:\n\n");
        gSudoku.print();
        const auto solved = gSudoku.solve__fewest_candidate_first();
        if(!solved)
        {
            fmt::print("No solution could be found.\n");
        }
        else
        {
            fmt::print("Got a solution in {} steps:\n\n", gSudoku.steps);
            gSudoku.print();
        }
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset"))
        gSudoku.reset();
    ImGui::SameLine();
    if(ImGui::Button("Save"))
        gSudoku.save(gFilename);

    ImGui::End();
}

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        gFilename = argv[1];
    }
    gSudoku.read_from_file(gFilename);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(900, 990, "Sudoku", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0, 0, 0, 1.00f);


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        sudoku_window();
        // ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
