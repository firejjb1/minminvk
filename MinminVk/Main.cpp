#include <graphics/Graphics.h>
#include <GLFW/glfw3.h>
#include <iostream>

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
        
        Graphics::InitGraphics(window);

    }

    void ApplicationLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            Graphics::MainRender(frameID);

            frameID++;
        }
    }

    void CleanUp()
    {
        Graphics::CleanUp();
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