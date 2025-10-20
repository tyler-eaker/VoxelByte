#ifndef VOXEL_H
#define VOXEL_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "shader.h"
#include "../include/FastNoiseLite/FastNoiseLite.h"

class VoxelRenderer;
class Chunk;
class MultiChunkSystem;
class NoiseGenerator;

typedef unsigned long long int ChunkID;

class VoxelRenderer
{
public:
    VoxelRenderer();

    uint8_t voxelId = 1;

    struct VoxelData {
        const char* name;
        glm::vec3 color;
        bool solid;
        bool destructible;
        bool interactable;
        bool flammable;
        bool affectedByGravity;
    };

    static const VoxelData& GetVoxelData(uint8_t voxelId);

    struct VoxelMesh
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        int AddVertex(float x, float y, float z, uint8_t id);
        void AddIndex(int v0, int v1, int v2);
    };

    struct VoxelRenderBufferInfo
    {
        GLuint VBO;
        GLuint EBO;
        ChunkID chunk_id;
        size_t vtx_size;
        size_t idx_size;
        glm::ivec3 chunk_origin;
    };

    void SetShader(std::shared_ptr<Shader> shader);

    VoxelMesh GenerateChunkMesh(Chunk chunk);
    void BufferVoxelMesh(const ChunkID& chunk_id, VoxelMesh& mesh);
    void DeleteVoxelMesh(const ChunkID& chunk_id);
    void RenderMesh(const ChunkID& chunk_id);
    void RenderAllMeshes();
    void FreeRenderMeshes();

    static const float voxelColors[256][3];

private:
    static bool m_initialized;

    std::shared_ptr<Shader> m_voxel_shader = nullptr;

    GLuint VoxelRendererVAO = 0;
    std::unordered_map<ChunkID, VoxelRenderBufferInfo> VoxelRendererBufferInfoMap;


    static VoxelData m_voxelRegistry[256];
    static void init();
};

class Chunk
{
public:
    static const int CHUNK_SIZE = 64;

    Chunk(ChunkID chunk_id, glm::ivec3 origin);
    void GenerateChunk();
    glm::ivec3 getOrigin();
    inline void SetVoxel(glm::ivec3 pos, const uint8_t& vd);
    uint8_t GetVoxel(glm::ivec3 pos) const;

private:
    bool m_generated = false;
    ChunkID m_chunkID;
    glm::ivec3 m_origin;
    std::vector<uint8_t> m_voxelArray;
};

class MultiChunkSystem {
public:
    MultiChunkSystem();
    void update_chunks();
    void reupdate_chunks();

    glm::ivec2 pos_to_nearest_chunk_idx(glm::vec3 camera_position);
    const std::unordered_map<ChunkID, std::shared_ptr<Chunk>>& get_chunk_map() const;
    const glm::ivec3 GetChunkOrigin(const ChunkID& chunk_id);

    
private:
    static const int WORLD_CHUNK_RADIUS = 2048;
    static const int CHUNK_HEIGHT = 1;

    float m_refresh_delta_time = 1.5f;
    float m_refresh_last_time = 0.0f;
    int m_chunk_gen_radius = 4;

    std::unordered_map<ChunkID, std::shared_ptr<Chunk>> m_chunk_list;
    std::vector<std::shared_ptr<Chunk>> loaded_chunks;

    glm::ivec2 chunk_idx_to_origin(glm::ivec2 chunk_idx);
    inline ChunkID chunk_idx_id(glm::ivec2 chunk_idx);
};

class NoiseGenerator
{
public:
    NoiseGenerator();
    
    FastNoiseLite GetNoise();

private:
    int m_seed = 6;
    float m_freq = 0.2f;
    FastNoiseLite m_noise;
    
};

#endif