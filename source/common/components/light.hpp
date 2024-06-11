#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our
{

    class LightComponent : public Component
    {
    public:
        int kind;
        glm::vec3 color;
        glm::vec3 attenuation;
        glm::vec2 cone_angles;

        void deserialize(const nlohmann::json &data) override;

        static std::string getID() { return "light"; }
    };
}