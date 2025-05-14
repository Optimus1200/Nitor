#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "Camera.h"
#include <iostream>


namespace ntr
{
	glm::vec3 Camera::front() const
	{
		return glm::normalize(glm::vec3{
			cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x)),
			sin(glm::radians(rotation.x)),
			sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x))
			});
	}

	glm::vec3 Camera::right() const
	{
		return glm::normalize(glm::cross(front(), WORLD_UP));
	}

	glm::vec3 Camera::up() const
	{
		glm::vec3 cFront = front();
		glm::vec3 cRight = glm::normalize(glm::cross(cFront, WORLD_UP));

		return glm::normalize(glm::cross(cRight, cFront));
	}

	glm::mat4 Camera::view() const
	{
		glm::vec3 cFront	= front();
		glm::vec3 cRight	= glm::normalize(glm::cross(cFront, WORLD_UP));
		glm::vec3 cUp		= glm::normalize(glm::cross(cRight, cFront));

		return glm::lookAt(position, position + cFront, cUp);
	}

	glm::mat4 Camera::projection() const
	{
		if (ortho)
		{
			float aspect = (float)viewport.width / viewport.height;
			return glm::ortho(-viewport.width, viewport.width, -viewport.height, viewport.height, zNear, zFar);
		}

		return glm::perspective(glm::radians(fovY), viewport.width / viewport.height, zNear, zFar);
	}
} // namespace ntr