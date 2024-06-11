#pragma once

#pragma once

#include "menu-state.hpp"
#include <application.hpp>
#include <functional>
#include <material/material.hpp>
#include <mesh/mesh.hpp>
#include <shader/shader.hpp>
#include <texture/texture-utils.hpp>
#include <texture/texture2d.hpp>



class WinState : public our::State
{
public:
    void onInitialize() override {}
    
    void onDraw(double deltaTime) override
    {
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void onDestroy() override {}

    void onImmediateGui() override
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground |
                                        ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoScrollbar;

        ImVec2 screen_center = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);

        ImGui::SetNextWindowPos(screen_center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        
        ImGui::Begin("Game Win", nullptr, window_flags);
        
        // Display the main message in bold and larger font
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Assuming the first font is bold
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 20)); // Add some spacing
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Congrats! You saved the world!");
        ImGui::PopStyleVar(); // Restore spacing
        ImGui::PopFont(); // Restore font
        
        // Display the small text for playing again
        ImGui::Text("Press Escape to play again");

        ImGui::End();

        auto& keyboard = getApp()->getKeyboard();
        if(keyboard.justPressed(GLFW_KEY_SPACE)){
            // If the space key is pressed in this frame, go to the play state
            getApp()->changeState("play");
        }
    }
};
