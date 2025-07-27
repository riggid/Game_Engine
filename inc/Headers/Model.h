#ifndef MODEL_H
#define MODEL_H

#define GLM_ENABLE_EXPERIMENTAL

#include "glad.h" 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "shader_m.h"

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model 
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    Model() : gammaCorrection(false) {}
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path);
    }
    ~Model() {
        meshes.clear();
        textures_loaded.clear();
    }

    void Draw(Shader &shader) {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    vector<Model> sliceModel(float xThreshold);
    void sliceModelCylinder(const glm::vec3& cylinderAxisStart, const glm::vec3& cylinderAxisEnd, float cylinderRadius);

    enum {
        NUMDIM = 3,
        RIGHT = 0,
        LEFT = 1,
        MIDDLE = 2
    };

    bool HitBoundingBox(const glm::vec3& minB, const glm::vec3& maxB, const glm::vec3& origin, const glm::vec3& dir, glm::vec3& coord);
    glm::vec3 getBoundingBoxMin() const;
    glm::vec3 getBoundingBoxMax() const;

private:
    void loadModel(string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif
