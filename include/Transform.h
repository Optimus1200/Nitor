#ifndef NTR_TRANSFORM_H
#define NTR_TRANSFORM_H

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

namespace ntr
{
	struct Transform
	{
		glm::vec3 position	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale		= { 1.0f, 1.0f, 1.0f };

		glm::mat4 matrix() const;
		glm::mat4 matrixPosition() const;
		glm::mat4 matrixRotation() const;
		glm::mat4 matrixScale() const;

		Transform operator+(const Transform& t) const;
		Transform& operator+=(const Transform& t);
	};
} // namespace ntr

#endif