#include <graphics/Graphics.h>
#include <Input.h>
#include <UI.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_glfw.h>

namespace Application
{
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    GLFWwindow* window;

    u32 frameID = 0;

    void InitApplication()
    {

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
        
        glfwSetKeyCallback(window, Input::keyCallback);

        Graphics::InitGraphics(window);

        // IMGui init
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();

            Graphics::InitUI();

        }
    }

    void ApplicationLoop()
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(window)) {
            auto currentTime = std::chrono::high_resolution_clock::now();

            glfwPollEvents();

            // IMGUI
            {
                //ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();


                ImGui::Begin("Renderer Options");
                {
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::Separator();
                    ImGui::Text("Hair Parameters");
                    bool resetHead = ImGui::Button("Reset Head Position");
                    ImGui::Checkbox("Rotate Head", &UI::rotateHead);

                    ImGui::SliderFloat("Wind Strength", &UI::windStrength, 0, 100);
                    ImGui::InputFloat3("Wind Direction", glm::value_ptr(UI::windDirection));
                    f32 shockStrength = 1;
                    ImGui::SliderFloat("Shock Strength", &UI::shockStrength, 0, 100);
                    int elc = UI::elcIteration;
                    ImGui::SliderInt("Edge Length Constraint Iteration", &elc, 0, 20);
                    UI::elcIteration = elc;
                    f32 stiffnessLocal = 0.5f;
                    ImGui::SliderFloat("Stiffness Local", &UI::stiffnessLocal, 0, 1);
                    ImGui::SliderFloat("Stiffness Global", &UI::stiffnessGlobal, 0, 1);
                    ImGui::SliderFloat("Range Global Constraint", &UI::effectiveRangeGlobal, 0, 1);
                    ImGui::SliderFloat("Capsule radius", &UI::capsuleRadius, 0, 0.2f);
                }
                ImGui::End();
                ImGui::Render();
            }

            auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            Graphics::MainRender(frameID, deltaTime);

            startTime = currentTime;
            frameID++;
        }
    }

    void CleanUp()
    {
        Graphics::CleanUp();
        {
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

int main() {
   
    Application::InitApplication();

    Application::ApplicationLoop();

    Application::CleanUp();
    return 0;
}