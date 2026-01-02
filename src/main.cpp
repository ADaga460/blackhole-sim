#define GLEW_STATIC
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

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
    BlackHole bh{ {100,100}, 0.1f, false };

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
    // return { ... };
    return {0,0};
}

// -------------------- Simulation --------------------
// TODO: Generate rays.
// - If no BH: straight rays.
// - If BH exists: bent rays (marching).
void recomputeRays(AppState& app) {
    // app.rays.clear();
    // app.rays.reserve(app.rayCount);

    // For each ray i:
    //   float t = i/(rayCount-1)
    //   float y = lerp(yMin, yMax, t)
    //   Vec2 start = {startX, y}
    //
    //   if (!bh.exists) push_back straight polyline
    //   else push_back bent polyline
}

// TODO (optional): Write helpers like:
// std::vector<Vec2> makeStraightRay(...)
// std::vector<Vec2> makeBentRayStable(...)

// -------------------- OpenGL helpers --------------------
// TODO: compileShader, makeProgram, create VAO/VBO, etc.
// Hint: Follow LearnOpenGL Hello Triangle structure. [web:23]

// -------------------- GLFW callbacks --------------------
// TODO: framebuffer resize callback
void framebufferSizeCallback(GLFWwindow* window, int w, int h) {
    // 1) glViewport(0,0,w,h)
    // 2) store w/h into app state (use glfwGetWindowUserPointer)
}

// TODO: key callback (SPACE resets BH, ESC quits)
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // if (action != GLFW_PRESS) return;
    // if (key == GLFW_KEY_SPACE) { clear BH; recomputeRays(); }
    // if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void DrawCircle(float cx, float cy, float r, int num_segments)
{
    glBegin(GL_LINE_LOOP);
    for(int ii = 0; ii < num_segments; ii++)
    {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle

        float x = r * cosf(theta);//calculate the x component
        float y = r * sinf(theta);//calculate the y component

        glVertex2f(x + cx, y + cy);//output vertex

    }
    //std::cout << "Drew circle at (" << cx << ", " << cy << ") with r=" << r << "\n" << std::endl;
    glEnd();
}

void drawBlackHole(const BlackHole& bh) {
    DrawCircle(bh.pos.x, bh.pos.y, bh.rs, 100);
    //std::cout << "Drew bh at (" << bh.pos.x << ", " << bh.pos.y << ") with r=" << bh.rs << "\n" << std::endl;
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            app->charging = true;
            app->chargeStartT = glfwGetTime();
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            app->chargePosNDC = Vec2{
                2.0f * static_cast<float>(mx) / static_cast<float>(app->fbW) - 1.0f,
                1.0f - 2.0f * static_cast<float>(my) / static_cast<float>(app->fbH)
            };
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
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }
    // Main loop:
    //   glClear
    //   draw each ray polyline (GL_LINE_STRIP)
    //   draw BH circle outline if exists or charging preview
    //   glfwSwapBuffers, glfwPollEvents
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // when the user clicks, draw a black hole at the click position
        // with radius based on how long the mouse button is held down
        if (app.charging) {
            double held = glfwGetTime() - app.chargeStartT;
            float rs = std::clamp(app.rsMin + static_cast<float>(held) * app.rsPerSecond, app.rsMin, app.rsMax);
            drawBlackHole(BlackHole{ app.chargePosNDC, rs, true });
        }
    }
    glfwTerminate();

    return 0;
}
