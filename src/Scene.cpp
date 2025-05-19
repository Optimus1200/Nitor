#include <iostream>
#include <stack>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Scene.h"

namespace ntr
{
    //-------------------------------------------------------------------------------------------------
    // CONSTRUCTORS
    //-------------------------------------------------------------------------------------------------

    Scene::Scene()
        : selectedCamera{ primaryCamera }
        , M_DEFAULT_TEXTURE_ALBEDO      { 16, 16, {127.0f, 127.0f, 127.0f, 255.0f} }
        , M_DEFAULT_TEXTURE_NORMAL      { 16, 16, {127.0f, 127.0f, 255.0f, 255.0f} }
        , M_DEFAULT_TEXTURE_ROUGHNESS   { 16, 16, {127.0f, 127.0f, 127.0f, 255.0f} }
        , M_DEFAULT_TEXTURE_METALLIC    { 16, 16, {127.0f, 127.0f, 127.0f, 255.0f} }
        , M_DEFAULT_TEXTURE_OCCLUSION   { 16, 16, {255.0f, 255.0f, 255.0f, 255.0f} }
        , M_DEFAULT_MATERIAL { new Material
            {
                M_DEFAULT_TEXTURE_ALBEDO.handle(),
                M_DEFAULT_TEXTURE_NORMAL.handle(),
                M_DEFAULT_TEXTURE_ROUGHNESS.handle(),
                M_DEFAULT_TEXTURE_METALLIC.handle(),
                M_DEFAULT_TEXTURE_OCCLUSION.handle()
            }
        }
    {

    }

    Scene::~Scene()
    {
        for (auto& [id, model] : mMapModels)
        {
            if (model != Model::EMPTY)
            {
                delete model;
            }
           
        }

        for (auto& [id, material] : mMapMaterials)
        {
            if (material != Material::EMPTY)
            {
                delete material;
            }
        }

        for (auto& [id, mesh] : mMapMeshes)
        {
            if (mesh != Mesh::EMPTY)
            {
                delete mesh;
            }
        }
    }

    Mesh* Scene::findMesh(const std::string& id)
    {
        auto itr = mMapMeshes.find(id);

        if (itr == mMapMeshes.end())
        {
            return Mesh::EMPTY;
        }

        return itr->second;
    }

    void Scene::replaceMeshID(const std::string& id, const std::string& new_id)
    {
        if (id == new_id)
        {
            return;
        }

        auto itr = mMapMeshes.find(id);

        // Return if mesh not found or new id already exists
        if (itr == mMapMeshes.end() || mMapMeshes.find(new_id) != mMapMeshes.end())
        {
            return;
        }

        Mesh* mesh = itr->second;

        mMapMeshes[new_id] = mesh;
        mRmapMeshes[mesh->vao()] = new_id;

        mMapMeshes.erase(id);
    }

    void Scene::removeMesh(const std::string& id)
    {
        const Mesh* meshToRemove = findMesh(id);

        if (meshToRemove == Mesh::EMPTY)
        {
            return;
        }

        for (auto& [id, model] : mMapModels)
        {
            model->replaceMeshes(meshToRemove, Mesh::EMPTY);
        }
            
        mRmapMeshes.erase(meshToRemove->vao());
        mMapMeshes.erase(id);
    }

    Model* Scene::loadModel(const std::string& id, const std::filesystem::path& filepath)
    {
        Assimp::Importer importer;
        const aiScene* SCENE = importer.ReadFile(filepath.string().c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_ImproveCacheLocality);

        if (!SCENE || SCENE->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !SCENE->mRootNode)
        {
            std::cerr << "ERROR: could not import file: " << filepath << std::endl;
            return Model::EMPTY;
        }

        if (mMapModels.find(id) != mMapModels.end())
        {
            std::cerr << "ERROR: model with ID \'" << id << "\' exists." << std::endl;
            return Model::EMPTY;
        }

        std::unordered_set<std::filesystem::path> fileCache;

        Model* model = processModel(filepath, SCENE->mRootNode, SCENE);

        mMapModels.emplace(id, model);
        mRmapModels.emplace(model, id);

        return model;
    }

