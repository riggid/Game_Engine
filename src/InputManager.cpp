#include "InputManager.h"
#include <iostream>

// Initialize static members only
bool InputManager::showMenu = false;
float InputManager::lastX = 800.0f;
float InputManager::lastY = 450.0f;
bool InputManager::firstMouse = true;
unsigned int InputManager::SCR_WIDTH = 1600;
unsigned int InputManager::SCR_HEIGHT = 900;

InputManager::InputManager(Camera& cam, float& dt, float& lt, float& ld, bool& vsync)
    : camera(cam), deltaTime(dt), laserTimer(lt), laserDuration(ld), vsyncEnabled(vsync), vKeyPressed(false) {
}

void InputManager::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // VSync toggle with V key
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!vKeyPressed) {
            vsyncEnabled = !vsyncEnabled;
            glfwSwapInterval(vsyncEnabled ? 1 : 0);
            std::cout << "VSync: " << (vsyncEnabled ? "ON" : "OFF") << std::endl;
            vKeyPressed = true;
        }
    } else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        vKeyPressed = false;
    }

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // Laser activation with space
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        laserTimer = laserDuration;
    }

    // Menu toggle with M
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        static bool mKeyPressed = false;
        if (!mKeyPressed) {
            showMenu = !showMenu;
            if (showMenu)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mKeyPressed = true;
        }
    } else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        static bool mKeyPressed = false;
        mKeyPressed = false;
    }
}

void InputManager::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void InputManager::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!showMenu) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (camera) {
            camera->ProcessMouseMovement(xoffset, yoffset);
        }
    }
}

void InputManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!showMenu) {
        Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (camera) {
            camera->ProcessMouseScroll(static_cast<float>(yoffset));
        }
    }
}

void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        showMenu = !showMenu;
} 