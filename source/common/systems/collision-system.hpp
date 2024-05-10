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
        Entity *detectFiring(World *world, Entity *laser)
        {
            for (auto entity : world->getEntities())
            {
                if ((entity->name == "enemy" || entity->name == "boss") && entity->getComponent<CollisionComponent>())
                {
                    glm::vec3 minLaserVertex = laser->getComponent<CollisionComponent>()->mesh->minVert;
                    glm::vec3 maxLaserVertex = laser->getComponent<CollisionComponent>()->mesh->maxVert;

                    minLaserVertex *= laser->localTransform.scale[2];
                    maxLaserVertex *= laser->localTransform.scale[2];
                    
                    minLaserVertex.x *= laser->localTransform.scale[0] * 0.005;
                    maxLaserVertex.x *= laser->localTransform.scale[0] * 0.005;
                    
                    minLaserVertex.y *= laser->localTransform.scale[0];
                    maxLaserVertex.y *= laser->localTransform.scale[0];
                    
                    if (laser->parent->parent)
                    {
                        minLaserVertex.x *= laser->localTransform.scale[0] * 1000;
                        maxLaserVertex.x *= laser->localTransform.scale[0] * 1000;
                        
                        minLaserVertex += laser->parent->parent->localTransform.position;
                        maxLaserVertex += laser->parent->parent->localTransform.position;
                    }
                    
                    minLaserVertex += laser->localTransform.position + laser->parent->localTransform.position;
                    maxLaserVertex += laser->localTransform.position + laser->parent->localTransform.position;

                    glm::vec3 minCollider = entity->getComponent<CollisionComponent>()->mesh->minVert;
                    glm::vec3 maxCollider = entity->getComponent<CollisionComponent>()->mesh->maxVert;

                    minCollider *= entity->localTransform.scale[0];
                    maxCollider *= entity->localTransform.scale[0];
                    
                    minCollider += entity->localTransform.position;
                    maxCollider += entity->localTransform.position;

                    if ((minLaserVertex.x <= maxCollider.x && maxLaserVertex.x >= minCollider.x) &&
                        (minLaserVertex.y <= maxCollider.y && maxLaserVertex.y >= minCollider.y) &&
                        (minLaserVertex.z <= maxCollider.z && maxLaserVertex.z >= minCollider.z))
                    {
                        return entity;
                    }
                }
            }
            return nullptr;
        }

        Entity *detectCollision(World *world, Entity *player, std::string name)
        {
            for (auto entity : world->getEntities())
            {
                if (entity->name == name && entity->getComponent<CollisionComponent>())
                {
                    Entity *result = checkCollision(player, entity);
                    if (result)
                    {
                        return result;
                    }
                }
            }
            return nullptr;
        }

        Entity *checkCollision(Entity *player, Entity *enemy)
        {
            glm::vec3 minPlayerVertex = player->getComponent<CollisionComponent>()->mesh->minVert;
            glm::vec3 maxPlayerVertex = player->getComponent<CollisionComponent>()->mesh->maxVert;

            minPlayerVertex *= player->localTransform.scale[0];
            maxPlayerVertex *= player->localTransform.scale[0];
            
            if (enemy->name == "enemy")
            {
                minPlayerVertex.y *= 0.1;
                maxPlayerVertex.y *= 0.1;
            }
            
            if (enemy->name == "egg")
            {
                minPlayerVertex.y *= 0.3;
                maxPlayerVertex.y *= 0.3;
            }

            minPlayerVertex += player->localTransform.position + player->parent->localTransform.position;
            maxPlayerVertex += player->localTransform.position + player->parent->localTransform.position;

            glm::vec3 minCollider = enemy->getComponent<CollisionComponent>()->mesh->minVert;
            glm::vec3 maxCollider = enemy->getComponent<CollisionComponent>()->mesh->maxVert;

            minCollider *= enemy->localTransform.scale[0];
            maxCollider *= enemy->localTransform.scale[0];

            minCollider += enemy->localTransform.position;
            maxCollider += enemy->localTransform.position;

            if (enemy->name == "heart")
            {
                minCollider.y += 5;
                maxCollider.y += 5;
            }

            if ((minPlayerVertex.x <= maxCollider.x && maxPlayerVertex.x >= minCollider.x) &&
                (minPlayerVertex.y <= maxCollider.y && maxPlayerVertex.y >= minCollider.y) &&
                (minPlayerVertex.z <= maxCollider.z && maxPlayerVertex.z >= minCollider.z))
            {
                return enemy;
            }

            return nullptr;
        }
    };
}