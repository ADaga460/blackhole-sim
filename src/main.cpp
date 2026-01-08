#define GLEW_STATIC
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

// mingw32-make SHELL=cmd.exe

// -------------------- Data types --------------------
struct Vec2 {
    float x, y;
};

struct BlackHole {
    Vec2 pos;     // NDC position in [-1, 1]
    float rs;     // radius in NDC units
    bool exists;
};

struct AppState {
    int fbW = 800, fbH = 600;

    // Rays: each ray is a polyline of points in NDC
    std::vector<std::vector<Vec2>> rays;

    // Ray settings (tune these)
    int rayCount = 27;        // try 21..31
    float yMin = -0.85f;
    float yMax =  0.85f;
    float startX = -0.98f;

    // One black hole
    BlackHole bh{ {0,0}, 0.1f, false };

    // Click-and-hold sizing
    bool charging = false;
    double chargeStartT = 0.0;
    Vec2 chargePosNDC{0,0};
    float rsMin = 0.03f, rsMax = 0.25f, rsPerSecond = 0.09f;
};

// -------------------- Coordinate helpers --------------------
// TODO: Implement mouse position -> NDC conversion.
// Hint: Use framebuffer size, not window size (HiDPI correctness).
Vec2 screenToNDC(double mx, double my, int fbW, int fbH) {
    return Vec2{
        2.0f * static_cast<float>(mx) / static_cast<float>(fbW) - 1.0f,
        1.0f - 2.0f * static_cast<float>(my) / static_cast<float>(fbH)
    };
}

// -------------------- Simulation --------------------
// TODO: Generate rays.
// - If no BH: straight rays.
// - If BH exists: bent rays (marching).
void recomputeRays(AppState& app) {
    app.rays.clear();
    app.rays.reserve(app.rayCount);
    
    for (int i = 0; i < app.rayCount; ++i) {
        float t = (app.rayCount == 1) ? 0.5f : static_cast<float>(i) / static_cast<float>(app.rayCount - 1);
        float y = app.yMin + t * (app.yMax - app.yMin);
        Vec2 start = { app.startX, y };
        
        std::vector<Vec2> ray;
        
        if (!app.bh.exists) {
            ray.push_back(start);
            ray.push_back({ 1.0f, y });
        } else {
            Vec2 relPos = { start.x - app.bh.pos.x, start.y - app.bh.pos.y };
            float r = std::sqrt(relPos.x * relPos.x + relPos.y * relPos.y);
            r = std::max(r, app.bh.rs * 1.01f);
            
            // Initial angle
            float phi = std::atan2(relPos.y, relPos.x);
            
            Vec2 vel = { 1.0f, (t - 0.5f) * 0.1f };
            float vMag = std::sqrt(vel.x * vel.x + vel.y * vel.y);
            vel.x /= vMag;
            vel.y /= vMag;
            
            float cosPhi = std::cos(phi), sinPhi = std::sin(phi);
            Vec2 e_r = { cosPhi, sinPhi };
            Vec2 e_phi = { -sinPhi, cosPhi };
            
            float v_r = vel.x * e_r.x + vel.y * e_r.y;
            float v_phi = vel.x * e_phi.x + vel.y * e_phi.y;
            
            float b = std::fabs(r * v_phi);
            if (b < 0.01f) b = 0.01f;
            
            float phi_direction = (v_phi >= 0) ? 1.0f : -1.0f;
            
            float u = 1.0f / r;
            
            // du/dφ = -v_r / |L| * sign(L)
            float dudphi = -v_r / b * phi_direction;
            
            const float dPhi = 0.01f * phi_direction; // Step
            const int maxSteps = 12000;
            
            ray.push_back(start);
            
            for (int step = 0; step < maxSteps; ++step) {
                // Schwarzschild geodesic eq
                // d²u/dφ² + u = (3/2)*Rs*u²
                auto f_u = [](float u, float du) { return du; };
                auto f_du = [&](float u, float du) { return -u + 1.5f * app.bh.rs * u * u; };
                
                // RK4
                float k1u = f_u(u, dudphi);
                float k1du = f_du(u, dudphi);
                
                float k2u = f_u(u + 0.5f*dPhi*k1u, dudphi + 0.5f*dPhi*k1du);
                float k2du = f_du(u + 0.5f*dPhi*k1u, dudphi + 0.5f*dPhi*k1du);
                
                float k3u = f_u(u + 0.5f*dPhi*k2u, dudphi + 0.5f*dPhi*k2du);
                float k3du = f_du(u + 0.5f*dPhi*k2u, dudphi + 0.5f*dPhi*k2du);
                
                float k4u = f_u(u + dPhi*k3u, dudphi + dPhi*k3du);
                float k4du = f_du(u + dPhi*k3u, dudphi + dPhi*k3du);
                
                u += (dPhi/6.0f) * (k1u + 2*k2u + 2*k3u + k4u);
                dudphi += (dPhi/6.0f) * (k1du + 2*k2du + 2*k3du + k4du);
                phi += dPhi;
                if (u <= 0.0f || !std::isfinite(u) || !std::isfinite(dudphi)) break;
                if (u >= 1.0f / app.bh.rs) break; // Event horizon
                
                r = 1.0f / u;
                Vec2 pos = {
                    app.bh.pos.x + r * std::cos(phi),
                    app.bh.pos.y + r * std::sin(phi)
                };
                if (pos.x >= 1.0f) {
                    ray.push_back({1.0f, pos.y});
                    break;
                }
                if (pos.y < -1.2f || pos.y > 1.2f || pos.x < -1.5f) break;
                
                if (step % 4 == 0) ray.push_back(pos);
            }
        }
        
        app.rays.push_back(ray);
    }
}

