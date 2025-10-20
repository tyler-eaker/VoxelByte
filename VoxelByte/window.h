#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

class Window
{
public:
    Window(int width = 1920, int height = 1080, const char* title = "VoxelByte");
    ~Window();

    GLFWwindow* GetGLFWwindow() const { return m_window; }

    bool ShouldClose() const { return glfwWindowShouldClose(m_window); }
    void SwapBuffers() { glfwSwapBuffers(m_window); }
    void PollEvents() { glfwPollEvents(); }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    // Input state
    float lastX;
    float lastY;
    bool firstMouse = true;

private:
    GLFWwindow* m_window = nullptr;
    int m_width, m_height;

    void InitializeGLFW();
    void InitializeGLAD();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif