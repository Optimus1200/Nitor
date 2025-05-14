#include "Model.h"

namespace ntr
{
	const ScopedPointer<Model> Model::EMPTY = new Model();
	
	void Model::replaceMeshes(const Mesh* mesh, const Mesh* new_mesh)
	{
		for (auto& [id, modelMesh] : meshes)
		{
			if (modelMesh.vao == mesh->vao())
			{
				meshes[id].vao = new_mesh->vao();
				meshes[id].indexCount = new_mesh->indexCount();
			}
		}
	}
	
	void Model::replaceMaterials(const Material* material, const Material* new_material)
	{
		for (auto& [id, modelMesh] : meshes)
		{
			if (modelMesh.material == material)
			{
				meshes[id].material = new_material;
			}
		}
	}
}