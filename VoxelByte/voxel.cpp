#include "globals.h"

// ----------<[ VOXELRENDERER CLASS IMPLEMENTATION ]>----------
VoxelRenderer::VoxelRenderer()
{
    VB::inst().GetLogger()->Print("VoxelRenderer obj constructed");
}

bool VoxelRenderer::m_initialized = false;
VoxelRenderer::VoxelData VoxelRenderer::m_voxelRegistry[256];

void VoxelRenderer::init() {
    if (m_initialized) return;
    m_initialized = true;

    // Fill with defaults
    for (int i = 0; i < 256; i++) {
        m_voxelRegistry[i] = 
        { 
            .name = "undefined", 
            .color = {1.0f, 0.0f, 1.0f}, 
            .solid = true, 
            .destructible = true, 
            .interactable = false, 
            .flammable = false, 
            .affectedByGravity = false 
        };
    }

    // Voxel ID registry
    m_voxelRegistry[0] = 
    { 
        .name = "air", 
        .color = {1.0f, 1.0f, 1.0f}, 
        .solid = false,
        .destructible = false,
        .interactable = false,
        .flammable = false,
        .affectedByGravity = false
    };
    m_voxelRegistry[1] = 
    { 
        .name = "grass", 
        .color = {0.0f, 0.43f, 0.0f}, 
        .solid = true,
        .destructible = true,
        .interactable = false,
        .flammable = false,
        .affectedByGravity = false
    };
    m_voxelRegistry[2] = 
    { 
        .name = "dirt", 
        .color = {0.30f, 0.15f, 0.0f}, 
        .solid = true,
        .destructible = true,
        .interactable = false,
        .flammable = false,
        .affectedByGravity = false
    };
    m_voxelRegistry[3] = 
    { 
        .name = "stone", 
        .color = {0.36f, 0.36f, 0.36f}, 
        .solid = true,
        .destructible = true,
        .interactable = false,
        .flammable = false,
        .affectedByGravity = false
    };
}

const VoxelRenderer::VoxelData& VoxelRenderer::GetVoxelData(uint8_t voxelId) {
    init();
    return m_voxelRegistry[voxelId];
}

int VoxelRenderer::VoxelMesh::AddVertex(float x, float y, float z, uint8_t id)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(GetVoxelData(id).color.x);
    vertices.push_back(GetVoxelData(id).color.y);
    vertices.push_back(GetVoxelData(id).color.z);
    return static_cast<int>((vertices.size() / 6) - 1);
}

void VoxelRenderer::VoxelMesh::AddIndex(int v0, int v1, int v2) {
    indices.push_back(v0);
    indices.push_back(v1);
    indices.push_back(v2);
}

void VoxelRenderer::SetShader(std::shared_ptr<Shader> shader)
{
    m_voxel_shader = shader;
}

