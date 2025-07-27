#ifndef MESH_H
#define MESH_H

#include <glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader_m.h"

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    void Draw(Shader &shader);
    vector<Mesh> sliceMesh(const Mesh& mesh, float xThreshold);
    Mesh sliceMeshCyl(const Mesh& mesh, const glm::vec3& cylinderAxisStart, const glm::vec3& cylinderAxisEnd, float cylinderRadius);

private:
    unsigned int VBO, EBO;
    void setupMesh();
};
#endif

