#pragma once

#include "../ecs/component.hpp"
#include "../ecs/transform.hpp"

#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

namespace our {

    enum class LightType {
        DIRECTIONAL,
        SPOT,
        POINT
    };

    class LightComponent : public Component {
    public:
        // Constants for default values
        static constexpr glm::vec3 DEFAULT_COLOR {1.0f, 1.0f, 1.0f};
        static constexpr glm::vec3 DEFAULT_DIFFUSE {1.0f, 1.0f, 1.0f};
        static constexpr glm::vec3 DEFAULT_SPECULAR {1.0f, 1.0f, 1.0f};
        static constexpr glm::vec3 DEFAULT_DIRECTION {1.0f, 1.0f, 1.0f};
        static constexpr glm::vec3 DEFAULT_ATTENUATION {1.0f, 0.0f, 0.0f};
        static constexpr glm::vec2 DEFAULT_CONE_ANGLES {glm::radians(10.0f), glm::radians(80.0f)};

        int lightType;
        glm::vec3 color;
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 direction;
        glm::vec3 attenuation;
        glm::vec2 cone_angles;

        void deserialize(const nlohmann::json &data) override;

        static std::string getID() { return "light"; }
    };
}
