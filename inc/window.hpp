#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

GLFWwindow* create_window(int width, int height, const char* title, bool fullscreen = false)
{
    if (!glfwInit())
    {
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(
        width,
        height,
        title,
        fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr);

    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoaderLoadGL())
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    return window;
}
