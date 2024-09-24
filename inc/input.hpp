#pragma once

#include <GLFW/glfw3.h>

#include <optional>

#include "geometry.hpp"

struct State
{
    // Mouse state
    bool mouse_pressed{false};

    float mouse_x{0.0f};
    float mouse_y{0.0f};

    // Additional buttons
    bool Q_pressed{false};
    bool W_pressed{false};
    bool E_pressed{false};

    // Polygons
    std::vector<polygon> polygons{};
    std::optional<polygon> current{std::nullopt};
    vertex* selected_vertex{nullptr};
    vertex* hovered_vertex{nullptr};

    // Camera
    vertex camera_pos{0.0f, 0.0f};
};

std::pair<float, float> get_mouse_pos()
{
    GLFWwindow* window = glfwGetCurrentContext();

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    // transform coordinates to [-1, 1]² space
    float x_ = x / w * 2.0 - 1.0;
    float y_ = 1.0 - y / h * 2.0;

    return {x_, y_};
}

void handle_mouse_drag(State& state)
{
    if (state.selected_vertex)
    {
        state.selected_vertex->x = state.mouse_x;
        state.selected_vertex->y = state.mouse_y;
    }
}

void handle_mouse_click(State& state)
{
    const vertex mouse = {state.mouse_x, state.mouse_y};

    if (state.hovered_vertex)
    {
        state.selected_vertex = state.hovered_vertex;
    }

    else
    {
        if (!state.current)
        {
            state.current = polygon{};
        }

        state.current->vertices.push_back(mouse);
    }
}

void handle_mouse_release(State& state)
{
    state.selected_vertex = nullptr;
}

float distance(const vertex& a, const vertex& b)
{
    return std::sqrt(
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y)
    );
}

void handle_mouse_move(State& state)
{
    const vertex mouse = {state.mouse_x, state.mouse_y};

    vertex* closest_vertex = nullptr;
    float closest_distance = 0.035f;

    for (auto& poly : state.polygons)
        for (auto& v : poly.vertices)
        {
            const float d = distance(mouse, v);

            if (d < closest_distance)
            {
                closest_vertex = &v;
                closest_distance = d;
            }
        }

    if (state.current)
        for (auto& v : state.current->vertices)
        {
            const float d = distance(mouse, v);

            if (d < closest_distance)
            {
                closest_vertex = &v;
                closest_distance = d;
            }
        }

    state.hovered_vertex = closest_vertex;
}

void handle_mouse(State& state)
{
    std::tie(state.mouse_x, state.mouse_y) = get_mouse_pos();

    GLFWwindow* window = glfwGetCurrentContext();
    bool is_pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (is_pressed && state.mouse_pressed)
        handle_mouse_drag(state);

    if (is_pressed && !state.mouse_pressed)
        handle_mouse_click(state);

    if (!is_pressed && state.mouse_pressed)
        handle_mouse_release(state);

    if (!is_pressed && !state.mouse_pressed)
        handle_mouse_move(state);

    state.mouse_pressed = is_pressed;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        state.camera_pos = {state.mouse_x, state.mouse_y};
}

void handle_Q(State& state)
{
    if (state.current && !state.current->vertices.empty())
    {
        state.polygons.push_back(*state.current);
        state.current = std::nullopt;
    }
}

void handle_W(State& state)
{
    if (state.current)
    {
        if (!state.current->vertices.empty())
            state.current->vertices.pop_back();
        else
            state.current = std::nullopt;
    }
}

void handle_E(State& state)
{
    if (state.polygons.size() > 0)
        state.polygons.pop_back();
}

void handle_keys(State& state)
{
    GLFWwindow* window = glfwGetCurrentContext();

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (!state.Q_pressed)
            handle_Q(state);

        state.Q_pressed = true;
    } else {
        state.Q_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (!state.W_pressed)
            handle_W(state);

        state.W_pressed = true;
    } else {
        state.W_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (!state.E_pressed)
            handle_E(state);

        state.E_pressed = true;
    } else {
        state.E_pressed = false;
    }
}

void handle_input(State& state)
{
    handle_mouse(state);
    handle_keys(state);
}
