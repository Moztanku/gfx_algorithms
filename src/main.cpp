#include "window.hpp"
#include "render.hpp"
#include "geometry.hpp"
#include "input.hpp"

#include <iostream>

void add_vertex(polygon& pol, const vertex& v)
{
    pol.vertices.push_back(v);
}

bool pointPolygon(const polygon& pol, const vertex& p)
{
    bool inside{false};

    size_t next{0};
    for (size_t i = 0; i < pol.vertices.size(); i++)
    {
        next = (i + 1) % pol.vertices.size();

        const auto& A = pol.vertices[i];
        const auto& B = pol.vertices[next];

        if (
            ((A.y > p.y && B.y < p.y) || (A.y < p.y && B.y > p.y)) &&
            (p.x < (( B.x - A.x ) * ( p.y - A.y ) / ( B.y - A.y )) + A.x)
        ){
            inside = !inside;
        }
    }

    return inside;
}

bool pointCircle(const circle& c, const vertex& p)
{
    return distance(c.center, p) < c.radius;
}

bool pointLine(const vertex& A, const vertex& B, const vertex& p)
{
    float d1 = distance(A, p);
    float d2 = distance(B, p);

    float line_len = distance(A, B);

    constexpr float epsilon = 0.1f;

    return 
        d1 + d2 >= line_len - epsilon &&
        d1 + d2 <= line_len + epsilon;
}

bool lineCircle(const vertex& A, const vertex& B, const circle& c)
{
    if (pointCircle(c, A) || pointCircle(c, B))
    {
        return true;
    }

    float len = distance(A, B);

    float dot = (
        ( ( c.center.x - A.x ) * ( B.x - A.x ) ) +
        ( ( c.center.y - A.y ) * ( B.y - A.y ) )
    ) / ( len * len );

    float closest_x = A.x + ( dot * ( B.x - A.x ) );
    float closest_y = A.y + ( dot * ( B.y - A.y ) );

    if (!pointLine(A, B, {closest_x, closest_y}))
    {
        return false;
    }

    float d1 = distance({closest_x, closest_y}, c.center);

    return d1 < c.radius;
}


// Check whether a circle intersects with a polygon or is contained within it
bool intersectionOrWithin(const polygon& frustum, const circle& c)
{
    size_t next{0};

    if (pointPolygon(frustum, c.center))
    {
        return true;
    }

    for (size_t i = 0; i < frustum.vertices.size(); i++)
    {
        // Get the next vertex, i and next form a line segment
        next = (i + 1) % frustum.vertices.size();

        const auto& A = frustum.vertices[i];
        const auto& B = frustum.vertices[next];

        if (lineCircle(A, B, c))
        {
            return true;
        }
    }

    return false;
}

struct ray
{
    vertex O;
    vertex D;
};

float dot(const vertex& a, const vertex& b)
{
    return a.x * b.x + a.y * b.y;
}

vertex normalize(const vertex& v)
{
    const float len = sqrt(v.x * v.x + v.y * v.y);

    return {
        v.x / len,
        v.y / len
    };
}

vertex operator-(const vertex& a, const vertex& b)
{
    return {
        a.x - b.x,
        a.y - b.y
    };
}

bool rayCircle(const ray& ray, const circle& c)
{
    const vertex OC = {
        c.center.x - ray.O.x,
        c.center.y - ray.O.y
    };

    const float t = dot(ray.D, OC);
    const vertex P = {
        ray.O.x + t * ray.D.x,
        ray.O.y + t * ray.D.y
    };

    const float d = distance(P, c.center);

    return d < c.radius;
}
bool toggle{true};

