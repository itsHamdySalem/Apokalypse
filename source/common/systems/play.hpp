#pragma once

#include "../components/camera.hpp"
#include "../components/collision.hpp"
#include "../ecs/world.hpp"
#include <ctime>

#include "../application.hpp"

#include "../systems/forward-renderer.hpp"
#include "./collision-system.hpp"

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

#define TIME_LEVEL_1 30
#define TIME_LEVEL_2 60
#define TIME_LEVEL_3 90
#define TIME_LEVEL_4 120
#define TIME_LEVEL_5 160
// the duration of post processing in frames
// after this duration, the post processing will return to the default
#define postProcessingDuration 20

namespace our
{
    class PlaySystem
    {
        Application *app;
        our::CollisionSystem collisionSystem;
        Entity *laser;
        Entity *player;

        our::ForwardRenderer *forwardRenderer;
        int postProcessTestCounter;

    public:
        int robotsPassedYou;
        int healthPercentage;
        void enter(World *world, Application *app, our::ForwardRenderer *forwardRenderer)
        {
            postProcessTestCounter = 0;
            healthPercentage = 100;
            this->forwardRenderer = forwardRenderer;
            this->app = app;
            robotsPassedYou = 0;

            for (auto entity : world->getEntities())
            {
                std::string name = entity->name;
                if (name == "rocket")
                    player = entity;
                
            }
        }
        
        void update(World *world, float deltaTime)
        {

            CameraComponent* camera = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                if(camera) break;
            }

            Entity* entity = camera->getOwner();
            glm::vec3& position = entity->localTransform.position;

            if (app->getKeyboard().justPressed(GLFW_KEY_SPACE)){
                world->addEntityAndDeserialize({{"name", "bullet"},
                                            {"rotation", {270,0,0}},
                                            {"position", {position[0]+0.2,position[1]+2,position[2]-5}},
                                            {"scale", {10, 10, 10}},
                                            {"components",
                                            {
                                            {{"type", "Mesh Renderer"}, {"mesh", "bullet"}, {"material", "bullet"}},
                                            {{"type", "Movement"}, {"angularVelocity", {0, 0, 0}}, {"linearVelocity", {0, 0, -50}}},
                                            {{"type", "collision"}, {"mesh", "bullet"}}
                                            }}});

            }

            int monkeysRemaining = 0;
            for (auto ent : world->getEntities()){
                glm::vec3& playerCenter = player->parent->localTransform.position;
                if(ent->name == "monkey" && ent->getComponent<CollisionComponent>()){
                    monkeysRemaining++;
                    if (collisionSystem.detectCollision(ent, playerCenter,3)) {
                        hitEnemy(world, ent);
                        healthPercentage -= 20;
                    } else if(ent->localTransform.position[2] - playerCenter[2] > 2) {
                        robotsPassedYou++;
                        ent->deleteComponent<CollisionComponent>();
                        continue;
                    }
                    for (auto bullet : world->getEntities())
                    {
                        glm::vec3 bulletCenter = bullet->localTransform.position;
                        if (bullet->name == "bullet" && bullet->getComponent<CollisionComponent>()){
                            if (collisionSystem.detectCollision(ent, bulletCenter,2.2f)) {
                                hitEnemy(world, ent);
                                bullet->deleteComponent<CollisionComponent>();
                                break;
                            }
                        }
                    }
                }
            }

            if(monkeysRemaining == 0 && healthPercentage > 0 && robotsPassedYou <= 5){
                app->changeState("win");
                return;
            } else if(healthPercentage <= 0 || robotsPassedYou > 5) {
                app->changeState("lose");
                return;
            }
        
        }



        void exit()
        {
        }

        void hitEnemy(World *world, Entity *enemy_collision)
        {
            enemy_collision->localTransform.scale = glm::vec3(0, 0, 0);
            enemy_collision->deleteComponent<CollisionComponent>();
            world->markForRemoval(enemy_collision);
        }

        void updateGUI() {
            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoTitleBar;
            window_flags |= ImGuiWindowFlags_NoBackground;
            window_flags |= ImGuiWindowFlags_NoScrollbar;
            window_flags |= ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoResize;

            bool *open_ptr = (bool *)true;
            if (ImGui::Begin("Player", open_ptr, window_flags))
            {
                ImGui::SetWindowSize(ImVec2((float)550, (float)200));
                ImGui::SetWindowPos(ImVec2(app->getWindowSize().x - 200.0f, 0.0f));
                                
                ImGui::Text("Health: ");
                ImGui::SameLine();
                ImGui::ProgressBar(1.0 * healthPercentage / 100.0f, ImVec2(0.0f, 0.0f), std::to_string(healthPercentage).c_str());

                std::string score_and_level =
                    "Robots Escaped: " + std::to_string((int)robotsPassedYou) +
                    "\nLevel: " + std::to_string(0);
                ImGui::TextUnformatted(score_and_level.c_str());
                
                ImGui::End();
            }
        }
    };

} // namespace our
