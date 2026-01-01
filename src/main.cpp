#define GLEW_STATIC
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    std::cout << "Starting program..." << std::endl;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    std::cout << "GLFW initialized!" << std::endl;

    // Optional: Tell GLFW we want a specific OpenGL version (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "Window created!" << std::endl;

    glfwMakeContextCurrent(window);

    // This is a common fix for GLEW on newer hardware
    glewExperimental = GL_TRUE; 

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(glewStatus) << std::endl;
        return -1;
    }
    std::cout << "GLEW initialized! OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "Closing program..." << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// g++ main.cpp -o main -static -I"C:\Users\aarav\Downloads\glfw-3.4.bin.WIN64\glfw-3.4.bin.WIN64\include" -I"C:\Users\aarav\Downloads\glew-2.3.0-win32\glew-2.3.0\include" -L"C:\Users\aarav\Downloads\glfw-3.4.bin.WIN64\glfw-3.4.bin.WIN64\lib-mingw-w64" -L"C:\Users\aarav\Downloads\glew-2.3.0-win32\glew-2.3.0\lib\Release\x64" -lglfw3 -lglew32s -lopengl32 -lgdi32