#ifndef NTR_VERTEX_H
#define NTR_VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace ntr
{
	struct Vertex
	{
		static constexpr int INDEX_POSITION		= 0;
		static constexpr int INDEX_NORMAL		= 1;
		static constexpr int INDEX_TEXCOORDS	= 2;
		static constexpr int INDEX_TANGENT		= 3;
		static constexpr int INDEX_BITANGENT	= 4;
		static constexpr int INDEX_BONE_IDS		= 5;
		static constexpr int INDEX_BONE_WEIGHTS	= 6;

		static constexpr size_t MAX_BONE_INFLUENCE = 4;
		
		glm::vec3	position;
		glm::vec3	normal;
		glm::vec2	texCoords;
		glm::vec3	tangent;
		glm::vec3	biTangent;
		int			boneIDs[MAX_BONE_INFLUENCE];
		float		weights[MAX_BONE_INFLUENCE];
	};
} // namespace ntr

#endif