#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

#include "../ecs/world.hpp"
#include "../ecs/entity.hpp"
#include "../application.hpp"
#include "../asset-loader.hpp"
#include "../components/camera.hpp"
#include "../components/collision.hpp"
#include "../components/mesh-renderer.hpp"


namespace our
{
    class CollisionSystem
    {
    public:

        bool detectCollision(Entity* entity, glm::vec3 center, float factor) {
            float x = center[0], y = center[1], z = center[2];
            float scaleX = entity->localTransform.scale[0]*factor;
            float scaleY = entity->localTransform.scale[1]*factor;
            float scaleZ = entity->localTransform.scale[2]*factor;

            float Ex = entity->localTransform.position[0];
            float Ey = entity->localTransform.position[1];
            float Ez = entity->localTransform.position[2];

            // float distance = glm::distance(center, entity->localTransform.position);

            float minX = Ex - scaleX, maxX = Ex + scaleX;
            float minY = Ey - scaleY, maxY = Ey + scaleY;
            float minZ = Ez - scaleZ, maxZ = Ez + scaleZ;
            return  ((x >= minX && x <= maxX) &&
                    (y >= minY && y <= maxY) &&
                    (z >= minZ && z <= maxZ));

            // alternative collision detection for player
            // float distance = glm::distance(playerCenter, asteroidCenter);
            // if (distance < 7) {
            //     // return to initial position
            //     playerCenter = glm::vec3(0, 0, 10);
            //     score--;
            //     world->markForRemoval(asteroid);
            //     spaceshipCollide = true;
            // }
        }
        

    };
}