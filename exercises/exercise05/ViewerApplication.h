#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>

class Texture2DObject;

class ViewerApplication : public Application
{
public:
    ViewerApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeModel();
    void InitializeCamera();
    void InitializeLights();

    void UpdateCamera();

    void RenderGUI();

private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Mouse position for camera controller
    glm::vec2 m_mousePosition;

    // Camera controller parameters
    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    // Loaded model
    Model m_model;

    int m_millIndex;
    int m_groundIndex;
    int m_shadowIndex;

    // (todo) 05.X: Add light variables
    glm::vec3 m_ambientColor;
    glm::vec3 m_lightColor;
    float m_lightIntensity;
    glm::vec3 m_lightPosition;

    float m_millAmbientReflection;
    float m_groundAmbientReflection;
    float m_shadowAmbientReflection;

    float m_millDiffuseReflection;
    float m_groundDiffuseReflection;
    float m_shadowDiffuseReflection;

    float m_millSpecularReflection;
    float m_groundSpecularReflection;
    float m_shadowSpecularReflection;

    float m_millSpecularExponent;
    float m_groundSpecularExponent;
    float m_shadowSpecularExponent;
};
