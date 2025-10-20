#ifndef GLOBALS_H
#define GLOBALS_H

#include "logger.h"
#include "camera.h"
#include "voxel.h"
#include "clock.h"
#include "player.h"
#include "gui.h"
#include "input.h"
#include "window.h"

#include <memory>

class VB
{
public:
    static VB& inst() { return m_voxelbyte_inst; }

    void init();

    std::shared_ptr<Logger>             GetLogger()             { return m_logger; }
    std::shared_ptr<Camera>             GetCamera()             { return m_camera; }
    std::shared_ptr<MultiChunkSystem>   GetMultiChunkSystem()   { return m_chunksystem; }
    std::shared_ptr<NoiseGenerator>     GetNoiseGenerator()     { return m_noisegenerator; }
    std::shared_ptr<VoxelRenderer>      GetVoxel()              { return m_voxel; }
    std::shared_ptr<Clock>              GetClock()              { return m_clock; }
    std::shared_ptr<Player>             GetPlayer()             { return m_player; }
    std::shared_ptr<GUI>                GetGUI()                { return m_gui; }
    std::shared_ptr<Input>              GetInput()              { return m_input; }
    std::shared_ptr<Window>             GetWindow()             { return m_window; }

private:
    static VB m_voxelbyte_inst;

    std::shared_ptr<Logger>             m_logger;
    std::shared_ptr<Camera>             m_camera;
    std::shared_ptr<MultiChunkSystem>   m_chunksystem;
    std::shared_ptr<NoiseGenerator>     m_noisegenerator;
    std::shared_ptr<VoxelRenderer>      m_voxel;
    std::shared_ptr<Clock>              m_clock;
    std::shared_ptr<Player>             m_player;
    std::shared_ptr<GUI>                m_gui;
    std::shared_ptr<Input>              m_input;
    std::shared_ptr<Window>             m_window;

};

#endif