polygon get_frustum(const vertex mouse_pos, const vertex camera_pos)
{
    polygon pol;

    constexpr float frustum_length  = 1.0f;
    constexpr float frustum_cutoff  = 0.02f;
    constexpr float frustum_angle   = M_PI / 4.0f;
    constexpr float angle_offset    = frustum_angle / 2.0f;

    const vertex center = camera_pos;
    const float angle = atan2f(mouse_pos.y - center.y, mouse_pos.x - center.x);

    std::cout << "Angle: " << angle << std::endl;

    /*
                                    _
            C------------D          | <- length
             |          |           |
              |        |            |
               |      |             |
                A----B  _           |
                 |  |   |<- cutoff  |
                  ||    |           |
                center  |           |

            |<--------->|
                angle
    */

    const vertex A = {
        center.x + frustum_cutoff * cosf(angle - angle_offset),
        center.y + frustum_cutoff * sinf(angle - angle_offset)
    };

    const vertex B = {
        center.x + frustum_cutoff * cosf(angle + angle_offset),
        center.y + frustum_cutoff * sinf(angle + angle_offset)
    };

    const vertex C = {
        center.x + frustum_length * cosf(angle - angle_offset),
        center.y + frustum_length * sinf(angle - angle_offset)
    };

    const vertex D = {
        center.x + frustum_length * cosf(angle + angle_offset),
        center.y + frustum_length * sinf(angle + angle_offset)
    };

    add_vertex(pol, A);
    add_vertex(pol, B);
    add_vertex(pol, D);
    add_vertex(pol, C);

    return pol;
}

int main()
{
    GLFWwindow* window = create_window(
        1000,
        1000,
        "Hello, World!",
        false
    );

    State state;

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_SPACE)
            {
                toggle = !toggle;
            }
        }
    });

    polygon camera_frustum;

    struct renderable
    {
        const polygon* pol{nullptr};
        float distance{0.0f};
        bool visible{false};
    };

    while (!glfwWindowShouldClose(window))
    {
        clear();

        handle_input(state);

        std::vector<renderable> objects;

        camera_frustum = get_frustum(
            {state.mouse_x, state.mouse_y},
            state.camera_pos
        );

        for (const auto& pol : state.polygons)
        {
            const circle boundary = bounding_circle(pol);
            const float d = distance(state.camera_pos, boundary.center);
            const bool within_frustum = intersectionOrWithin(camera_frustum, boundary);

            const renderable r = {
                .pol = &pol,
                .distance = d,
                .visible = within_frustum
            };

            objects.push_back(r);
        }

        std::sort(
            objects.begin(),
            objects.end(),
            [](const renderable& a, const renderable& b)
            {
                return a.distance > b.distance;
            }
        );

        for (const auto& obj : objects)
        {
            if (!obj.visible)
                continue;

            // Our object becomes a occluder, we need to check whether it occludes other objects
            circle occluder = bounding_circle(*obj.pol);

            for (auto& other : objects)
            {
                if (other.pol == obj.pol || !other.visible)
                    continue;

                bool occluded = true;

                for (const auto& v : other.pol->vertices)
                {
                    // We need to check if ray from camera to vertex passes through occluder twice
                    // If it does, the object should be occluded

                    // Ray from camera to vertex
                    const ray r = {
                        .O = state.camera_pos,
                        .D = normalize(v - state.camera_pos)
                    };

                    if (!rayCircle(r, occluder))
                    {
                        occluded = false;
                        break;
                    }
                }

                if (occluded)
                {
                    other.visible = false;
                }
            }
        }

        if (toggle)
        {
            render(camera_frustum, 0xFF'FF'FF'A0);
        }

        for (const auto& obj : objects)
        {
            render(*obj.pol, obj.visible ? 0xFF'AA'00'A0 : 0x55'33'00'A0);

            if (toggle)
            {
                circle c = bounding_circle(*obj.pol);

                render(
                    polygonize(c, 64),
                    obj.visible ? 0x00'FF'00'A0 : 0x00'55'00'A0
                );
            }
        }

        // Render currently drawn polygon in green
        if (state.current)
        {
            render(*state.current, 0x00'FF'00'A0);
        }
        
        // Render hovered vertex in yellow
        if (state.hovered_vertex)
        {
            circle c;

            c.center = *state.hovered_vertex;
            c.radius = 0.025f;

            render(
                polygonize(c, 64),
                0xFF'FF'00'A0
            );
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
