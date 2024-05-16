#include <glad/glad.h> // OpenGL libs
#include <GLFW/glfw3.h>

#include <iostream> // other libs

int main(void) {
    GLFWwindow* window;

    if (!glfwInit()) {
        return -1;
    }
        
    window = glfwCreateWindow(640, 480, "Hello Window :)", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cerr << "Can't initialize GLAD!" << std::endl;
        return -1;
    }

    std::cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << std::endl;

    glClearColor(0, 1, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);



        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
