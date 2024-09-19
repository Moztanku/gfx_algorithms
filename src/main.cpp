#include "window.hpp"
#include "render.hpp"
#include "geometry.hpp"

#include <iostream>

void add_vertex(polygon& pol, const vertex& v)
{
    pol.vertices.push_back(v);
}

void handle_mouse(GLFWwindow* window, polygon& pol, circle& c)
{
    static bool is_pressed = false;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!is_pressed)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            int w, h;
            glfwGetWindowSize(window, &w, &h);

            // transform coordinates to [-1, 1]² space
            double x_ = x / w * 2.0 - 1.0;
            double y_ = 1.0 - y / h * 2.0;

            const auto v = vertex{
                static_cast<float>(x_),
                static_cast<float>(y_)
            };

            add_vertex(pol, v);

            c = bounding_circle(pol);
        }

        is_pressed = true;
    }

    else
    {
        is_pressed = false;
    }
}

int main()
{
    GLFWwindow* window = create_window(
        800,
        600,
        "Hello, World!",
        false
    );

    polygon pol;
    circle c;

    while (!glfwWindowShouldClose(window))
    {
        clear();

        handle_mouse(window, pol, c);

        render(pol, 0xFF'00'00'A0);
        
        render(
            polygonize(c, 32),
            0xFF'FF'00'A0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
