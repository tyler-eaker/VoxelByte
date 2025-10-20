#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globals.h"

#include "shader.h"
#include "window.h"

#include <iostream>
#include <unordered_map>

int main() {

    // Initialize Global Class
    VB::inst().init();

    // Initialize ImGUI
    VB::inst().GetGUI()->InitializeImGUI(VB::inst().GetWindow()->GetGLFWwindow());

    std::shared_ptr<Shader> VoxelShader = std::make_shared<Shader>("../shaders/voxel_vert.glsl", "../shaders/voxel_frag.glsl");
    Shader crosshairShader("../shaders/crosshair_vert.glsl", "../shaders/crosshair_frag.glsl");

    // Setup crosshair
    VB::inst().GetGUI()->SetupCrosshairMesh();

    VB::inst().GetVoxel()->SetShader(VoxelShader);

    VB::inst().GetMultiChunkSystem()->update_chunks();
    // for (const auto& id_chunk : VB::inst().GetMultiChunkSystem()->get_chunk_map())
    // {
    //     VoxelRenderer::VoxelMesh curr_mesh = VB::inst().GetVoxel()->GenerateChunkMesh(*(id_chunk.second));
    //     VB::inst().GetVoxel()->BufferVoxelMesh(id_chunk.first, curr_mesh);
    // }

    // Enable OpenGL functionality
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!VB::inst().GetWindow()->ShouldClose()) {
        VB::inst().GetClock()->Update();
        VB::inst().GetMultiChunkSystem()->reupdate_chunks();

        VB::inst().GetInput()->ProcessInput(VB::inst().GetWindow()->GetGLFWwindow());

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        VB::inst().GetVoxel()->RenderAllMeshes();

        crosshairShader.use();
        glBindVertexArray(VB::inst().GetGUI()->crosshairVAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        VB::inst().GetGUI()->UpdateImGui(VB::inst().GetWindow()->GetGLFWwindow());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        VB::inst().GetWindow()->SwapBuffers();
        VB::inst().GetWindow()->PollEvents();
    }

    // Cleanup
    VB::inst().GetVoxel()->FreeRenderMeshes();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
