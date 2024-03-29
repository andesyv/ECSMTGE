#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <map>
#include <string>
#include <memory>

#include "texture.h"

#include "vertex.h"

#include "meshdata.h"

#ifdef _WIN32
#include <al.h>
#include <alc.h>
#elif __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include "wavfilehandler.h"

class SoundSource;

/**
 * @brief The resource manager is responsible for handling all resources, making only a single copy whenever possible to keep memory usage low.
 */
class ResourceManager : protected QOpenGLFunctions_4_1_Core
{

    friend class App;
private:

    static ResourceManager* inst;

    ResourceManager();

public:
    static ResourceManager& instance()
    {
        assert(inst != nullptr);
        return *inst;
    }

    virtual ~ResourceManager();

    /**
     * @brief Helper function that iteratively goes through all folders in gsl::assetFilePath and loads the files.
     *  NOTE: Does not include shaders. Only meshes, textures and sounds.
     */
    void LoadAssetFiles();

    // Shaders

    void addShader(const std::string& name, std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> getShader(const std::string& name);

    // Textures

    void addTexture(const std::string& name, const std::string& path, GLenum type = GL_TEXTURE_2D);
    std::shared_ptr<Texture> getTexture(const std::string& name);

    // Meshes

    std::shared_ptr<MeshData> addMesh(const std::string& name, const std::string& path, GLenum renderType = GL_TRIANGLES);
    std::shared_ptr<MeshData> getMesh(const std::string& name);

    /**
     * @brief Adds the LOD meshdata to the baseMesh meshdata at the given level and removes the LOD mesh from the mesharray.
     * @param baseMesh - The receiver of the LOD
     * @param LOD - The meshdata of the LOD
     * @param level - At what level the LOD shall be added; 0, 1 or 2. Defaults to 1. (0 is the basemesh)
     */
    void setupLOD(std::shared_ptr<MeshData> baseMesh, std::shared_ptr<MeshData> LOD, unsigned int level = 1);

    // Sound

    void loadWav(const std::string& name, const std::string& path);
    wave_t* getWav(const std::string& name);
    int getSourceBuffer(const std::string& name);

    // Helper functions for UI

    std::vector<std::string> getAllMeshNames();
    std::vector<std::string> getAllShaderNames();
    std::vector<std::string> getAllTextureNames();
    std::vector<std::string> getAllWavFileNames();

private:

    /**
     * @brief Reads an obj file at the specified fileName. Relative means gsl::assetFilePath/Meshes if true, working directory if false
     */
    std::pair<std::vector<Vertex>, std::vector<GLuint>> readObjFile(std::string filename, bool relative = true);

    /**
     * @brief Reads a txt file written like the Vertex << overload is setup. Can only be read by the Vertex >> overload.
     */
    std::pair<std::vector<Vertex>, std::vector<GLuint>> readTxtFile(std::string filename);

    /**
     * @brief Sets up LOD meshes for a .obj. It looks for same name suffixed with '_L01/_L02'. Hardcoded to create 2 LODs
     */
    std::pair<std::shared_ptr<MeshData>, std::shared_ptr<MeshData> > initializeLODs(const std::string& name, const std::string& path, GLenum renderType);

    /**
     * @brief Sets up the base mesh data vao, vertexcount and indexcount arrays with the provided values from LOD1 and LOD2 respectively.
     */
    void setupLODs(std::shared_ptr<MeshData> baseMeshData, std::shared_ptr<MeshData> LOD1 = nullptr, std::shared_ptr<MeshData> LOD2 = nullptr);

    /**
     * @brief Creates the MeshData. Does the OpenGL initialization of buffers using the provided vertex and indices data.
     */
    std::shared_ptr<MeshData> initializeMeshData(const std::string& name, GLenum renderType, std::pair<std::vector<Vertex>, std::vector<GLuint>> data);

    /**
     * @brief Calculates bounds for the mesh based on vertices
     * @param vertices - list of vertices for mesh
     * @return A bounding sphere in local space
     */
    static MeshData::Bounds CalculateBounds(const std::vector<Vertex>& vertices);

    // Data

    std::map<std::string, std::shared_ptr<Shader>> mShaders;
    std::map<std::string, std::shared_ptr<Texture>> mTextures;
    std::map<std::string, std::shared_ptr<MeshData>> mMeshes;
    std::map<std::string, wave_t*> mWavFiles;
    std::map<std::string, unsigned int> mSourceBuffers;

    // Used to check if it's needed to call initializeOpenGLFunctions().
    bool mIsInitialized = false;
};

#endif // RESOURCEMANAGER_H
