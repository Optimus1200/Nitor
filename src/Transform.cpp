#include <glm/ext/matrix_transform.hpp>

#include "Transform.h"


namespace ntr
{
	glm::mat4 Transform::matrix() const
	{
		glm::mat4 matrix{ 1.0f };
		
		matrix = glm::translate(matrix, position);
		matrix = glm::rotate(matrix, glm::radians(rotation.x), { 1, 0, 0 });
		matrix = glm::rotate(matrix, glm::radians(rotation.y), { 0, 1, 0 });
		matrix = glm::rotate(matrix, glm::radians(rotation.z), { 0, 0, 1 });
		matrix = glm::scale(matrix, scale);
		
		return matrix;
	}
	
	glm::mat4 Transform::matrixPosition() const
	{
		return glm::translate(glm::mat4(1.0f), position);
	}
	
	glm::mat4 Transform::matrixRotation() const
	{
		glm::mat4 matrix{ 1.0f };
		matrix = glm::rotate(matrix, glm::radians(rotation.x), { 1, 0, 0 });
		matrix = glm::rotate(matrix, glm::radians(rotation.y), { 0, 1, 0 });
		matrix = glm::rotate(matrix, glm::radians(rotation.z), { 0, 0, 1 });

		return matrix;
	}
	
	glm::mat4 Transform::matrixScale() const
	{
		return glm::scale(glm::mat4(1.0f), scale);
	}
	
	Transform Transform::operator+(const Transform& t) const
	{
		return Transform {
			position + t.position,
			rotation + t.rotation,
			scale * t.scale
		};
	}

	Transform& Transform::operator+=(const Transform& t)
	{
		position += t.position;
		rotation += t.rotation;
		scale *= t.scale;
		return *this;
	}
} // namespace ntr