VoxelRenderer::VoxelMesh VoxelRenderer::GenerateChunkMesh(Chunk chunk)
{
    VoxelMesh chunkMesh;

    // Sweep over each axis (X, Y and Z)
    for (int d = 0; d < 3; d++)
    {
        int i = 0, j = 0, k = 0, l = 0, w = 0, h = 0;
        int u = (d + 1) % 3;
        int v = (d + 2) % 3;
        int* x = new int[3] {0};
        int* q = new int[3] {0};

        bool* mask = new bool[Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE] {false};
        q[d] = 1;

        // Check each slice of the chunk one at a time
        for (x[d] = -1; x[d] < Chunk::CHUNK_SIZE;)
        {
            // Compute the mask
            int n = 0;
            for (x[v] = 0; x[v] < Chunk::CHUNK_SIZE; x[v]++)
            {
                for (x[u] = 0; x[u] < Chunk::CHUNK_SIZE; x[u]++)
                {
                    // q determines the direction (X, Y or Z) that we are searching
                    // m.IsBlockAt(x,y,z) takes global map positions and returns true if a block exists there


                    bool blockCurrent;
                    bool blockCompare;

                    if (0 <= x[d]) {
                        uint8_t voxelId = chunk.GetVoxel(glm::ivec3(x[0], x[1], x[2]));
                        blockCurrent = !VoxelRenderer::GetVoxelData(voxelId).solid;
                    }
                    else {
                        blockCurrent = true;
                    }

                    if (x[d] < (Chunk::CHUNK_SIZE - 1)) {
                        uint8_t voxelId = chunk.GetVoxel(glm::ivec3(x[0] + q[0], x[1] + q[1], x[2] + q[2]));
                        blockCompare = !VoxelRenderer::GetVoxelData(voxelId).solid;
                    }
                    else {
                        blockCompare = true;
                    }

                    // The mask is set to true if there is a visible face between two blocks,
                    //   i.e. both aren't empty and both aren't blocks
                    mask[n] = (blockCurrent != blockCompare);
                    n += 1;
                }
            }

            x[d]++;

            n = 0;

            // Generate a mesh from the mask using lexicographic ordering,      
            //   by looping over each block in this slice of the chunk
            for (j = 0; j < Chunk::CHUNK_SIZE; j++)
            {
                for (i = 0; i < Chunk::CHUNK_SIZE;)
                {
                    if (mask[n])
                    {
                        // Compute the width of this quad and store it in w                        
                        //   This is done by searching along the current axis until mask[n + w] is false
                        for (w = 1; ((i + w) < Chunk::CHUNK_SIZE) && mask[n + w]; w++) {}

                        // Compute the height of this quad and store it in h                        
                        //   This is done by checking if every block next to this row (range 0 to w) is also part of the mask.
                        //   For example, if w is 5 we currently have a quad of dimensions 1 x 5. To reduce triangle count,
                        //   greedy meshing will attempt to expand this quad out to CHUNK_SIZE x 5, but will stop if it reaches a hole in the mask

                        bool done = false;
                        for (h = 1; (j + h) < Chunk::CHUNK_SIZE; h++)
                        {
                            // Check each block next to this quad
                            for (k = 0; k < w; k++)
                            {
                                // If there's a hole in the mask, exit
                                if (!mask[n + k + h * Chunk::CHUNK_SIZE])
                                {
                                    done = true;
                                    break;
                                }
                            }

                            if (done)
                                break;
                        }

                        x[u] = i;
                        x[v] = j;

                        // du and dv determine the size and orientation of this face
                        int* du = new int[3] {0};
                        du[u] = w;

                        int* dv = new int[3] {0};
                        dv[v] = h;

                        // Create a quad for this face. Colour, normal or textures are not stored in this block vertex format.

                        int v0 = chunkMesh.AddVertex((float)(x[0]), (float)(x[1]), (float)(x[2]), voxelId);
                        int v1 = chunkMesh.AddVertex((float)(x[0] + du[0]), (float)(x[1] + du[1]), (float)(x[2] + du[2]), voxelId);
                        int v2 = chunkMesh.AddVertex((float)(x[0] + dv[0]), (float)(x[1] + dv[1]), (float)(x[2] + dv[2]), voxelId);
                        int v3 = chunkMesh.AddVertex((float)(x[0] + du[0] + dv[0]), (float)(x[1] + du[1] + dv[1]), (float)(x[2] + du[2] + dv[2]), voxelId);
                        chunkMesh.AddIndex(v0, v1, v2);
                        chunkMesh.AddIndex(v1, v2, v3);

                        // Clear this part of the mask, so we don't add duplicate faces
                        for (l = 0; l < h; l++)
                            for (k = 0; k < w; k++)
                                mask[n + k + l * Chunk::CHUNK_SIZE] = false;

                        // Increment counters and continue
                        i += w;
                        n += w;
                    }
                    else
                    {
                        i++;
                        n++;
                    }
                }
            }
        }
    }

    VB::inst().GetLogger()->Print("Chunk mesh generated. Vtx: " + std::to_string(chunkMesh.vertices.size()) + " Idx: " + std::to_string(chunkMesh.indices.size()));
    return chunkMesh;
}

