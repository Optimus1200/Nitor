#ifndef NTR_SCENE_H
#define NTR_SCENE_H

#include <filesystem>
#include <string>
#include <map>
#include <unordered_set>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>

#include <entt/entt.hpp>

#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "Transform.h"

namespace ntr
{
	struct Scene
	{
	public:

		Camera	primaryCamera;
		Camera&	selectedCamera;

		DirectionalLight directionalLight;

		entt::registry registry;
		
		Scene();

		Scene(const Scene& scene)				= delete;
		Scene& operator=(const Scene& scene)	= delete;

		Scene(Scene&& scene)			= delete;
		Scene& operator=(Scene&& scene) = delete;

		~Scene();
				
		// Returns Mesh::EMPTY if no Mesh found.
		Mesh* findMesh(const std::string& id);

		void replaceMeshID(const std::string& id, const std::string& new_id);
		
		// Removes the Mesh and replaces all MeshInstances that use this Mesh with Mesh::EMPTY.
		void removeMesh(const std::string& id);

		Model* loadModel(const std::string& id, const std::filesystem::path& modelPath);
		
		// Returns Model::EMPTY if no Model found.
		Model* findModel(const std::string& id);

		void replaceModelID(const std::string& id, const std::string& new_id);
		
		// Removes the Model and replaces all entities with ConstPointer<Model> component that use this Model with Model::EMPTY.
		void removeModel(const std::string& id);

		// Returns Texture::EMPTY if unsuccessful.
		TextureHandle loadTexture(const std::string& id, const std::filesystem::path& filepath);
		
		// Returns Texture::EMPTY if no Texture found.
		TextureHandle findTexture(const std::string& id) const;

		void replaceTextureID(const std::string& id, const std::string& new_id);
		
		// Removes the texture and replaces all Material textures that use this Texture with Texture::EMPTY.
		void removeTexture(const std::string& id);

		// Returns Material::EMPTY if unsuccessful.
		Material* addMaterial(const std::string& id, const Material& material);
		
		// Returns Material::EMPTY if no Material found.
		Material* findMaterial(const std::string& id);

		void replaceMaterialID(const std::string& id, const std::string& new_id);
		
		// Removes the Material and replaces all MeshInstance.material's that use this Material with Material::EMPTY.
		void removeMaterial(const std::string id);

		// extra helpers - helpful if you have an asset, but don't know the string id

		// Returns "None" if no Texture found.
		std::string findTextureID(TextureHandle texture) const;

		// Returns "None" if no Mesh found.
		std::string findMeshID(const Mesh* mesh) const;
		
		// Returns "None" if no Mesh found.
		std::string findMeshID(const MeshInstance& mesh) const;
		
		// Returns "None" if no Model found.
		std::string findModelID(const Model* model) const;
		
		// Returns "None" if no Material found.
		std::string findMaterialID(const Material* material) const;

		const std::map<std::string, Model*>&		getModelMap() const;
		const std::map<std::string, Material*>&		getMaterialMap() const;
		const std::map<std::string, Texture>&		getTextureMap() const;
		const std::map<std::string, Mesh*>&			getMeshMap() const;

		const Material* getDefaultMaterial() const;

	private:

		const Texture					M_DEFAULT_TEXTURE_ALBEDO;
		const Texture					M_DEFAULT_TEXTURE_NORMAL;
		const Texture					M_DEFAULT_TEXTURE_ROUGHNESS;
		const Texture					M_DEFAULT_TEXTURE_METALLIC;
		const Texture					M_DEFAULT_TEXTURE_OCCLUSION;
		const ScopedPointer<Material>	M_DEFAULT_MATERIAL;

		std::map<std::string, Texture>				mMapTextures;
		std::map<std::string, Mesh*>				mMapMeshes;
		std::map<std::string, Model*>				mMapModels;
		std::map<std::string, Material*>			mMapMaterials;

		std::map<TextureHandle,		std::string>		mRmapTextures;
		std::map<GLuint,			std::string>		mRmapMeshes;
		std::map<const Model*,		std::string>		mRmapModels;
		std::map<const Material*,	std::string>		mRmapMaterials;

		void			processCameras(const aiScene* scene);
		void			processLights(const aiScene* scene);
		void			processModelRecursive(const std::filesystem::path& modelPath, Model* ntr_model, const std::string& id, const aiNode* ai_node, const aiScene* ai_scene);
		Mesh*			processMesh(const aiMesh* ai_mesh, const std::string& modelName, const aiScene* ai_scene);
		TextureHandle	processMaterialTexture(const std::filesystem::path& modelPath, const aiMaterial* ai_material, aiTextureType ai_texture_type, unsigned int index = 0);
		Transform		toTransform(const aiMatrix4x4& matrix);
	};
	
} // namespace ntr

#endif