    Model* Scene::findModel(const std::string& id)
    {
        auto itr = mMapModels.find(id);

        if (itr == mMapModels.end())
        {
            return Model::EMPTY;
        }

        return itr->second;
    }

    void Scene::replaceModelID(const std::string& id, const std::string& new_id)
    {
        if (id == new_id)
        {
            return;
        }

        auto itr = mMapModels.find(id);

        // Return if model not found or new id already exists
        if (itr == mMapModels.end() || mMapModels.find(new_id) != mMapModels.end())
        {
            return;
        }

        Model* model = itr->second;

        mMapModels[new_id] = model;
        mRmapModels[model] = new_id;

        mMapModels.erase(id);
    }

    void Scene::removeModel(const std::string& id)
    {
        const Model* modelToRemove = findModel(id);

        if (modelToRemove == Model::EMPTY)
        {
            return;
        }

        auto view = registry.view<ConstPointer<Model>>();

        for (auto [entity, model] : view.each())
        {
            if (model == modelToRemove)
            {
                model = Model::EMPTY;
            }
        }

        mRmapModels.erase(modelToRemove);
        mMapModels.erase(id);
    }

    TextureHandle Scene::loadTexture(const std::string& id, const std::filesystem::path& filepath)
    {
        if (mMapTextures.find(id) != mMapTextures.end())
        {
            return 0;
        }

        const auto& [itr, inserted] =  mMapTextures.emplace(id, filepath);

        TextureHandle texture = itr->second.handle();

        mRmapTextures.emplace(texture, id);

        return itr->second.handle();
    }

    TextureHandle Scene::findTexture(const std::string& id) const
    {
        auto itr = mMapTextures.find(id);

        if (itr == mMapTextures.end())
        {
            return 0;
        }

        return itr->second.handle();
    }

    void Scene::replaceTextureID(const std::string& id, const std::string& new_id)
    {
        if (id == new_id)
        {
            return;
        }

        auto itr = mMapTextures.find(id);

        // Return if texture not found or new id already exists
        if (itr == mMapTextures.end() || mMapTextures.find(new_id) != mMapTextures.end())
        {
            return;
        }

        TextureHandle handle = itr->second.handle();

        mMapTextures[new_id] = std::move(itr->second);
        mRmapTextures[handle] = new_id;

        mMapTextures.erase(id);
    }

    void Scene::removeTexture(const std::string& id)
    {
        TextureHandle textureToRemove = findTexture(id);

        if (textureToRemove == Texture::EMPTY)
        {
            return;
        }

        for (auto& [id, material] : mMapMaterials)
        {
            if (material->albedo == textureToRemove)
            {
                material->albedo = Texture::EMPTY;
            }
            if (material->normal == textureToRemove)
            {
                material->normal = Texture::EMPTY;
            }
            if (material->roughness == textureToRemove)
            {
                material->roughness = Texture::EMPTY;
            }
            if (material->metallic == textureToRemove)
            {
                material->metallic = Texture::EMPTY;
            }
            if (material->occlusion == textureToRemove)
            {
                material->occlusion = Texture::EMPTY;
            }
        }

        mRmapTextures.erase(textureToRemove);
        mMapTextures.erase(id);
    }

    Material* Scene::addMaterial(const std::string& id, const Material& material)
    {
        if (mMapMaterials.find(id) != mMapMaterials.end())
        {
            return Material::EMPTY;
        }

        Material* newMaterial = new Material(material);

        mMapMaterials.emplace(id, newMaterial);
        mRmapMaterials.emplace(newMaterial, id);

        if (newMaterial->albedo == 0)
        {
            newMaterial->albedo = M_DEFAULT_TEXTURE_ALBEDO.handle();
        }
        if (newMaterial->normal == 0)
        {
            newMaterial->normal = M_DEFAULT_TEXTURE_NORMAL.handle();
        }
        if (newMaterial->roughness == 0)
        {
            newMaterial->roughness = M_DEFAULT_TEXTURE_ROUGHNESS.handle();
        }
        if (newMaterial->metallic == 0)
        {
            newMaterial->metallic = M_DEFAULT_TEXTURE_METALLIC.handle();
        }
        if (newMaterial->occlusion == 0)
        {
            newMaterial->occlusion = M_DEFAULT_TEXTURE_OCCLUSION.handle();
        }

        return newMaterial;
    }

