#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>

// 1. GLFW: Download from https://www.glfw.org/download.html
//                        "C:\Users\aarav\Downloads\glfw-3.4.zip\glfw-3.4\include\GLFW"
// 2. GLEW: Download from http://glew.sourceforge.net/
//                     "C:\Users\aarav\Downloads\glew-2.3.0-win32.zip\glew-2.3.0\include\GL"
// 3. GLM: Download from https://github.com/g-truc/glm
//                       "C:\Users\aarav\glm\glm"

int main()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Black Hole Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "All libraries loaded successfully!" << std::endl;

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}