// TODO (optional): Write helpers like:
// std::vector<Vec2> makeStraightRay(...)
// std::vector<Vec2> makeBentRayStable(...)

// -------------------- OpenGL helpers --------------------
// TODO: compileShader, makeProgram, create VAO/VBO, etc.
// Hint: Follow LearnOpenGL Hello Triangle structure. [web:23]
static GLuint compileShader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n";
    }
    return sh;
}

static GLuint makeProgram(const char* vsSrc, const char* fsSrc) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, 1024, nullptr, log);
        std::cerr << "Program link error:\n" << log << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

static std::vector<Vec2> makeCircle(Vec2 c, float r, int segments = 100) {
    std::vector<Vec2> pts;
    pts.reserve(segments);
    for (int i = 0; i < segments; ++i) {
        float a = (float)i / (float)segments * 6.28318530718f;
        pts.push_back({c.x + std::cos(a) * r, c.y + std::sin(a) * r});
    }
    return pts;
}


// -------------------- GLFW callbacks --------------------
// TODO: framebuffer resize callback
void framebufferSizeCallback(GLFWwindow* window, int w, int h) {
    // 1) glViewport(0,0,w,h)
    // 2) store w/h into app state (use glfwGetWindowUserPointer)
    glViewport(0, 0, w, h);
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->fbW = w;
        app->fbH = h;
    }
}

// TODO: key callback (SPACE resets BH, ESC quits)
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // if (action != GLFW_PRESS) return;
    // if (key == GLFW_KEY_SPACE) { clear BH; recomputeRays(); }
    // if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (action != GLFW_PRESS) return;
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    if (key == GLFW_KEY_SPACE) {
        app->bh.exists = false;
        app->charging = false;
        recomputeRays(*app);
    }
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            app->charging = true;
            app->chargeStartT = glfwGetTime();
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            app->chargePosNDC = screenToNDC(mx, my, app->fbW, app->fbH);
            std::cout << "Charging BH at (" << app->chargePosNDC.x << ", " << app->chargePosNDC.y << ")\n";
        }   
    } else if (action == GLFW_RELEASE) {
        AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && app->charging) {
            double held = glfwGetTime() - app->chargeStartT;
            float rs = std::clamp(app->rsMin + static_cast<float>(held) * app->rsPerSecond, app->rsMin, app->rsMax);
            app->bh = { app->chargePosNDC, rs, true };
            app->charging = false;
            recomputeRays(*app);
            std::cout << "Placed BH with rs=" << rs << "\n";
        }
    }
}

// -------------------- main --------------------
int main() {
    // Step 1: init GLFW + create window + make context current. [web:129]
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    // Step 2: init GLEW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Black Hole Simulator)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }
    
    // Step 3: setup shaders + VAO/VBO (modern OpenGL). [web:23]
    AppState app;
    glfwSetWindowUserPointer(window, &app);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwGetFramebufferSize(window, &app.fbW, &app.fbH);
    glViewport(0, 0, app.fbW, app.fbH);
    // Step 4: set callbacks (mouse/key/framebuffer). [web:40][web:146]
    // initialize listeners

    // Step 5: recomputeRays(app) once at startup
    recomputeRays(app);
    const char* vsSrc = R"(
#version 330 core
layout(location=0) in vec2 aPos;
void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
)";

    const char* fsSrc = R"(
#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main() { FragColor = vec4(uColor, 1.0); }
)";

    GLuint program = makeProgram(vsSrc, fsSrc);
    GLint uColorLoc = glGetUniformLocation(program, "uColor");

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 30000 * sizeof(Vec2), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);

    glBindVertexArray(0);
    
    // Main loop:
    //   glClear
    //   draw each ray polyline (GL_LINE_STRIP)
    //   draw BH circle outline if exists or charging preview
    //   glfwSwapBuffers, glfwPollEvents
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program);
        glBindVertexArray(vao);
        
        // when the user clicks, draw a black hole at the click position
        // with radius based on how long the mouse button is held down
        if (app.charging) {
            double held = glfwGetTime() - app.chargeStartT;
            float rs = std::clamp(app.rsMin + static_cast<float>(held) * app.rsPerSecond, app.rsMin, app.rsMax);
            auto circle = makeCircle(app.chargePosNDC, rs, 100);
            glUniform3f(uColorLoc, 0.4f, 1.0f, 0.2f);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, circle.size() * sizeof(Vec2), circle.data());
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)circle.size());
        }
        
        if (app.bh.exists) {
            auto circle = makeCircle(app.bh.pos, app.bh.rs, 100);
            glUniform3f(uColorLoc, 1.0f, 0.55f, 0.2f);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, circle.size() * sizeof(Vec2), circle.data());
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)circle.size());
        }

        // Draw rays
        glUniform3f(uColorLoc, 0.8f, 0.8f, 0.8f);
        for (const auto& ray : app.rays) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, ray.size() * sizeof(Vec2), ray.data());
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)ray.size());
        }

        
        glBindVertexArray(0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
