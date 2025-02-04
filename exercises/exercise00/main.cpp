#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ituGL/core/DeviceGL.h>
#include <ituGL/application/Window.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <ituGL/geometry/ElementBufferObject.h>

#include <iostream>
#include <vector>

#include "Utils.h"
#include "Circle.h"

struct Vector2 {
    float x, y;
};

unsigned int BuildShaderProgram();
void processInput(GLFWwindow* window);
void updateVertices(std::vector<float>& vertices, float angle);
void printVector(std::vector<float> vertices);
void printVector(std::vector<float> vector);
Vector2 GetMovementVector(float movementSpeed);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const float length = sqrtf(2.0f) / 2.0f;

// input variables
bool left, right, up, down;

int main()
{
    DeviceGL device;

    // glfw window creation
    // --------------------

    Window window(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL");

    if (!window.IsValid())
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    device.SetCurrentWindow(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!device.IsReady())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int shaderProgram = BuildShaderProgram();

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f, // bottom left  
         0.5f, -0.5f, 0.0f, // bottom right 
         0.5f,  0.5f, 0.0f,  // top right
        -0.5f, 0.5f, 0.0f, // top left
    };

    std::vector<unsigned int> indices{
        0, 1, 2, // first triangle
        2, 0, 3 // second triangle
    };

    Circle circle(.5f, 100);

    VertexBufferObject vbo;
    VertexArrayObject vao;
    ElementBufferObject ebo;

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    vao.Bind();

    vbo.Bind();
    vbo.AllocateData<const float>(circle.vertices);

    ebo.Bind();
    ebo.AllocateData<const unsigned int>(circle.indices);

    VertexAttribute position(Data::Type::Float, 3);

    vao.SetAttribute(0, position, 0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    vbo.Unbind();

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    vao.Unbind();
    ebo.Unbind();

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float time = 0.0f;
    float rotationSpeed = 0.1f;
    float movementSpeed = 0.05f;

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        processInput(window.GetInternalWindow());

        //update
        Vector2 movementVector = GetMovementVector(movementSpeed);
        circle.TranslateCircle(movementVector.x, movementVector.y);

        vbo.Bind();
        vbo.UpdateData<const float>(circle.vertices, 0);
        vbo.Unbind();

        // render
        // ------
        const Color backgroundColor(0.0f, 0.5f, 0.5f, 1.0f);
        device.Clear(backgroundColor);

        // draw our first triangle
        glUseProgram(shaderProgram);
        vao.Bind(); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        glDrawElements(GL_TRIANGLES, circle.indices.size(), GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        window.SwapBuffers();
        device.PollEvents();
        time += .1f;

        vao.Unbind();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteProgram(shaderProgram);

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    up = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    down = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

}

void updateVertices(std::vector<float>& vertices, float angle)
{
    angle += ConvertDegreesToRadians(45);
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        // x
        vertices[i] = std::sin(angle) * length;
        // y
        vertices[i + 1] = std::cos(angle) * length;
        // z
        vertices[i + 2] = 0.0f;
        angle += ConvertDegreesToRadians(90);
    }
}

void printVector(std::vector<float> vector)
{
    for (const auto element : vector)
        std::cout << element << " ";
    std::cout << std::endl;
}

Vector2 GetMovementVector(float movementSpeed)
{
    Vector2 movementVector{};

    if (up && down)
        movementVector.y = 0.0f;
    else if (up)
        movementVector.y = movementSpeed;
    else if (down)
        movementVector.y = -movementSpeed;

    if (left && right)
        movementVector.x = 0.0f;
    else if (left)
        movementVector.x = -movementSpeed;
    else if (right)
        movementVector.x = movementSpeed;

    return movementVector;
}

std::vector<float> createCircleVertices(float radius, int detail)
{
    return std::vector<float>();
}

std::vector<unsigned int> createCircleIndices(float radius)
{
    return std::vector<unsigned int>();
}

unsigned int BuildShaderProgram()
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.2f, 0.2f, 1.0f);\n"
        "}\n\0";

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}