void VoxelRenderer::BufferVoxelMesh(const ChunkID& chunk_id, VoxelMesh& mesh)
{
    if (VoxelRendererBufferInfoMap.find(chunk_id) != VoxelRendererBufferInfoMap.end()) return;

    if (VoxelRendererVAO == 0) glGenVertexArrays(1, &VoxelRendererVAO);

    GLuint CurrentMeshVBO, CurrentMeshEBO;

    glBindVertexArray(VoxelRendererVAO);

    glGenBuffers(1, &CurrentMeshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &CurrentMeshEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurrentMeshEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    VoxelRenderBufferInfo VRBI;
    VRBI.VBO = CurrentMeshVBO;
    VRBI.EBO = CurrentMeshEBO;
    VRBI.chunk_id = chunk_id;
    VRBI.vtx_size = mesh.vertices.size();
    VRBI.idx_size = mesh.indices.size();
    VRBI.chunk_origin = VB::inst().GetMultiChunkSystem()->GetChunkOrigin(chunk_id);

    VoxelRendererBufferInfoMap.insert(std::make_pair(chunk_id, VRBI));

    glBindVertexArray(0);
}

void VoxelRenderer::DeleteVoxelMesh(const ChunkID& chunk_id)
{
    if (VoxelRendererBufferInfoMap.find(chunk_id) == VoxelRendererBufferInfoMap.end()) return;
    VoxelRenderBufferInfo CurrentMeshBufferInfo = VoxelRendererBufferInfoMap.at(chunk_id);

    glDeleteBuffers(1, &CurrentMeshBufferInfo.VBO);
    glDeleteBuffers(1, &CurrentMeshBufferInfo.EBO);

    VoxelRendererBufferInfoMap.erase(chunk_id);
}

void VoxelRenderer::RenderMesh(const ChunkID& chunk_id)
{
    if (VoxelRendererBufferInfoMap.find(chunk_id) == VoxelRendererBufferInfoMap.end()) return;
    VoxelRenderBufferInfo CurrentMeshBufferInfo = VoxelRendererBufferInfoMap.at(chunk_id);

    glBindVertexArray(VoxelRendererVAO);

    // position attribute
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshBufferInfo.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // color attribute
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurrentMeshBufferInfo.EBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(CurrentMeshBufferInfo.idx_size * 3), GL_UNSIGNED_INT, 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void VoxelRenderer::RenderAllMeshes()
{
    if (m_voxel_shader == nullptr) return;

    m_voxel_shader->use();

    glm::mat4 projection = glm::perspective(glm::radians(VB::inst().GetCamera()->Zoom),
        (float)(VB::inst().GetWindow()->GetWidth()) / (float)(VB::inst().GetWindow()->GetHeight()),
        0.1f, VB::inst().GetGUI()->GetViewDistance());
    glUniformMatrix4fv(glGetUniformLocation(m_voxel_shader->ID, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));

    // View
    glm::mat4 view = VB::inst().GetCamera()->GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(m_voxel_shader->ID, "view"),
        1, GL_FALSE, glm::value_ptr(view));

    // Model
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(m_voxel_shader->ID, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    for (const auto& render_item : VoxelRendererBufferInfoMap)
    {
        m_voxel_shader->setFloat3(  "pos_offset",
                                    render_item.second.chunk_origin.x,
                                    render_item.second.chunk_origin.y,
                                    render_item.second.chunk_origin.z);

        RenderMesh(render_item.first);
    }
}

void VoxelRenderer::FreeRenderMeshes()
{
    for (const auto& render_item : VoxelRendererBufferInfoMap) DeleteVoxelMesh(render_item.first);
    glDeleteVertexArrays(1, &VoxelRendererVAO);
}

// ----------<[ CHUNK CLASS IMPLEMENTATION ]>----------
Chunk::Chunk(ChunkID chunk_id, glm::ivec3 origin)
{
    m_chunkID = chunk_id;
    m_origin = origin;

    m_voxelArray.resize(Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE);
    VB::inst().GetLogger()->Print("Chunk obj with id: " + std::to_string(chunk_id) + " constructed");
}

glm::ivec3 Chunk::getOrigin()
{
    return m_origin;
}

void Chunk::SetVoxel(glm::ivec3 pos, const uint8_t& id)
{
    m_voxelArray.at(pos.x * CHUNK_SIZE * CHUNK_SIZE + pos.y * CHUNK_SIZE + pos.z) = id;
}

uint8_t Chunk::GetVoxel(glm::ivec3 pos) const
{
    return m_voxelArray.at(pos.x * CHUNK_SIZE * CHUNK_SIZE + pos.y * CHUNK_SIZE + pos.z);
}

void Chunk::GenerateChunk()
{
    if (m_generated == true) return;
    m_generated = true;

    for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
    {
        for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
        {
            float height_noise = VB::inst().GetNoiseGenerator()->GetNoise().GetNoise(static_cast<float>(m_origin.x + x), static_cast<float>(m_origin.z + z));

            for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
            {
                uint8_t v_id;

                if ((height_noise + 1.0f) * 32.0f > y)
                    v_id = 1;
                else
                    v_id = 0;

                SetVoxel(glm::vec3(x, y, z), v_id);
            }
        }
    }
}

// ----------<[ MULTICHUNK CLASS IMPLEMENTATION ]>----------
MultiChunkSystem::MultiChunkSystem()
{
    VB::inst().GetLogger()->Print("MultiChunk obj constructed");
}

void MultiChunkSystem::update_chunks()
{
    // get nearest chunk indices in range
    glm::ivec2 nearest_chunk_idx;
    nearest_chunk_idx = pos_to_nearest_chunk_idx(VB::inst().GetCamera()->Position);
    
    for (int x = -m_chunk_gen_radius; x < m_chunk_gen_radius; x++)
    {
        for (int z = -m_chunk_gen_radius; z < m_chunk_gen_radius; z++)
        {
            glm::ivec2 pawsible_chunk_idx(  nearest_chunk_idx.x + x,
                                            nearest_chunk_idx.y + z);

            
            ChunkID curr_id = chunk_idx_id(pawsible_chunk_idx);
            if (m_chunk_list.find(curr_id) != m_chunk_list.end()) continue;
            if (glm::distance(glm::vec2(nearest_chunk_idx), glm::vec2(pawsible_chunk_idx)) > static_cast<float>(m_chunk_gen_radius)) continue;

            std::shared_ptr<Chunk> curr_chunk = std::make_shared<Chunk>(curr_id, glm::ivec3(chunk_idx_to_origin(pawsible_chunk_idx).x, 0, chunk_idx_to_origin(pawsible_chunk_idx).y));
            m_chunk_list.insert({curr_id, curr_chunk});
            curr_chunk->GenerateChunk();

            VB::inst().GetLogger()->Print("ChunkId: " + std::to_string(curr_id) + " X: " + std::to_string(pawsible_chunk_idx.x) + " Y: " + std::to_string(pawsible_chunk_idx.y));
            VB::inst().GetLogger()->Print("Origin X: " + std::to_string(chunk_idx_to_origin(pawsible_chunk_idx).x) + " Z: " + std::to_string(chunk_idx_to_origin(pawsible_chunk_idx).y));

            VoxelRenderer::VoxelMesh curr_mesh = VB::inst().GetVoxel()->GenerateChunkMesh(*(curr_chunk));
            VB::inst().GetVoxel()->BufferVoxelMesh(curr_id, curr_mesh);
        }
    }
}

void MultiChunkSystem::reupdate_chunks()
{
    if ((VB::inst().GetClock()->GetTime() - m_refresh_last_time) < m_refresh_delta_time) return;
    
    update_chunks();
    m_refresh_last_time = VB::inst().GetClock()->GetTime();
}

glm::ivec2 MultiChunkSystem::pos_to_nearest_chunk_idx(glm::vec3 camera_position)
{
    glm::ivec2 idx;
    if (camera_position.x < 0.0f) idx.x = static_cast<int>((camera_position.x - Chunk::CHUNK_SIZE) / Chunk::CHUNK_SIZE);
    else idx.x = static_cast<int>(camera_position.x / Chunk::CHUNK_SIZE);
    if (camera_position.z < 0.0f) idx.y = static_cast<int>((camera_position.z - Chunk::CHUNK_SIZE) / Chunk::CHUNK_SIZE);
    else idx.y = static_cast<int>(camera_position.z / Chunk::CHUNK_SIZE);

    return idx;
}

const std::unordered_map<ChunkID, std::shared_ptr<Chunk>>& MultiChunkSystem::get_chunk_map() const
{
    return m_chunk_list;
}

glm::ivec2 MultiChunkSystem::chunk_idx_to_origin(glm::ivec2 chunk_idx)
{
    return glm::ivec2(  chunk_idx.x * Chunk::CHUNK_SIZE,
                        chunk_idx.y * Chunk::CHUNK_SIZE);
}

inline ChunkID MultiChunkSystem::chunk_idx_id(glm::ivec2 chunk_idx)
{
    ChunkID id;
    id = chunk_idx.x;
    id = id << 32;
    id = id | chunk_idx.y;
    return id;
}

const glm::ivec3 MultiChunkSystem::GetChunkOrigin(const ChunkID& chunk_id)
{
    if (m_chunk_list.find(chunk_id) == m_chunk_list.end()) return glm::ivec3(0, 0, 0);
    else return m_chunk_list.at(chunk_id)->getOrigin();
}

// ----------<[ NOISEGENERATOR CLASS IMPLEMENTATION ]>----------
NoiseGenerator::NoiseGenerator()
{
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

FastNoiseLite NoiseGenerator::GetNoise()
{
    return m_noise;
}