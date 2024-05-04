#include "light.hpp"
#include "../deserialize-utils.hpp"
#include <glm/gtx/euler_angles.hpp>
#include <iostream>

namespace our
{

    void LightComponent::deserialize(const nlohmann::json &data)
    {
        try
        {
            // get the light type (default value: "DIRECTIONAL")
            std::string lightTypeStr = data.value("typeOfLight", "DIRECTIONAL");
            if (lightTypeStr == "DIRECTIONAL")
            {
                lightType = static_cast<int>(LightType::DIRECTIONAL);
            }
            else if (lightTypeStr == "POINT")
            {
                lightType = static_cast<int>(LightType::POINT);
            }
            else if (lightTypeStr == "SPOT")
            {
                lightType = static_cast<int>(LightType::SPOT);
            }

            // get the direction of the light (default value: (1, 1, 1))
            direction = glm::vec3(data.value("direction", DEFAULT_DIRECTION));

            // get the diffuse color of the light (default value: (1, 1, 1))
            diffuse = glm::vec3(data.value("diffuse", DEFAULT_DIFFUSE));

            // get the specular color of the light (default value: (1, 1, 1))
            specular = glm::vec3(data.value("specular", DEFAULT_SPECULAR));

            // Handle spotlight specific attributes
            if (lightType == static_cast<int>(LightType::SPOT))
            {
                cone_angles.x = glm::radians(static_cast<float>(data.value("cone_angles.in", DEFAULT_CONE_ANGLES.x)));
                cone_angles.y = glm::radians(static_cast<float>(data.value("cone_angles.out", DEFAULT_CONE_ANGLES.y)));
            }

            // Handle attenuation
            if (lightType == static_cast<int>(LightType::POINT) || lightType == static_cast<int>(LightType::SPOT))
            {
                attenuation = glm::vec3(data.value("attenuation", DEFAULT_ATTENUATION));
            }
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Error deserializing LightComponent: " + std::string(e.what()));
        }
    }
}
