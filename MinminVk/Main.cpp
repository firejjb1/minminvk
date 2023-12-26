// macros to setup depending on settings
#define VULKAN_IMPL
//#define NODEBUG
// end macros
#include "Graphics.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace Application
{
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    GLFWwindow* window;
    void InitApplication()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);

        Graphics::InitGraphics();

    }

    void ApplicationLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            Graphics::MainRender();
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