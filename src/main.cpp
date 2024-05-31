#include <glad/glad.h> // OpenGL libs
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/ShaderProgram.hpp" // my libs
#include "Renderer/Texture2D.hpp"
#include "Resources/ResourceManager.hpp"

#include <iostream> // other libs
#include <string>

GLfloat point[] = {
     0,  100, 0,
     0, 0, 0,
    -100, 0, 0
};

GLfloat colors[] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
};

GLfloat texCoord[] = {
    1.f, 1.f,
    1.f, 0.f,
    0.f, 0.f
};

glm::ivec2 Size(640, 480);

void sizeHandler(GLFWwindow* win, int width, int height) {
    Size.x = width;
    Size.y = height;
    glViewport(0, 0, Size.x, Size.y);
}

void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "M2 Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, sizeHandler);
    glfwSetKeyCallback(window, keyHandler);

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cerr << "Can't load GLAD!" << std::endl;
    }

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glClearColor(1, 1, 0, 1);

    {
        ResourceManager resMan(argv[0]);

        auto defaultShaderProgram = resMan.loadShaders("Default", "res/shaders/vertex.txt", "res/shaders/fragment.txt");

        if (!defaultShaderProgram) {
            std::cerr << "Can't create shader program!" << std::endl;
            return -1;
        }

        resMan.loadTexture("Default", "res/textures/texture.png");

        auto tex = resMan.loadTexture("DefaultTexture", "res/textures/texture.png");

        GLuint points_vbo = 0;
        glGenBuffers(1, &points_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);

        GLuint colors_vbo = 0;
        glGenBuffers(1, &colors_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

        GLuint texCoord_vbo = 0;
        glGenBuffers(1, &texCoord_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, texCoord_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_STATIC_DRAW);

        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, texCoord_vbo);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        defaultShaderProgram->use();
        defaultShaderProgram->setInt("tex", 0);

        glm::mat4 projMat = glm::ortho(0.f, static_cast <float> (Size.x), 0.f, static_cast <float> (Size.y), -100.f, 100.f);

        defaultShaderProgram->setMat4("projMat", projMat);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            defaultShaderProgram->use();
            glBindVertexArray(vao);
            tex->bind();

            for (int i = 0; i < 4; i++) {
                glm::mat4 modelMat_curr = glm::mat4(1.0f);
                modelMat_curr =  glm::translate(modelMat_curr, glm::vec3(100.f * (i + 1), 100.f * i, 0.f));

                defaultShaderProgram->setMat4("modelMat", modelMat_curr);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}