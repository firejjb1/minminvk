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
    KeyState E;
    KeyState Q;

    struct MouseState
    {
        bool leftPressed = false;
        bool rightPressed = false;
        double xpos = 0;
        double ypos = 0;
    };
    MouseState mouseState;

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
        if (key == GLFW_KEY_Q && action == GLFW_PRESS)
            Q.pressed = true;
        if (key == GLFW_KEY_E && action == GLFW_PRESS)
            E.pressed = true;

        // release
        if (key == GLFW_KEY_W && action == GLFW_RELEASE)
            W.pressed = false;
        if (key == GLFW_KEY_A && action == GLFW_RELEASE)
            A.pressed = false;
        if (key == GLFW_KEY_S && action == GLFW_RELEASE)
            S.pressed = false;
        if (key == GLFW_KEY_D && action == GLFW_RELEASE)
            D.pressed = false;
        if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
            Q.pressed = false;
        if (key == GLFW_KEY_E && action == GLFW_RELEASE)
            E.pressed = false;

    }

    static void mousePosCallback(double xpos, double ypos)
    {
        mouseState.xpos = xpos;
        mouseState.ypos = ypos;
    }

    static void mouseButtonCallback(int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            mouseState.leftPressed = true;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            mouseState.rightPressed = true;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            mouseState.leftPressed = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            mouseState.rightPressed = false;
        }
    }
}