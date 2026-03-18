#pragma once

#include <filesystem>

/* Struct that contains any data linked to a texture.
 * From the texture itself to its metadata.
 */
struct sTextureData
{
};

class TextureMgr final
{
public:
    /*
     * Loads the texture using the given path, including its metadata.
     * Obviously, this function will also store the loaded data
     * If errors occur during the loading process, this function must return false.
     * Return true otherwise.
    */
    bool LoadTexture(const std::filesystem::path& path);

    /*
     * Returns the corresponding TextureData using the name of the texture
     */
    const sTextureData& GetTextureData(const std::string& name) const;
};
