#include <ituGL/core/DeviceGL.h>

#include <ituGL/application/Window.h>
#include <GLFW/glfw3.h>
#include <cassert>

DeviceGL* DeviceGL::m_instance = nullptr;

DeviceGL::DeviceGL() : m_contextLoaded(false)
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

DeviceGL::~DeviceGL()
{
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
}

// Set the window that OpenGL will use for rendering
void DeviceGL::SetCurrentWindow(Window& window)
{
	GLFWwindow* glfwWindow = window.GetInternalWindow();
	glfwMakeContextCurrent(glfwWindow);
	glfwSetFramebufferSizeCallback(glfwWindow, FrameBufferResized);
	
	m_contextLoaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

// Set the dimensions of the viewport
void DeviceGL::SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
}

// Poll the events in the window event queue
void DeviceGL::PollEvents()
{
}

// Clear the framebuffer with the specified color
void DeviceGL::Clear(float r, float g, float b, float a)
{
}

// Callback called when the framebuffer changes size
void DeviceGL::FrameBufferResized(GLFWwindow* window, GLsizei width, GLsizei height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
