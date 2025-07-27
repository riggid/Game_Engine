#include "Model.h"

void Model::loadModel(string const &path) {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    // process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        
        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        // texture coordinates
        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);        
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
    
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    
    vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    
    vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++) {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

vector<Model> Model::sliceModel(float xThreshold) {
    vector<Model> resultModels;
    vector<Mesh> leftMeshes, rightMeshes;

    for (Mesh& mesh : meshes) {
        vector<Mesh> slicedMeshes = mesh.sliceMesh(mesh, xThreshold);
        if (!slicedMeshes.empty()) {
            leftMeshes.push_back(slicedMeshes[0]);
            if (slicedMeshes.size() > 1) {
                rightMeshes.push_back(slicedMeshes[1]);
            }
        }
    }

    if (!leftMeshes.empty()) {
        Model leftModel;
        leftModel.meshes = leftMeshes;
        leftModel.textures_loaded = textures_loaded;
        leftModel.directory = directory;
        leftModel.gammaCorrection = gammaCorrection;
        resultModels.push_back(leftModel);
    }
    if (!rightMeshes.empty()) {
        Model rightModel;
        rightModel.meshes = rightMeshes;
        rightModel.textures_loaded = textures_loaded;
        rightModel.directory = directory;
        rightModel.gammaCorrection = gammaCorrection;
        resultModels.push_back(rightModel);
    }

    return resultModels;
}

void Model::sliceModelCylinder(const glm::vec3& cylinderAxisStart, const glm::vec3& cylinderAxisEnd, float cylinderRadius) {
    std::vector<Mesh> slicedMeshes;
    slicedMeshes.reserve(meshes.size());

    for (Mesh& mesh : meshes) {
        Mesh slicedMesh = mesh.sliceMeshCyl(mesh, cylinderAxisStart, cylinderAxisEnd, cylinderRadius);
        if (!slicedMesh.vertices.empty() && !slicedMesh.indices.empty()) {
            slicedMeshes.push_back(std::move(slicedMesh));
        }
    }

    meshes = std::move(slicedMeshes);
}

bool Model::HitBoundingBox(const glm::vec3& minB, const glm::vec3& maxB, const glm::vec3& origin, const glm::vec3& dir, glm::vec3& coord) {
    bool inside = true;
    int quadrant[NUMDIM];
    int whichPlane;
    double maxT[NUMDIM];
    glm::vec3 candidatePlane(0.0f);

    for (int i = 0; i < NUMDIM; i++) {
        if (origin[i] < minB[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        } else if (origin[i] > maxB[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        } else {
            quadrant[i] = MIDDLE;
        }
    }

    if (inside) {
        coord = origin;
        return true;
    }

    for (int i = 0; i < NUMDIM; i++) {
        if (quadrant[i] != MIDDLE && dir[i] != 0.0) {
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        } else {
            maxT[i] = -1.0;
        }
    }

    whichPlane = 0;
    for (int i = 1; i < NUMDIM; i++) {
        if (maxT[whichPlane] < maxT[i]) {
            whichPlane = i;
        }
    }

    if (maxT[whichPlane] < 0.0) return false;
    for (int i = 0; i < NUMDIM; i++) {
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] * dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i]) {
                return false;
            }
        } else {
            coord[i] = candidatePlane[i];
        }
    }
    return true;
}

glm::vec3 Model::getBoundingBoxMin() const {
    glm::vec3 minBounds(std::numeric_limits<float>::max());
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            minBounds = glm::min(minBounds, vertex.Position);
        }
    }
    return minBounds;
}

glm::vec3 Model::getBoundingBoxMax() const {
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            maxBounds = glm::max(maxBounds, vertex.Position);
        }
    }
    return maxBounds;
} 