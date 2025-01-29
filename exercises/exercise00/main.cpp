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

unsigned int BuildShaderProgram();
void processInput(GLFWwindow* window);
void updateVertices(float vertices[], size_t count, float angle);
inline static double convert(double degree)
{
    double pi = 3.14159265359;
    return (degree * (pi / 180));
}

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const float length = sqrtf(2.0f) / 2.0f;

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

    float verticesArray[] = {
        -0.5f, -0.5f, 0.0f, // bottom left  
         0.5f, -0.5f, 0.0f, // bottom right 
         0.5f,  0.5f, 0.0f,  // top right
        -0.5f, 0.5f, 0.0f, // top left
    };

    auto vertexCount = sizeof(verticesArray) / sizeof(float);

    float indicesArray[] = {
        0, 1, 2, // first triangle
        2, 0, 3 // second triangle
    };

    auto indexCount = sizeof(indicesArray) / sizeof(float);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f, // bottom left  
         0.5f, -0.5f, 0.0f, // bottom right 
         0.5f,  0.5f, 0.0f,  // top right
        -0.5f, 0.5f, 0.0f, // top left
    };
    
    std::vector<float> indices = {
        0, 1, 2, // first triangle
        2, 0, 3 // second triangle
    };

    VertexBufferObject vbo;
    VertexArrayObject vao;
    ElementBufferObject ebo;

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    vao.Bind();
  
    vbo.Bind();
    vbo.AllocateData<const float>(std::span(verticesArray, vertexCount));

    ebo.Bind();
    ebo.AllocateData<const float>(std::span(indicesArray, indexCount));
    
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

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        processInput(window.GetInternalWindow());

        //update
        float angle = time * rotationSpeed;

        // render
        // ------
        device.Clear(.5f, 0.25f, .75f, 1.0f);

        // draw our first triangle
        glUseProgram(shaderProgram);
        vao.Bind(); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        window.SwapBuffers();
        device.PollEvents();
        time += .1f;
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
}

void updateVertices(float vertices[], size_t count, float angle)
{
    angle += convert(45);
    for (size_t i = 0; i < count; i+=3)
    {
        // x
        vertices[i] = std::sin(angle) * length;
        // y
        vertices[i + 1] = std::cos(angle) * length;
        // z
        vertices[i + 2] = 0.0f;
        angle += convert(90);
    }
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
