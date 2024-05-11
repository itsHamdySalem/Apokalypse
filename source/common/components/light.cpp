#include "light.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our
{
    void LightComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
        {
            return;
        }

        kind = data.value("kind", 0);
        color = data.value("color", glm::vec3(1.0f, 0.0f, 0.0f));
        attenuation = data.value("attenuation", glm::vec3(1.0f, 0.0f, 0.0f));
        cone_angles = data.value("cone_angles", glm::vec2(0.0f, 0.0f));
    }
}
