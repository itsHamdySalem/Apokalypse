#include "collision.hpp"
#include "mesh-renderer.hpp"
#include "../deserialize-utils.hpp"
#include "../asset-loader.hpp"
#include "../ecs/entity.hpp"
#include "../mesh/mesh.hpp"

namespace our {
    void CollisionComponent::deserialize(const nlohmann::json &data)
    {
        try {
            if (!data.is_object()) {
                return;
            }
            
            mesh = AssetLoader<Mesh>::get(data["mesh"].get<std::string>());
            
            minCollider = mesh->minVert;
            maxCollider = mesh->maxVert;
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Error deserializing collisionComponent: " + std::string(e.what()));
        }
    }
}