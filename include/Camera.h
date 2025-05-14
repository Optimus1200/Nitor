#ifndef NTR_VIEW_H
#define NTR_VIEW_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "Structs.h"

namespace ntr
{
	struct Camera
	{
		static constexpr glm::vec3 WORLD_RIGHT	= { 1.0f, 0.0f,  0.0f };
		static constexpr glm::vec3 WORLD_UP		= { 0.0f, 1.0f,  0.0f };
		static constexpr glm::vec3 WORLD_FRONT	= { 0.0f, 0.0f, -1.0f };

		glm::vec3 position	= { 0.0f,   0.0f, 0.0f };
		glm::vec3 rotation	= { 0.0f, -90.0f, 0.0f };

		Rect viewport		= { 0.0f, 0.0f, 800.0f, 600.0f };

		float fovY			=   60.0f; // degrees
		float zNear			=    0.1f;
		float zFar			= 1000.0f;
		bool  ortho			= false;

		double sensitivity	= 0.2;

		glm::vec3 front() const;
		glm::vec3 right() const;
		glm::vec3 up() const;

		glm::mat4 view() const;
		glm::mat4 projection() const;
	};
} // namespace ntr

#endif