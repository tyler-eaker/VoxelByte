#include "globals.h"

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height)
{
    InitializeGLFW();

    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);

    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetCursorPosCallback(m_window, mouseCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSwapInterval(0);

    InitializeGLAD();

    // Initialize mouse starting position
    lastX = m_width / 2.0f;
    lastY = m_height / 2.0f;

    VB::inst().GetLogger()->Print("Window obj constructed");
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        VB::inst().GetLogger()->Print("Window destroyed");
    }
    glfwTerminate();
}

void Window::InitializeGLFW() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void Window::InitializeGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Window::mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (self->firstMouse) {
            self->lastX = xpos;
            self->lastY = ypos;
            self->firstMouse = false;
        }

        float xoffset = xpos - self->lastX;
        float yoffset = self->lastY - ypos;

        self->lastX = xpos;
        self->lastY = ypos;

        VB::inst().GetCamera()->ProcessMouseMovement(xoffset, yoffset);
    }
    else {
        self->firstMouse = true;
    }
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        VB::inst().GetCamera()->ProcessMouseScroll(static_cast<float>(yoffset));
    }
}