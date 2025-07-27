#include "Drawer.h"

Drawer::Drawer(Model& model, Shader& shader) : model(model), shader(shader) {
    position = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);
    rotation = glm::vec3(0.0f);
    rotationMode = RotationMode::NONE;
    target = glm::vec3(0.0f);
}

glm::mat4 Drawer::calculateModelMatrix() const {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    
    // Apply initial rotation first
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Calculate rotation based on target
    if (rotationMode != RotationMode::NONE) {
        glm::vec3 direction = glm::normalize(target - position);
        
        switch(rotationMode) {
            case RotationMode::Y_ONLY: {
                float yaw = glm::degrees(atan2(direction.x, direction.z));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            }
            case RotationMode::ALL: {
                float yaw = glm::degrees(atan2(direction.x, direction.z));
                float pitch = glm::degrees(-atan2(direction.y, glm::length(glm::vec2(direction.x, direction.z))));
                
                modelMatrix = glm::rotate(modelMatrix, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            }
            default:
                break;
        }
    }
    
    modelMatrix = glm::scale(modelMatrix, scale);
    return modelMatrix;
}

void Drawer::draw() {
    shader.use();
    shader.setMat4("model", calculateModelMatrix());
    model.Draw(shader);
}

void Drawer::setPosition(const glm::vec3& pos) {
    position = pos;
}

void Drawer::setScale(const glm::vec3& s) {
    scale = s;
}

void Drawer::setRotation(const glm::vec3& rot) {
    rotation = rot;
}

void Drawer::setRotationMode(RotationMode mode) {
    rotationMode = mode;
}

void Drawer::setTarget(const glm::vec3& newTarget) {
    target = newTarget;
}

Model& Drawer::getModel() {
    return model;
}

void Drawer::setModel(Model& newModel) {
    model = newModel;
}

void Drawer::setupBoundingBox(Shader& lineShader) {
    glm::vec3 minBounds = model.getBoundingBoxMin();
    glm::vec3 maxBounds = model.getBoundingBoxMax();
    
    std::vector<glm::vec3> vertices = {
        glm::vec3(minBounds.x, minBounds.y, minBounds.z),
        glm::vec3(maxBounds.x, minBounds.y, minBounds.z),
        glm::vec3(maxBounds.x, maxBounds.y, minBounds.z),
        glm::vec3(minBounds.x, maxBounds.y, minBounds.z),
        glm::vec3(minBounds.x, minBounds.y, maxBounds.z),
        glm::vec3(maxBounds.x, minBounds.y, maxBounds.z),
        glm::vec3(maxBounds.x, maxBounds.y, maxBounds.z),
        glm::vec3(minBounds.x, maxBounds.y, maxBounds.z)
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 1, 2, 2, 3, 3, 0,  // Front face
        4, 5, 5, 6, 6, 7, 7, 4,  // Back face
        0, 4, 1, 5, 2, 6, 3, 7   // Connecting lines
    };
    
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    lineShader.use();
    lineShader.setMat4("model", calculateModelMatrix());
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
} 