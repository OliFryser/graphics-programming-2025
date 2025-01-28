#include <ituGL/core/DeviceGL.h>

#include <ituGL/application/Window.h>
#include <GLFW/glfw3.h>
#include <cassert>

DeviceGL* DeviceGL::m_instance = nullptr;

DeviceGL::DeviceGL() : m_contextLoaded(false)
{
<<<<<<< HEAD
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
=======
    m_instance = this;

    // Init GLFW
    glfwInit();
>>>>>>> 5a8aa30e34b62f79f538a7ccda97a18336491680
}

DeviceGL::~DeviceGL()
{
<<<<<<< HEAD
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
=======
    m_instance = nullptr;

    // Terminate GLFW
    glfwTerminate();
>>>>>>> 5a8aa30e34b62f79f538a7ccda97a18336491680
}

// Set the window that OpenGL will use for rendering
void DeviceGL::SetCurrentWindow(Window& window)
{
<<<<<<< HEAD
	GLFWwindow* glfwWindow = window.GetInternalWindow();
	glfwMakeContextCurrent(glfwWindow);
	glfwSetFramebufferSizeCallback(glfwWindow, FrameBufferResized);
	
	m_contextLoaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
=======
    GLFWwindow* glfwWindow = window.GetInternalWindow();
    glfwMakeContextCurrent(glfwWindow);

    // Load required GL libraries and initialize the context
    m_contextLoaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (m_contextLoaded)
    {
        // Set callback to be called when the window is resized
        glfwSetFramebufferSizeCallback(glfwWindow, FrameBufferResized);
    }
>>>>>>> 5a8aa30e34b62f79f538a7ccda97a18336491680
}

// Set the dimensions of the viewport
void DeviceGL::SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);
}

// Poll the events in the window event queue
void DeviceGL::PollEvents()
{
    glfwPollEvents();
}

// Clear the framebuffer with the specified color
void DeviceGL::Clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

// Callback called when the framebuffer changes size
void DeviceGL::FrameBufferResized(GLFWwindow* window, GLsizei width, GLsizei height)
{
<<<<<<< HEAD
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
=======
    if (m_instance)
    {
        // Adjust the viewport when the framebuffer is resized
        m_instance->SetViewport(0, 0, width, height);
    }
>>>>>>> 5a8aa30e34b62f79f538a7ccda97a18336491680
}
