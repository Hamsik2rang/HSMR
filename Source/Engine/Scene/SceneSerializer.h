//
//  SceneSerializer.h
//  Engine
//
//  Scene serialization/deserialization to JSON
//

#pragma once

#include "Precompile.h"
#include <string>

HS_NS_BEGIN

class Scene;
class Entity;

/**
 * @brief Scene serializer for saving/loading scenes to JSON format
 */
class HS_API SceneSerializer
{
public:
    /**
     * @brief Construct serializer for a scene
     * @param scene Scene to serialize/deserialize
     */
    explicit SceneSerializer(Scene* scene);

    /**
     * @brief Save scene to a JSON file
     * @param filePath Path to save the file
     * @return true if successful
     */
    bool SaveToFile(const std::string& filePath);

    /**
     * @brief Save scene to a JSON string
     * @return JSON string representation of the scene
     */
    std::string SaveToString();

    /**
     * @brief Load scene from a JSON file
     * @param filePath Path to the file
     * @return true if successful
     */
    bool LoadFromFile(const std::string& filePath);

    /**
     * @brief Load scene from a JSON string
     * @param jsonString JSON string to parse
     * @return true if successful
     */
    bool LoadFromString(const std::string& jsonString);

    /**
     * @brief Clear the scene and start fresh
     */
    void ClearScene();

private:
    Scene* _scene;

    // Current schema version
    static constexpr const char* SCHEMA_VERSION = "1.0";
};

HS_NS_END
