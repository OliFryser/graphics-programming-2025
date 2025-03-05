#include "ViewerApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include <assert.h>
#include <iostream>

ViewerApplication::ViewerApplication()
    : Application(1024, 1024, "Viewer demo")
    , m_cameraPosition(0, 30, 30)
    , m_cameraTranslationSpeed(20.0f)
    , m_cameraRotationSpeed(0.5f)
    , m_cameraEnabled(false)
    , m_cameraEnablePressed(false)
    , m_mousePosition(GetMainWindow().GetMousePosition(true))
    , m_ambientColor(0.25f)
    , m_lightColor(1.0f)
    , m_lightIntensity(1.0f)
    , m_lightPosition(-10.0f, 20.0f, 10.0f)
    , m_millIndex(2)
    , m_groundIndex(1)
    , m_shadowIndex(0)
    , m_millDiffuseReflection(1.0f)
    , m_groundDiffuseReflection(1.0f)
    , m_shadowDiffuseReflection(1.0f)
    , m_millAmbientReflection(1.0f)
    , m_groundAmbientReflection(1.0f)
    , m_shadowAmbientReflection(1.0f)
    , m_millSpecularReflection(1.0f)
    , m_groundSpecularReflection(1.0f)
    , m_shadowSpecularReflection(1.0f)
    , m_millSpecularExponent(100.0f)
    , m_groundSpecularExponent(100.0f)
    , m_shadowSpecularExponent(100.0f)
{
}

void ViewerApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeModel();
    InitializeCamera();
    InitializeLights();

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    device.SetVSyncEnabled(true);
}

void ViewerApplication::Update()
{
    Application::Update();

    // Update camera controller
    UpdateCamera();

    m_model.GetMaterial(m_shadowIndex).SetUniformValue("AmbientReflection", m_shadowAmbientReflection);
    m_model.GetMaterial(m_groundIndex).SetUniformValue("AmbientReflection", m_groundAmbientReflection);
    m_model.GetMaterial(m_millIndex).SetUniformValue("AmbientReflection", m_millAmbientReflection);

    m_model.GetMaterial(m_shadowIndex).SetUniformValue("DiffuseReflection", m_shadowDiffuseReflection);
    m_model.GetMaterial(m_groundIndex).SetUniformValue("DiffuseReflection", m_groundDiffuseReflection);
    m_model.GetMaterial(m_millIndex).SetUniformValue("DiffuseReflection", m_millDiffuseReflection);

    m_model.GetMaterial(m_shadowIndex).SetUniformValue("SpecularReflection", m_shadowSpecularReflection);
    m_model.GetMaterial(m_groundIndex).SetUniformValue("SpecularReflection", m_groundSpecularReflection);
    m_model.GetMaterial(m_millIndex).SetUniformValue("SpecularReflection", m_millSpecularReflection);

    m_model.GetMaterial(m_shadowIndex).SetUniformValue("SpecularExponent", m_shadowSpecularExponent);
    m_model.GetMaterial(m_groundIndex).SetUniformValue("SpecularExponent", m_groundSpecularExponent);
    m_model.GetMaterial(m_millIndex).SetUniformValue("SpecularExponent", m_millSpecularExponent);
}

void ViewerApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    m_model.Draw();
    RenderGUI();
}

void ViewerApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void ViewerApplication::InitializeModel()
{
    // Load and build shader
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/blinn-phong.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/blinn-phong.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("AmbientColor");
    filteredUniforms.insert("LightColor");
    filteredUniforms.insert("LightPosition");
    filteredUniforms.insert("CameraPosition");

    // Create reference material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, filteredUniforms);
    material->SetUniformValue("Color", glm::vec4(1.0f));
    material->SetUniformValue("AmbientReflection", 1.0f);
    material->SetUniformValue("DiffuseReflection", 1.0f);
    material->SetUniformValue("SpecularReflection", 1.0f);
    material->SetUniformValue("SpecularExponent", 100.0f);

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");

    material->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
            shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
            shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
            shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
            shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
        }); 

    // Configure loader
    ModelLoader loader(material);
    loader.SetCreateMaterials(true);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

    // Load model
    m_model = loader.Load("models/mill/Mill.obj");

    Texture2DLoader textureLoader(TextureObject::Format::FormatRGBA, TextureObject::InternalFormat::InternalFormatRGBA8);
    textureLoader.SetFlipVertical(true);

    auto millColor = textureLoader.LoadShared("models/mill/MillCat_color.jpg");
    auto groundColor = textureLoader.LoadShared("models/mill/Ground_color.jpg");
    auto groundShadow = textureLoader.LoadShared("models/mill/Ground_shadow.jpg");

    m_model.GetMaterial(m_shadowIndex).SetUniformValue("ColorTexture", groundShadow);
    m_model.GetMaterial(m_groundIndex).SetUniformValue("ColorTexture", groundColor);
    m_model.GetMaterial(m_millIndex).SetUniformValue("ColorTexture", millColor);
}

void ViewerApplication::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}

void ViewerApplication::InitializeLights()
{
    // (todo) 05.X: Initialize light variables

}

void ViewerApplication::RenderGUI()
{
    m_imGui.BeginFrame();
    if (ImGui::CollapsingHeader("Ambient Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Ambient Color", &m_ambientColor.r);
    }
    
    if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Light Position", &m_lightPosition.x);
        ImGui::ColorEdit3("Light Color", &m_lightColor.r);
        ImGui::DragFloat("Light Intensity", &m_lightIntensity, .01f, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Material Properties"))
    {
        ImGui::DragFloat("Mill Ambient Intensity", &m_millAmbientReflection, .01f, 0.0f, 1.0f);
        ImGui::DragFloat("Ground Ambient Intensity", &m_groundAmbientReflection, .01f, 0.0f, 1.0f);
        ImGui::DragFloat("Shadow Ambient Intensity", &m_shadowAmbientReflection, .01f, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::DragFloat("Mill Diffuse Intensity", &m_millDiffuseReflection, .01f, 0.0f, 1.0f);
        ImGui::DragFloat("Ground Diffuse Intensity", &m_groundDiffuseReflection, .01f, 0.0f, 1.0f);
        ImGui::DragFloat("Shadow Diffuse Intensity", &m_shadowDiffuseReflection, .01f, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::DragFloat("Mill Specular Intensity", &m_millSpecularReflection, .01f, 0.0f, 1.0f);
        ImGui::DragFloat("Ground Specular Intensity", &m_groundSpecularReflection, .01f, 0.0f, 1.0f);
        ImGui::DragFloat("Shadow Specular Intensity", &m_shadowSpecularReflection, .01f, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::DragFloat("Mill Specular Exponent", &m_millSpecularExponent, 1.0f, 0.0f, 200.0f);
        ImGui::DragFloat("Ground Specular Exponent", &m_groundSpecularExponent, 1.0f, 0.0f, 200.0f);
        ImGui::DragFloat("Shadow Specular Exponent", &m_shadowSpecularExponent, 1.0f, 0.0f, 200.0f);
    }

    m_imGui.EndFrame();
}

void ViewerApplication::UpdateCamera()
{
    Window& window = GetMainWindow();

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_cameraEnablePressed)
        {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_cameraEnablePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A))
            inputTranslation.x = -1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_D))
            inputTranslation.x = 1.0f;

        if (window.IsKeyPressed(GLFW_KEY_W))
            inputTranslation.y = 1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_S))
            inputTranslation.y = -1.0f;

        inputTranslation *= m_cameraTranslationSpeed;
        inputTranslation *= GetDeltaTime();

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

    // Update camera rotation
   {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_cameraRotationSpeed;

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0,1,0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

   // Update view matrix
   m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}
