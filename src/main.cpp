#include <glad/glad.h> // OpenGL libs
#include <GLFW/glfw3.h>

#include <iostream> // other libs

int sizeX = 640, sizeY = 480;

void sizeHandler(GLFWwindow* win, int width, int heght) {
    sizeX = width;
    sizeY = heght;
    glViewport(0, 0, width, heght);
}

void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        std::cout << "Move forward" << std::endl;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        std::cout << "Move backward" << std::endl;
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        std::cout << "Strafe left" << std::endl;
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        std::cout << "Strafe right" << std::endl;
    }

    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }
}

int main(void) {
    GLFWwindow* window1;
    GLFWwindow* window2;

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window1 = glfwCreateWindow(sizeX, sizeY, "M2 engine handler", NULL, NULL);
    if (!window1) {
        std::cerr << "Failed to create window!";
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window1, sizeHandler);
    glfwSetKeyCallback(window1, keyHandler);

    glfwMakeContextCurrent(window1);

    if (!gladLoadGL()) {
        std::cerr << "Can't initialize GLAD!" << std::endl;
        return -1;
    }

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glClearColor(1, 1, 1, 1);

    while (!glfwWindowShouldClose(window1)) {
        glClear(GL_COLOR_BUFFER_BIT);



        glfwSwapBuffers(window1);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}