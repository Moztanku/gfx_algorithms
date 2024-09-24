#pragma once

#include <numbers>
#include <vector>
#include <random>

#include <immer/vector.hpp>

struct vertex
{
    float x, y{0.f};
};

struct polygon
{
    std::vector<vertex> vertices;
};

struct circle
{
    vertex center;
    float radius{0.f};
};

bool inside_of(const vertex& v, const circle& c)
{
    // Point (x, y) is inside of circle (a, b, r) if
    // (x - a)² + (y - b)² <= r²
    return
        (v.x - c.center.x) * (v.x - c.center.x) +
        (v.y - c.center.y) * (v.y - c.center.y) <=
        c.radius * c.radius;
}

circle welzl(
    const immer::vector<vertex>& P,
    const immer::vector<vertex>& R)
{
    if (P.empty() || R.size() == 3)
    {
        if (R.empty())
            return {{0.f, 0.f}, 0.f};

        else if (R.size() == 1)
            return {R[0], 0.f};

        else if (R.size() == 2)
        {
            const auto c = vertex{
                (R[0].x + R[1].x) / 2.f,
                (R[0].y + R[1].y) / 2.f
            };

            const auto r = std::sqrt(
                (R[0].x - c.x) * (R[0].x - c.x) +
                (R[0].y - c.y) * (R[0].y - c.y)
            );

            return {c, r};
        }

        else
        {
            const auto a = R[0];
            const auto b = R[1];
            const auto c = R[2];

            const auto d = 2.f * (a.x * (b.y - c.y) +
                                  b.x * (c.y - a.y) +
                                  c.x * (a.y - b.y));

            const auto ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) +
                             (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                             (c.x * c.x + c.y * c.y) * (a.y - b.y)) / d;

            const auto uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) +
                             (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                             (c.x * c.x + c.y * c.y) * (b.x - a.x)) / d;

            const auto r = std::sqrt(
                (a.x - ux) * (a.x - ux) +
                (a.y - uy) * (a.y - uy)
            );

            return {{ux, uy}, r};
        }
    }

    // remove p from P
    const auto P_ = P.take(P.size() - 1);

    circle D = welzl(P_, R);

    const auto p = P.back();

    if (inside_of(p, D))
        return D;

    // add p to R
    const auto R_ = R.push_back(p);

    // recursively call welzl
    return welzl(P_, R_);
}

circle bounding_circle(const polygon& poly)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<vertex> vertices = poly.vertices;

    std::shuffle(vertices.begin(), vertices.end(), gen);

    const immer::vector<vertex> P(
        vertices.begin(),
        vertices.end()
    );

    return welzl(P, {});
}

polygon polygonize(const circle& c, const size_t n)
{
    if (n < 3)
        return {};

    std::vector<vertex> vertices(n);

    for (size_t i{0}; i < n; i++)
    {
        const float angle = 2.f * std::numbers::pi * i / n;

        vertices[i] = {
            c.center.x + c.radius * std::cos(angle),
            c.center.y + c.radius * std::sin(angle)
        };
    }

    return {vertices};
}
