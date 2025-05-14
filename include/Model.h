#ifndef NTR_MODEL_H
#define NTR_MODEL_H

#include <atomic>
#include <vector>
#include <map>

#include "Mesh.h"
#include "Pointer.h"

namespace ntr
{
	struct Model
	{
		static const ScopedPointer<Model> EMPTY;

		std::map<std::string, MeshInstance> meshes;

		// Replaces all Meshinstances in this Model that use mesh with new_mesh.
		void replaceMeshes(const Mesh* mesh, const Mesh* new_mesh);
		// Replaces all Materials in all MeshInstances in this Model that use material with new_material.
		void replaceMaterials(const Material* material, const Material* new_material);
	};
}

#endif
