#include "world.hpp"

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

            if (entity->name == "monkey")
            {
                for (int i = 0; i < 1000; i++)
                {
                    Entity *newEntity = add();
                    newEntity->parent = parent;
                    int x = i%4;
                    newEntity->deserialize({{"name", "monkey"},
                                            {"rotation", {0, 0, 0}},
                                            {"position", {((1 + i / 2) ^ 2) % 10, 0, -100 - i * 5}},
                                            {"scale", {0.4, 0.4, 0.4}},
                                            {"components",
                                            {{{"type", "Mesh Renderer"}, {"mesh", "monkey"}, {"material", "monkey"}},
                                            {{"type", "Movement"}, {"angularVelocity", {0, 100, 0}}, {"linearVelocity", {0, 0, 0.6}}}}}});
                }
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