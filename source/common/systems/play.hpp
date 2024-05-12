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
        int timeFromLastPostprocess = 0;
        int postProcessIndex = 0;
        int robotsYouKilled = 0;
        void enter(World *world, Application *app, our::ForwardRenderer *forwardRenderer)
        {
            postProcessTestCounter = 0;
            healthPercentage = 100;
            this->forwardRenderer = forwardRenderer;
            this->app = app;
            robotsPassedYou = -1;
            robotsYouKilled = 0;

            for (auto entity : world->getEntities())
            {
                std::string name = entity->name;
                if (name == "rocket")
                    player = entity;
                
            }

            for (int i = 0; i < 100; i++)
            {
                int x = i%4;
                
                auto config = app->getConfig();
                auto monkeyData = config["scene"]["runTimeEntities"][0];
                monkeyData["position"] = {((1 + i / 2 + rand()%60) ^ 2) % 30, 0, -100 - i * 5};
                world->addEntityAndDeserialize(monkeyData);
            }
        }
        
        void update(World *world, float deltaTime)
        {


            timeFromLastPostprocess++;
            if(postProcessIndex == 3 && timeFromLastPostprocess > 50){
                forwardRenderer->changePostprocessShaderMode(0);
                postProcessIndex = 0;
            } else if(postProcessIndex == 1 && timeFromLastPostprocess > 10) {
                forwardRenderer->changePostprocessShaderMode(0);
                postProcessIndex = 0;
            }


            CameraComponent* camera = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                if(camera) break;
            }

            Entity* entity = camera->getOwner();
            glm::vec3& position = entity->localTransform.position;

            if (app->getKeyboard().justPressed(GLFW_KEY_SPACE)){
                auto bulletData = app->getConfig()["scene"]["runTimeEntities"][1];
                bulletData["position"] = {position[0]+0.2,position[1]+2,position[2]-5};
                world->addEntityAndDeserialize(bulletData);
            }

            int monkeysRemaining = 0;
            for (auto ent : world->getEntities()){
                glm::vec3& playerCenter = player->parent->localTransform.position;
                if(ent->name == "monkey" && ent->getComponent<CollisionComponent>()){
                    monkeysRemaining++;
                    if (collisionSystem.detectCollision(ent, playerCenter,2)) {
                        hitEnemy(world, ent);
                        healthPercentage -= 20;
                        postProcessIndex = 3;
                        timeFromLastPostprocess = 0;
                        forwardRenderer->changePostprocessShaderMode(postProcessIndex);
                    } else if(ent->localTransform.position[2] - playerCenter[2] > 2) {
                        robotsPassedYou++;
                        postProcessIndex = 1;
                        timeFromLastPostprocess = 0;
                        forwardRenderer->changePostprocessShaderMode(postProcessIndex);
                        ent->deleteComponent<CollisionComponent>();
                        continue;
                    }
                    for (auto bullet : world->getEntities())
                    {
                        glm::vec3 bulletCenter = bullet->localTransform.position;
                        if (bullet->name == "bullet" && bullet->getComponent<CollisionComponent>()){
                            if (collisionSystem.detectCollision(ent, bulletCenter,2.0f)) {
                                hitEnemy(world, ent);
                                bullet->deleteComponent<CollisionComponent>();
                                robotsYouKilled++;
                                break;
                            }
                        }
                    }
                }
            }

            if(monkeysRemaining == 0 && healthPercentage > 0 && robotsPassedYou <= 5 && robotsYouKilled > 0){
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
                ImGui::ProgressBar(1.0 * healthPercentage / 100.0f, ImVec2(130.0f, 20.0f), std::to_string((int) healthPercentage).c_str());

                std::string score_and_level =
                    "Robots Escaped: " + std::to_string((int)robotsPassedYou);
                ImGui::TextUnformatted(score_and_level.c_str());
                
                ImGui::End();
            }
        }

        void applyPostprocessing()
        {
            forwardRenderer->applyPostprocessShader();
        }
    };

} // namespace our
