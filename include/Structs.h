#ifndef NTR_STRUCTS_H
#define NTR_STRUCTS_H

#include <glm/vec4.hpp>

namespace ntr
{
	struct Color
	{
		static constexpr float MIN = 0.0f;
		static constexpr float MAX = 255.0f;

		float r, g, b, a;

		Color(float r = MAX, float g = MAX, float b = MAX, float a = MAX);
		Color(const glm::vec4& v);
		operator glm::vec4() const;
	};

	struct Rect
	{
		float x, y, width, height;

		Rect(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
		Rect(const glm::vec4& v);
		operator glm::vec4() const;
	};
}

#endif