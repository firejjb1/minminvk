#pragma once

#include <GLFW/glfw3.h>

namespace Input
{
    struct KeyState
    {
        bool pressed = false;
    };

    KeyState W;
    KeyState A;
    KeyState S;
    KeyState D;

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        // press
        if (key == GLFW_KEY_W && action == GLFW_PRESS)
            W.pressed = true;
        if (key == GLFW_KEY_A && action == GLFW_PRESS)
            A.pressed = true;
        if (key == GLFW_KEY_S && action == GLFW_PRESS)
            S.pressed = true;
        if (key == GLFW_KEY_D && action == GLFW_PRESS)
            D.pressed = true;

        // release
        if (key == GLFW_KEY_W && action == GLFW_RELEASE)
            W.pressed = false;
        if (key == GLFW_KEY_A && action == GLFW_RELEASE)
            A.pressed = false;
        if (key == GLFW_KEY_S && action == GLFW_RELEASE)
            S.pressed = false;
        if (key == GLFW_KEY_D && action == GLFW_RELEASE)
            D.pressed = false;

    }
}