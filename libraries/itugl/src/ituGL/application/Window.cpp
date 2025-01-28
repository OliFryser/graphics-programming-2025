#include <ituGL/application/Window.h>

// Create the internal GLFW window. We provide some hints about it to OpenGL
Window::Window(int width, int height, const char* title) : m_window(nullptr)
{
<<<<<<< HEAD
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    m_window = window;
=======
    // Set some hints for window creation
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
>>>>>>> 5a8aa30e34b62f79f538a7ccda97a18336491680
}

// If we have an internal GLFW window, destroy it
Window::~Window()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
}

// Get the current dimensions (width and height) of the window
void Window::GetDimensions(int& width, int& height) const
{
    glfwGetWindowSize(m_window, &width, &height);
}

// Tell the window that it should close
void Window::Close()
{
    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

// Get if the window should be closed this frame
bool Window::ShouldClose() const
{
<<<<<<< HEAD
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
        return true;
    }
=======
    return glfwWindowShouldClose(m_window);
>>>>>>> 5a8aa30e34b62f79f538a7ccda97a18336491680
}

// Swaps the front and back buffers of the window
void Window::SwapBuffers()
{
    glfwSwapBuffers(m_window);
}
