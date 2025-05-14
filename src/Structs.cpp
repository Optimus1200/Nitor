#include "Structs.h"

namespace ntr
{
	Color::Color(float r, float g, float b, float a)
		: r { r }, g{ g }, b{ b }, a{ a }
	{
	}
	
	Color::Color(const glm::vec4& v)
		: r{ v.x }, g{ v.y }, b{ v.z }, a{ v.x }
	{
	}
	
	Color::operator glm::vec4() const
	{
		return { r, g, b, a };
	}

	Rect::Rect(float x, float y, float width, float height)
		: x{ x }, y{ y }, width{ width }, height{ height }
	{
	}

	Rect::Rect(const glm::vec4& v)
		: x{ v.x }, y{ v.y }, width{ v.z }, height{ v.w }
	{
	}

	Rect::operator glm::vec4() const
	{
		return { x, y, width, height };
	}
}