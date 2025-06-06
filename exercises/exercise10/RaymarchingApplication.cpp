#include "RaymarchingApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>
#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/shader/Material.h>
#include <ituGL/renderer/PostFXRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <imgui.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

RaymarchingApplication::RaymarchingApplication()
    : Application(1024, 1024, "Ray-marching demo")
    , m_renderer(GetDevice())
{
}

void RaymarchingApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeMaterial();
    InitializeRenderer();
}

void RaymarchingApplication::Update()
{
    Application::Update();

    // Update camera controller
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    // Set renderer camera
    const Camera& camera = *m_cameraController.GetCamera()->GetCamera();
    m_renderer.SetCurrentCamera(camera);

    // Update the material properties
    m_material->SetUniformValue("ProjMatrix", camera.GetProjectionMatrix());
    m_material->SetUniformValue("InvProjMatrix", glm::inverse(camera.GetProjectionMatrix()));
}

void RaymarchingApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    m_renderer.Render();

    // Render the debug user interface
    RenderGUI();
}

void RaymarchingApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void RaymarchingApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void RaymarchingApplication::InitializeMaterial()
{
    m_material = CreateRaymarchingMaterial("shaders/exercise10.glsl");

    // (todo) 10.X: Initialize material uniforms
    m_material->SetUniformValue("SphereColor", glm::vec3(0.0f, 0.0f, 1.0f));
    m_material->SetUniformValue("SphereCenter", glm::vec3(-2.0f, 0.0f, -10.0f));
    m_material->SetUniformValue("SphereRadius", 1.25f);
    m_material->SetUniformValue("BoxColor", glm::vec3(1.0f, 0.0f, .0f));
    m_material->SetUniformValue("BoxMatrix", glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 2, 0, -10, 1));
    m_material->SetUniformValue("BoxSize", glm::vec3(1.0f, 1.0f, 1.0f));
    m_material->SetUniformValue("Smoothness", .5f);
}

void RaymarchingApplication::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<PostFXRenderPass>(m_material));
}

std::shared_ptr<Material> RaymarchingApplication::CreateRaymarchingMaterial(const char* fragmentShaderPath)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back("shaders/sdflibrary.glsl");
    fragmentShaderPaths.push_back("shaders/raymarcher.glsl");
    fragmentShaderPaths.push_back(fragmentShaderPath);
    fragmentShaderPaths.push_back("shaders/raymarching.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);
    
    return material;
}

void RaymarchingApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for camera controller
    //m_cameraController.DrawGUI(m_imGui);

    if (auto window = m_imGui.UseWindow("Raymarch parameters"))
    {
        auto viewTransform = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

        if (ImGui::TreeNodeEx("Sphere", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 sphereCenter(glm::vec3(-2.0f, 0.0f, -10.0f));
            
            ImGui::ColorEdit3("Sphere Color", m_material->GetDataUniformPointer<float>("SphereColor"));
            ImGui::DragFloat3("Sphere Center", &sphereCenter.x, .1f);
            auto transformedPosition = (viewTransform * glm::vec4(sphereCenter, 1.0));
            m_material->SetUniformValue("SphereCenter", glm::vec3(transformedPosition.x, transformedPosition.y, transformedPosition.z));
            
            ImGui::DragFloat("Sphere Radius", m_material->GetDataUniformPointer<float>("SphereRadius"), .1f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Box", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 translation(2, 0, -10);
            static glm::vec3 rotation(0.0f);

            ImGui::ColorEdit3("Box Color", m_material->GetDataUniformPointer<float>("BoxColor"));
            ImGui::DragFloat3("Box Translation", &translation.x, .1f);
            ImGui::DragFloat3("Box rotation", &rotation.x, .1f);

            auto boxTransform = glm::translate(translation);
            boxTransform = glm::rotate(boxTransform, rotation.x, glm::vec3(1.0f, .0f, .0f));
            boxTransform = glm::rotate(boxTransform, rotation.y, glm::vec3(.0f, 1.0f, .0f));
            boxTransform = glm::rotate(boxTransform, rotation.z, glm::vec3(.0f, .0f, 1.0f));

            m_material->SetUniformValue("BoxMatrix", viewTransform * boxTransform);

            ImGui::DragFloat("Box Size", m_material->GetDataUniformPointer<float>("BoxSize"), .1f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Ray Marching", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat("Smoothness", m_material->GetDataUniformPointer<float>("Smoothness"), .01f);
            ImGui::TreePop();
        }
    }

    m_imGui.EndFrame();
}