    Material* Scene::findMaterial(const std::string& id)
    {
        const auto itr = mMapMaterials.find(id);

        if (itr == mMapMaterials.end())
        {
            return Material::EMPTY;
        }

        return itr->second;
    }

    void Scene::replaceMaterialID(const std::string& id, const std::string& new_id)
    {
        if (id == new_id)
        {
            return;
        }

        auto itr = mMapMaterials.find(id);

        // Return if material not found or new id already exists
        if (itr == mMapMaterials.end() || mMapMaterials.find(new_id) != mMapMaterials.end())
        {
            return;
        }

        Material* material = itr->second;

        mMapMaterials[new_id] = material;
        mRmapMaterials[material] = new_id;

        mMapMaterials.erase(id);
    }

    void Scene::removeMaterial(const std::string id)
    {
        const Material* materialToRemove = findMaterial(id);

        if (materialToRemove == Material::EMPTY)
        {
            return;
        }

        for (auto& [id, model] : mMapModels)
        {
            model->replaceMaterials(materialToRemove, M_DEFAULT_MATERIAL);
        }

        mRmapMaterials.erase(materialToRemove);
        mMapMaterials.erase(id);
    }

    std::string Scene::findTextureID(TextureHandle texture) const
    {
        auto itr = mRmapTextures.find(texture);

        if (itr == mRmapTextures.end())
        {
            return "None";
        }

        return itr->second;
    }

    std::string Scene::findMeshID(const Mesh* mesh) const
    {
        auto itr = mRmapMeshes.find(mesh->vao());

        if (itr == mRmapMeshes.end())
        {
            return "None";
        }

        return itr->second;
    }

    std::string Scene::findMeshID(const MeshInstance& mesh) const
    {
        auto itr = mRmapMeshes.find(mesh.vao);

        if (itr == mRmapMeshes.end())
        {
            return "None";
        }

        return itr->second;
    }

    std::string Scene::findModelID(const Model* model) const
    {
        auto itr = mRmapModels.find(model);

        if (itr == mRmapModels.end())
        {
            return "None";
        }

        return itr->second;
    }

    std::string Scene::findMaterialID(const Material* material) const
    {
        auto itr = mRmapMaterials.find(material);

        if (itr == mRmapMaterials.end())
        {
            return "None";
        }

        return itr->second;
    }

    const std::map<std::string, Model*>& Scene::getModelMap() const
    {
        return mMapModels;
    }

    const std::map<std::string, Material*>& Scene::getMaterialMap() const
    {
        return mMapMaterials;
    }

    const std::map<std::string, Texture>& Scene::getTextureMap() const
    {
        return mMapTextures;
    }

    const std::map<std::string, Mesh*>& Scene::getMeshMap() const
    {
        return mMapMeshes;
    }

    const Material* Scene::getDefaultMaterial() const
    {
        return M_DEFAULT_MATERIAL;
    }

    //-------------------------------------------------------------------------------------------------
    // PRIVATE MEMBER FUNCTIONS
    //-------------------------------------------------------------------------------------------------

