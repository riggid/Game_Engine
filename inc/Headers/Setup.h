#pragma once

#define GLM_ENABLE_EXPERIMENTAL

// OpenGL and Window Management
#include <glad.h>
#include <glfw3.h>

// GLM Mathematics
// Note: Dependency errors (e.g., stddef.h, cmath) likely indicate a compiler/IntelliSense configuration issue,
// not an error in these include paths themselves.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/fwd.hpp>

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Project Headers
#include "Model.h"
#include "camera.h"
#include "shader_m.h"
#include "Drawer.h"
#include "InputManager.h"

// Standard Library
#include <iostream>
#include <vector>

class Setup {
public:
    static GLFWwindow* initializeWindow(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT, Camera& camera);
    static void initializeGLAD();
    static void initializeImGui(GLFWwindow* window);
    static void setupOpenGLState();
    static void cleanup();
}; 