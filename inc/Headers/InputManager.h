#pragma once

#include "glad.h"
#include "glfw3.h"
#include "camera.h"
#include "imgui.h"

class InputManager {
public:
    InputManager(Camera& camera, float& deltaTime, float& laserTimer, float& laserDuration, bool& vsyncEnabled);
    
    void processInput(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    static void setShowMenu(bool show) { showMenu = show; }
    static bool isShowMenu() { return showMenu; }
    
private:
    Camera& camera;
    float& deltaTime;
    float& laserTimer;
    float laserDuration;
    bool vKeyPressed = false;
    bool& vsyncEnabled;
    
    static bool showMenu;
    static float lastX;
    static float lastY;
    static bool firstMouse;
    static unsigned int SCR_WIDTH;
    static unsigned int SCR_HEIGHT;
}; 