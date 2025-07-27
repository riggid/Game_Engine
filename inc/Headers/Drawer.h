#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Model.h"
#include "shader_m.h"

enum class RotationMode {
    NONE,
    Y_ONLY,
    ALL
};

class Drawer {
public:
    Drawer(Model& model, Shader& shader);
    void draw();
    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec3& s);
    void setRotation(const glm::vec3& rot);
    void setRotationMode(RotationMode mode);
    void setTarget(const glm::vec3& newTarget);
    void setupBoundingBox(Shader& lineShader);
    Model& getModel();
    void setModel(Model& newModel);
    
    // Add getter methods
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getTarget() const { return target; }
    glm::mat4 calculateModelMatrix() const;
    
private:
    Model& model;
    Shader& shader;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    RotationMode rotationMode;
    glm::vec3 target;
};