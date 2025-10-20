#include "globals.h"

void VB::init()
{
    m_logger = std::make_shared<Logger>();
    m_camera = std::make_shared<Camera>(glm::vec3(800.0f, 100.0f, 950.0f));
    m_chunksystem = std::make_shared<MultiChunkSystem>();
    m_noisegenerator = std::make_shared<NoiseGenerator>();
    m_voxel = std::make_shared<VoxelRenderer>();
    m_clock = std::make_shared<Clock>();
    m_player = std::make_shared<Player>();
    m_gui = std::make_shared<GUI>();
    m_input = std::make_shared<Input>();
    m_window = std::make_shared<Window>(1920, 1080, "VoxelByte");
    
    VB::inst().GetLogger()->Print("Singleton class constructed");
}

VB VB::m_voxelbyte_inst;