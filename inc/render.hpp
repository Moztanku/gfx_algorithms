#pragma once

#include <glad/gl.h>

#include "geometry.hpp"

void clear()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.2f);
    glClear(GL_COLOR_BUFFER_BIT);
}

int create_shader()
{
    constexpr const char* vert_src = R"(
        #version 460 core

        layout (location = 0) in vec2 aPos;

        void main()
        {
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    constexpr const char* frag_src = R"(
        #version 460 core

        uniform vec4 uColor;

        out vec4 fragColor;

        void main()
        {
            fragColor = uColor;
        }
    )";

    const auto vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vert_src, nullptr);
    glCompileShader(vert);

    const auto frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &frag_src, nullptr);
    glCompileShader(frag);

    const auto program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

void render(const polygon& poly, const uint32_t color = 0xFF'FF'FF'FF)
{
    static auto shader = create_shader();

    glUseProgram(shader);

    const auto vertices = poly.vertices;

    auto vbo = GLuint{};
    auto vao = GLuint{};

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(vertex),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr);
    glEnableVertexAttribArray(0);

    const float R = ((color >> 24) & 0xFF) / 255.f;
    const float G = ((color >> 16) & 0xFF) / 255.f;
    const float B = ((color >> 8) & 0xFF) / 255.f;
    const float A = ((color >> 0) & 0xFF) / 255.f;

    static auto uniform = glGetUniformLocation(shader, "uColor");

    glUniform4f(uniform, R, G, B, A);

    glDrawArrays(GL_LINE_LOOP, 0, vertices.size());

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glUseProgram(0);
}