    void Scene::processCameras(const aiScene* scene)
    {
        for (size_t i = 0; i < scene->mNumCameras; ++i)
        {
            aiCamera& ai_camera = *scene->mCameras[i];
            
            Camera ntrCamera;

            ntrCamera.position = {
                ai_camera.mPosition.x,
                ai_camera.mPosition.y,
                ai_camera.mPosition.z
            };

            ntrCamera.viewport = {
                0,
                0,
                ai_camera.mOrthographicWidth,
                ai_camera.mOrthographicWidth / ai_camera.mAspect
            };

            ntrCamera.fovY         = ai_camera.mHorizontalFOV;
            ntrCamera.zNear        = ai_camera.mClipPlaneNear;
            ntrCamera.zFar         = ai_camera.mClipPlaneFar;
            ntrCamera.ortho        = ai_camera.mOrthographicWidth > 0.0f;

            entt::entity entity_camera = registry.create();

            registry.emplace<Camera>(entity_camera, std::move(ntrCamera));
        }
    }

    void Scene::processLights(const aiScene* scene)
    {
        for (size_t i = 0; i < scene->mNumLights; ++i)
        {
            aiLight* ai_light = scene->mLights[i];
            
            switch (ai_light->mType)
            {
            case aiLightSource_POINT:
                
                PointLight ntrPointLight;

                ntrPointLight.position = {
                    ai_light->mPosition.x,
                    ai_light->mPosition.y,
                    ai_light->mPosition.z
                };

                ntrPointLight.color = {
                    ai_light->mColorDiffuse.r,
                    ai_light->mColorDiffuse.g,
                    ai_light->mColorDiffuse.b
                };
                
                break;
            }
        }
    }

    Model* Scene::processModel(const std::filesystem::path& modelPath, const aiNode* ai_node, const aiScene* ai_scene)
    {
        Model* model = new Model();

        AssetCache assetCache;

        std::stack<const aiNode*> nodeStack;
        nodeStack.push(ai_node);

        while (!nodeStack.empty())
        {
            const aiNode* currentNode = nodeStack.top();
            nodeStack.pop();

            // Process child nodes

            Transform meshTransform = processMeshTransform(currentNode, ai_scene);
            
            size_t numMeshes = currentNode->mNumMeshes;

            for (size_t i = 0; i < numMeshes; ++i)
            {
                aiMesh* ai_mesh = ai_scene->mMeshes[currentNode->mMeshes[i]];

                Mesh*       mesh            = processMesh(ai_mesh, ai_scene, assetCache);
                Material*   meshMaterial    = processMeshMaterial(modelPath, ai_mesh, currentNode, ai_scene, assetCache);
                std::string meshID          = findMeshID(mesh);

                // Handle duplicate mesh id in model

                while (model->meshes.find(meshID) != model->meshes.end())
                {
                    meshID += "+";
                }

                // Create model

                model->meshes.try_emplace(meshID, mesh, meshMaterial, meshTransform).first->second;
            }

            // push children in nodeStack in reverse to maintain original processing order

            int numChildMeshes = (int)currentNode->mNumChildren;

            for (int i = numChildMeshes - 1; i >= 0; --i)
            {
                nodeStack.push(currentNode->mChildren[i]);
            }
        }

        return model;
    }

    // Creates the ntr::Mesh from the aiMesh, and returns a pointer to the Mesh
    Mesh* Scene::processMesh(const aiMesh* ai_mesh, const aiScene* ai_scene, AssetCache& assetCache)
    {
        // Reuse mesh if it already exists

        auto itr = assetCache.meshes.find(ai_mesh);

        if (itr != assetCache.meshes.end())
        {
            return itr->second;
        }

        // New mesh: handle duplicate mesh name in map

        std::string meshName = ai_mesh->mName.C_Str();

        std::string idToUse = meshName;
        
        while (mMapMeshes.find(idToUse) != mMapMeshes.end())
        {
            idToUse += "+";
        }

        // Create and store mesh in map

        auto [vertices, indices] = processMeshVerticesAndIndices(ai_mesh);

        Mesh* mesh = new Mesh(std::move(vertices), std::move(indices));

        mMapMeshes.emplace(idToUse, mesh);
        mRmapMeshes.emplace(mesh->vao(), idToUse);

        // Record mesh in cache for reuse
        assetCache.meshes.try_emplace(ai_mesh, mesh);

        return mesh;
    }

