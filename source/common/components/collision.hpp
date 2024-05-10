#pragma once

#include <glm/glm.hpp>

#include "../mesh/mesh.hpp"
#include "../ecs/component.hpp"

namespace our {
    class CollisionComponent : public Component {
        public:
            Mesh* mesh;
            glm::vec3 minCollider = {0.0f, 0.0f , 0.0f};
            glm::vec3 maxCollider = {0.0f, 0.0f, 0.0f};

            void deserialize(const nlohmann::json &data) override;

            static std::string getID() { return "collision"; }
    };
}