#ifndef NTR_LIGHT_H
#define NTR_LIGHT_H

#include <glm/vec3.hpp>

namespace ntr
{
    struct DirectionalLight
    {
        alignas(16) glm::vec3 direction = { 0.5f, -0.8f, -0.5f };
        alignas(16) glm::vec3 color = { 10.0f, 10.0f, 10.0f };
    };

    struct PointLight
    {
        alignas(16) glm::vec3 position = { 0.0f, 0.0f, 0.0f };
        alignas(16) glm::vec3 color = { 100.0f, 100.0f, 100.0f };
    };
} // namespace ntr

#endif