    std::pair<std::vector<Vertex>, std::vector<GLuint>> Scene::processMeshVerticesAndIndices(const aiMesh* ai_mesh)
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        const size_t NUM_VERTICES = ai_mesh->mNumVertices;

        vertices.reserve(NUM_VERTICES);

        // Process vertices
        for (size_t i = 0; i < NUM_VERTICES; ++i)
        {
            Vertex vertex{};

            vertex.position = {
                ai_mesh->mVertices[i].x,
                ai_mesh->mVertices[i].y,
                ai_mesh->mVertices[i].z
            };

            if (ai_mesh->HasNormals())
            {
                vertex.normal = {
                    ai_mesh->mNormals[i].x,
                    ai_mesh->mNormals[i].y,
                    ai_mesh->mNormals[i].z
                };
            }

            if (ai_mesh->mTextureCoords[0])
            {
                vertex.texCoords = {
                    ai_mesh->mTextureCoords[0][i].x,
                    ai_mesh->mTextureCoords[0][i].y
                };

                vertex.tangent = {
                    ai_mesh->mTangents[i].x,
                    ai_mesh->mTangents[i].y,
                    ai_mesh->mTangents[i].z
                };

                vertex.biTangent = {
                    ai_mesh->mBitangents[i].x,
                    ai_mesh->mBitangents[i].y,
                    ai_mesh->mBitangents[i].z
                };
            }
            else
            {
                vertex.texCoords = { 0.0f, 0.0f };
            }

            vertices.emplace_back(std::move(vertex));
        }

        // Process each mesh face

        const size_t NUM_FACES = ai_mesh->mNumFaces;

        for (size_t i = 0; i < NUM_FACES; ++i)
        {
            const aiFace& FACE = ai_mesh->mFaces[i];

            if (FACE.mNumIndices > 0)
            {
                indices.reserve(indices.size() + FACE.mNumIndices);
            }

            for (size_t j = 0; j < FACE.mNumIndices; ++j)
            {
                indices.emplace_back(FACE.mIndices[j]);
            }
        }

