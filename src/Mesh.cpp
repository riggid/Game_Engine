#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::Draw(Shader &shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for(unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        string number;
        string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);
        else if(name == "texture_normal")
            number = std::to_string(normalNr++);
        else if(name == "texture_height")
            number = std::to_string(heightNr++);

        glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

vector<Mesh> Mesh::sliceMesh(const Mesh& mesh, float xThreshold) {
    vector<Vertex> leftVertices, rightVertices;
    vector<unsigned int> leftIndices, rightIndices;
    
    // First, separate vertices
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        if (mesh.vertices[i].Position.x <= xThreshold) {
            leftVertices.push_back(mesh.vertices[i]);
        } else {
            rightVertices.push_back(mesh.vertices[i]);
        }
    }

    // Create new indices for separated meshes
    unordered_map<unsigned int, unsigned int> leftIndexMap, rightIndexMap;
    unsigned int leftCurrentIndex = 0, rightCurrentIndex = 0;

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        bool isLeft = false, isRight = false;
        for (int j = 0; j < 3; j++) {
            if (mesh.vertices[mesh.indices[i + j]].Position.x <= xThreshold) {
                isLeft = true;
            } else {
                isRight = true;
            }
        }

        if (isLeft && !isRight) {
            // Triangle belongs to left mesh
            for (int j = 0; j < 3; j++) {
                unsigned int oldIndex = mesh.indices[i + j];
                if (leftIndexMap.find(oldIndex) == leftIndexMap.end()) {
                    leftIndexMap[oldIndex] = leftCurrentIndex++;
                }
                leftIndices.push_back(leftIndexMap[oldIndex]);
            }
        } else if (!isLeft && isRight) {
            // Triangle belongs to right mesh
            for (int j = 0; j < 3; j++) {
                unsigned int oldIndex = mesh.indices[i + j];
                if (rightIndexMap.find(oldIndex) == rightIndexMap.end()) {
                    rightIndexMap[oldIndex] = rightCurrentIndex++;
                }
                rightIndices.push_back(rightIndexMap[oldIndex]);
            }
        }
    }

    vector<Mesh> result;
    if (!leftVertices.empty() && !leftIndices.empty()) {
        result.push_back(Mesh(leftVertices, leftIndices, mesh.textures));
    }
    if (!rightVertices.empty() && !rightIndices.empty()) {
        result.push_back(Mesh(rightVertices, rightIndices, mesh.textures));
    }
    return result;
}

Mesh Mesh::sliceMeshCyl(const Mesh& mesh, const glm::vec3& cylinderAxisStart, const glm::vec3& cylinderAxisEnd, float cylinderRadius) {
    vector<Vertex> remainingVertices;
    vector<unsigned int> remainingIndices;
    
    // Helper function to check if a point is inside the cylinder
    auto isInsideCylinder = [](const glm::vec3& point, const glm::vec3& cylinderStart, const glm::vec3& cylinderEnd, float radius) {
        glm::vec3 cylinderDir = glm::normalize(cylinderEnd - cylinderStart);
        glm::vec3 pointVec = point - cylinderStart;
        
        // Project point onto cylinder axis
        float t = glm::dot(pointVec, cylinderDir);
        glm::vec3 projection = cylinderStart + t * cylinderDir;
        
        // Check if projection is within cylinder bounds
        float cylinderLength = glm::length(cylinderEnd - cylinderStart);
        if (t < 0 || t > cylinderLength)
            return false;
            
        // Check radial distance
        float distance = glm::length(point - projection);
        return distance <= radius;
    };

    // Create a map to store new indices for remaining vertices
    unordered_map<unsigned int, unsigned int> newIndexMap;
    unsigned int currentIndex = 0;

    // Process triangles
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        bool keepTriangle = true;
        for (int j = 0; j < 3; j++) {
            if (isInsideCylinder(mesh.vertices[mesh.indices[i + j]].Position, 
                               cylinderAxisStart, cylinderAxisEnd, cylinderRadius)) {
                keepTriangle = false;
                break;
            }
        }

        if (keepTriangle) {
            // Add vertices and update indices
            for (int j = 0; j < 3; j++) {
                unsigned int oldIndex = mesh.indices[i + j];
                if (newIndexMap.find(oldIndex) == newIndexMap.end()) {
                    newIndexMap[oldIndex] = currentIndex++;
                    remainingVertices.push_back(mesh.vertices[oldIndex]);
                }
                remainingIndices.push_back(newIndexMap[oldIndex]);
            }
        }
    }

    return Mesh(remainingVertices, remainingIndices, mesh.textures);
} 