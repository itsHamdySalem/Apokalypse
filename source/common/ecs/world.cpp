#include "world.hpp"
#include <iostream>
namespace our {

    // This will deserialize a json array of entities and add the new entities to the current world
    // If parent pointer is not null, the new entities will be have their parent set to that given pointer
    // If any of the entities has children, this function will be called recursively for these children
    void World::deserialize(const nlohmann::json &data, Entity *parent)
    {
        if (!data.is_array())
            return;
        for (const auto &entityData : data)
        {
            Entity *entity = add(); // to create an entity and add it to the world
            entity->parent = parent;
            entity->deserialize(entityData);

            std::cout << entity->name << "\n";

            if (entity->name == "monkey")
            {
                for (int i = 0; i < 100; i++)
                {
                    Entity *newEntity = add();
                    newEntity->parent = parent;
                    int x = i%4;
                    newEntity->deserialize({{"name", "monkey"},
                                            {"rotation", {0, 90, 0}},
                                            {"position", {((1 + i / 2 + rand()%60) ^ 2) % 30, 0, -60 - i * 5}},
                                            {"scale", {1.5, 1.5, 1.5}},
                                            {"components",
                                            {
                                            {{"type", "Mesh Renderer"}, {"mesh", "monkey"}, {"material", "monkey"}},
                                            {{"type", "Movement"}, {"angularVelocity", {0, 0, 0}}, {"linearVelocity", {0, 0, 1.6}}},
                                            {{"type", "collision"}, {"mesh", "monkey"}}
                                            }}});
                }
            }

            if (entity->name == "bullet")
            {
                std::cout << "Add Bullet\n";
            }
            // data is map of maps
            // entityData is map
            // if there is a key called "children" in the map so it has children
            if (entityData.contains("children"))
            {
                // TODO: (Req 8) Recursively call this world's "deserialize" using the children data
                //  and the current entity as the parent
                deserialize(entityData["children"], entity);
            }
        }
    }

}