        return { vertices, indices };
    }

    Material* Scene::processMeshMaterial
    (
        const std::filesystem::path& modelPath, 
        const aiMesh* ai_mesh, 
        const aiNode* ai_node,
        const aiScene* ai_scene, 
        AssetCache& assetCache
    )
    {
        aiMaterial* ai_mesh_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

        // Reuse material if it already exists

        std::string materialName = ai_mesh_material->GetName().C_Str();

        auto itr = assetCache.materials.find(materialName);

        if (itr != assetCache.materials.end())
        {
            return itr->second;
        }

        // New material: handle duplicate material name, then add material

        std::string materialIDToUse = std::string(ai_node->mName.C_Str()) + " - " + ai_mesh->mName.C_Str() + " - " + materialName;

        while (findMaterial(materialIDToUse) != Material::EMPTY)
        {
            materialIDToUse += "+";
        }

        // process material and textures, then add them to mesh instance

        TextureHandle mapAlbedo = processMaterialTexture(modelPath, ai_mesh_material, aiTextureType_DIFFUSE, 0, assetCache);
        TextureHandle mapNormal = processMaterialTexture(modelPath, ai_mesh_material, aiTextureType_NORMALS, 0, assetCache);
        TextureHandle mapRoughness = processMaterialTexture(modelPath, ai_mesh_material, aiTextureType_DIFFUSE_ROUGHNESS, 0, assetCache);
        TextureHandle mapMetallic = processMaterialTexture(modelPath, ai_mesh_material, aiTextureType_METALNESS, 0, assetCache);
        TextureHandle mapAO = processMaterialTexture(modelPath, ai_mesh_material, aiTextureType_AMBIENT_OCCLUSION, 0, assetCache);

        Material* ntr_material;
        bool sameAsDefaultMaterial = true;

        if (mapAlbedo == Texture::EMPTY)
        {
            mapAlbedo = M_DEFAULT_TEXTURE_ALBEDO.handle();
        }
        else
        {
            sameAsDefaultMaterial = false;
        }

        if (mapNormal == Texture::EMPTY)
        {
            mapNormal = M_DEFAULT_TEXTURE_NORMAL.handle();
        }
        else
        {
            sameAsDefaultMaterial = false;
        }

        if (mapRoughness == Texture::EMPTY)
        {
            mapRoughness = M_DEFAULT_TEXTURE_ROUGHNESS.handle();
        }
        else
        {
            sameAsDefaultMaterial = false;
        }

        if (mapMetallic == Texture::EMPTY)
        {
            mapMetallic = M_DEFAULT_TEXTURE_METALLIC.handle();
        }
        else
        {
            sameAsDefaultMaterial = false;
        }

        if (mapAO == Texture::EMPTY)
        {
            mapAO = M_DEFAULT_TEXTURE_OCCLUSION.handle();
        }
        else
        {
            sameAsDefaultMaterial = false;
        }

        if (sameAsDefaultMaterial)
        {
            ntr_material = M_DEFAULT_MATERIAL;
        }
        else
        {
            ntr_material = addMaterial(materialIDToUse, {});
            ntr_material->albedo = mapAlbedo;
            ntr_material->normal = mapNormal;
            ntr_material->roughness = mapRoughness;
            ntr_material->metallic = mapMetallic;
            ntr_material->occlusion = mapAO;
        }

        // Record material in cache for reuse
        assetCache.materials.emplace(materialName, ntr_material);

        return ntr_material;
    }

    Transform Scene::processMeshTransform(const aiNode* ai_node, const aiScene* ai_scene)
    {
        aiVector3D ai_pos, ai_scl, ai_rot;
        ai_node->mTransformation.Decompose(ai_scl, ai_rot, ai_pos);

        Transform ntr_transform;
        ntr_transform.position.x = ai_pos.x;
        ntr_transform.position.y = ai_pos.y;
        ntr_transform.position.z = ai_pos.z;
        ntr_transform.rotation.x = glm::degrees(ai_rot.x);
        ntr_transform.rotation.y = glm::degrees(ai_rot.y);
        ntr_transform.rotation.z = glm::degrees(ai_rot.z);
        ntr_transform.scale.x = ai_scl.x;
        ntr_transform.scale.y = ai_scl.y;
        ntr_transform.scale.z = ai_scl.z;

        return ntr_transform;
    }

    TextureHandle Scene::processMaterialTexture
    (
        const std::filesystem::path& modelPath, 
        const aiMaterial* ai_material, 
        aiTextureType ai_texture_type, 
        unsigned int index, 
        AssetCache& assetCache
    )
    {
        aiString ai_texture_path;
        
        if (ai_material->GetTexture(ai_texture_type, index, &ai_texture_path) != AI_SUCCESS)
        {
            return Texture::EMPTY;
        }

        std::filesystem::path texturePath = modelPath.parent_path().append(ai_texture_path.C_Str());

        // Reuse texture if already loaded

        auto itr = assetCache.textures.find(texturePath);

        if (itr != assetCache.textures.end())
        {
            return itr->second;
        }

        // Load texture if not loaded, handle duplicate id, record in cache for reuse

        std::string idToUse = texturePath.filename().string();

        while (findTexture(idToUse) != Texture::EMPTY)
        {
            idToUse += "+";
        }

        TextureHandle handle = loadTexture(idToUse, texturePath);

        // Record texture in cache for reuse
        assetCache.textures.emplace(texturePath, handle);

        return handle;
    }

    Transform Scene::toTransform(const aiMatrix4x4& matrix)
    {
        Transform transform;
        glm::mat4 transformMatrix(1.0);

        for (uint32_t row = 0; row < 3; ++row)
        {
            for (uint32_t col = 0; col < 3; ++col)
            {
                transformMatrix[row][col] = matrix[row][col];
            }
        }

        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat rotationQuat;

        glm::decompose(transformMatrix, transform.scale, rotationQuat, transform.position, skew, perspective);

        transform.rotation = glm::eulerAngles(rotationQuat);

        return transform;
    }
} // namespace ntr

