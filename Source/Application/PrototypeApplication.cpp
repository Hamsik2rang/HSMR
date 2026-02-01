//
//  PrototypeApplication.cpp
//  HSMR
//
//  Lightweight prototyping application for rendering techniques
//
#include "PrototypeApplication.h"
#include "Core/Profiler/Profiler.h"
#include "GizmoController.h"
//#include "Editor/Resource/ShaderWatcher.h"

#include "Core/Log.h"
#include "Core/HAL/Timer.h"
#include "Engine/Window.h"
#include "RHI/RHIContext.h"
#include "RHI/Swapchain.h"
#include "Engine/Renderer/RenderPath.h"

#include "imgui.h"

#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

HS_NS_BEGIN

PrototypeApplication::PrototypeApplication()
{
}

PrototypeApplication::~PrototypeApplication()
{
    Shutdown();
}

bool PrototypeApplication::Init(const char* configPath)
{
    // Load configuration
    if (configPath && !loadConfig(configPath))
    {
        HS_LOG(warning, "Failed to load config, using defaults");
    }

    // Create window
    if (!createWindow())
    {
        HS_LOG(error, "Failed to create window");
        return false;
    }

    // Initialize RHI
    if (!initRHI())
    {
        HS_LOG(error, "Failed to initialize RHI");
        return false;
    }

    // Initialize ImGui
    if (!initImGui())
    {
        HS_LOG(error, "Failed to initialize ImGui");
        return false;
    }

    // Create camera
    _camera = MakeScoped<Camera>();
    _camera->SetAspectRatio(
        static_cast<float>(_config.windowWidth) /
        static_cast<float>(_config.windowHeight)
    );
    _camera->Update();

    // Create scene
    _scene = MakeScoped<Scene>();

    // Create mouse picker
    _picker = MakeScoped<MousePicker>();
    _picker->SetScreenSize(_config.windowWidth, _config.windowHeight);

    // Create profiler
//    _profiler = MakeScoped<Profiler>();

    // Create gizmo controller
    _gizmo = MakeScoped<GizmoController>();

    // Create shader watcher
    //_shaderWatcher = MakeScoped<ShaderWatcher>();

    // Initialize timer
    Timer::Start();
    _lastFrameTime = Timer::GetElapsedMilliseconds();

    // Call derived class initialization
    OnInit();

    // Load default scene if specified
    if (!_config.defaultScene.empty())
    {
        LoadScene(_config.defaultScene.c_str());
    }

    _isRunning = true;
    HS_LOG(info, "PrototypeApplication initialized successfully");
    return true;
}

bool PrototypeApplication::LoadScene(const char* scenePath)
{
    if (!_scene) return false;

    if (!_scene->LoadFromJSON(scenePath))
    {
        HS_LOG(error, "Failed to load scene: %s", scenePath);
        return false;
    }

    // Apply camera configuration from scene
    _scene->ApplyCameraConfig(_camera.get());

    // Reset orbit target to scene center
    if (!_scene->GetObjects().empty())
    {
        // Find center of all objects
        glm::vec3 center(0.0f);
        for (auto& obj : _scene->GetObjects())
        {
            center += obj.GetPosition();
        }
        center /= static_cast<float>(_scene->GetObjects().size());
        _orbitTarget = center;
    }

    // Notify derived class
    OnSceneLoaded(_scene.get());

    HS_LOG(info, "Scene loaded: %s", scenePath);
    return true;
}

void PrototypeApplication::Run()
{
    if (!_isRunning || !_window) return;

    while (_window->IsOpened())
    {
        // Process window events
        _window->ProcessEvent();

        if (!_window->IsOpened()) break;

        // Calculate delta time
        float currentTime = Timer::GetElapsedMilliseconds();
        _deltaTime = (currentTime - _lastFrameTime) / 1000.0f;
        _lastFrameTime = currentTime;
        _totalTime += _deltaTime;
        _frameCount++;

        // Skip frame if minimized
        if (_window->IsMinimize())
        {
            continue;
        }

        // Begin profiling frame
//        if (_profiler)
//        {
//            _profiler->BeginFrame();
//        }

        // Update input
        updateInput();

        // Update camera
        updateCamera(_deltaTime);

        // Update gizmo
        if (_gizmo && _showGizmo)
        {
            _gizmo->ProcessInput();

            if (SceneObject* selected = _scene ? _scene->GetSelectedObject() : nullptr)
            {
                _gizmo->Manipulate(_camera.get(), selected);
            }
        }

        // Update shader watcher
//        if (_shaderWatcher)
//        {
//            _shaderWatcher->Update(_deltaTime);
//        }

        // Call derived class update
        OnUpdate(_deltaTime);

        // Render
        _window->NextFrame();

//        if (_profiler)
//        {
//            _profiler->BeginGPUTimer("Render");
//        }

        _window->Render();

        // Call derived class render
        OnRender();

//        if (_profiler)
//        {
//            _profiler->EndGPUTimer("Render");
//        }

        // Render GUI
        renderGUI();

        // Present
        _window->Present();

        // End profiling frame
//        if (_profiler)
//        {
//            _profiler->EndFrame();
//        }
    }

    Shutdown();
}

void PrototypeApplication::Shutdown()
{
    if (!_isRunning) return;

    _isRunning = false;

    // Call derived class shutdown
    OnShutdown();

    // Cleanup components
//    _shaderWatcher.reset();
    _gizmo.reset();
//    _profiler.reset();
    _picker.reset();
    _scene.reset();
    _camera.reset();
    _renderer.reset();

    // Cleanup RHI
    if (_rhiContext)
    {
        _rhiContext->WaitForIdle();

        if (_swapchain)
        {
            _rhiContext->DestroySwapchain(_swapchain);
            _swapchain = nullptr;
        }

        _rhiContext->Finalize();
        _rhiContext = nullptr;
    }

    // Cleanup window
    if (_window)
    {
        _window->Shutdown();
        delete _window;
        _window = nullptr;
    }

    HS_LOG(info, "PrototypeApplication shutdown complete");
}

void PrototypeApplication::OnMouseClick(float x, float y, int button)
{
    if (button >= 0 && button < 3)
    {
        _mouseButtons[button] = true;
    }

    // Left click - object selection
    if (button == 0 && _scene && _picker)
    {
        // Check if mouse is over ImGui
        if (ImGui::GetIO().WantCaptureMouse)
        {
            return;
        }

        // Pick object
        SceneObject* picked = _picker->PickObject(_camera.get(), _scene.get(), x, y);
        _scene->SetSelectedObject(picked);

        if (picked)
        {
            HS_LOG(info, "Selected: %s", picked->GetName().c_str());
            _orbitTarget = picked->GetPosition();
        }
    }

    // Right click - start orbiting
    if (button == 1)
    {
        _isOrbiting = true;
    }

    // Middle click - start panning
    if (button == 2)
    {
        _isPanning = true;
    }

    _lastMouseX = x;
    _lastMouseY = y;
}

void PrototypeApplication::OnMouseMove(float x, float y, float dx, float dy)
{
    // Check if mouse is over ImGui
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (_isOrbiting && _camera)
    {
        // Orbit camera around target
        float sensitivity = 0.005f;
        _camera->Orbit(-dx * sensitivity, -dy * sensitivity, _orbitTarget);
    }

    if (_isPanning && _camera)
    {
        // Pan camera
        float sensitivity = 0.01f;
        glm::vec3 right = _camera->GetRight();
        glm::vec3 up = _camera->GetUp();
        glm::vec3 offset = right * (-dx * sensitivity) + up * (dy * sensitivity);
        _camera->Move(offset);
        _orbitTarget += offset;
    }

    _lastMouseX = x;
    _lastMouseY = y;
}

void PrototypeApplication::OnMouseScroll(float delta)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (_camera)
    {
        // Dolly camera
        _camera->Dolly(-delta * 0.5f, _orbitTarget);
    }
}

void PrototypeApplication::OnKeyDown(int key)
{
    if (key >= 0 && key < 256)
    {
        _keys[key] = true;
    }

    // Gizmo mode shortcuts
    if (_gizmo)
    {
        if (key == 'W' || key == 'w')
        {
            _gizmo->SetMode(GizmoController::Mode::Translate);
        }
        else if (key == 'E' || key == 'e')
        {
            _gizmo->SetMode(GizmoController::Mode::Rotate);
        }
        else if (key == 'R' || key == 'r')
        {
            _gizmo->SetMode(GizmoController::Mode::Scale);
        }
    }

    // Toggle UI visibility
    if (key == 'P' || key == 'p')
    {
        _showProfiler = !_showProfiler;
    }
    if (key == 'G' || key == 'g')
    {
        _showGizmo = !_showGizmo;
    }
}

void PrototypeApplication::OnKeyUp(int key)
{
    if (key >= 0 && key < 256)
    {
        _keys[key] = false;
    }

    // Release orbit/pan on mouse button release
    // (This should be in mouse up handler, simplified here)
}

void PrototypeApplication::updateInput()
{
    // Update mouse button states (release detection)
    ImGuiIO& io = ImGui::GetIO();

    if (!io.MouseDown[1])
    {
        _isOrbiting = false;
    }
    if (!io.MouseDown[2])
    {
        _isPanning = false;
    }
}

void PrototypeApplication::updateCamera(float deltaTime)
{
    if (!_camera) return;

    // WASD camera movement
    glm::vec3 movement(0.0f);

    if (_keys['W'] || _keys['w'])
    {
        movement += _camera->GetForward();
    }
    if (_keys['S'] || _keys['s'])
    {
        movement -= _camera->GetForward();
    }
    if (_keys['A'] || _keys['a'])
    {
        movement -= _camera->GetRight();
    }
    if (_keys['D'] || _keys['d'])
    {
        movement += _camera->GetRight();
    }
    if (_keys['Q'] || _keys['q'])
    {
        movement -= _camera->GetUp();
    }
    if (_keys['E'] || _keys['e'])
    {
        movement += _camera->GetUp();
    }

    if (glm::length(movement) > 0.0f)
    {
        movement = glm::normalize(movement) * _cameraSpeed * deltaTime;
        _camera->Move(movement);
        _orbitTarget += movement;
    }

    _camera->Update();
}

void PrototypeApplication::renderGUI()
{
    // Start ImGui frame
    ImGui::NewFrame();

    // Profiler window
//    if (_showProfiler && _profiler)
//    {
//        _profiler->DrawUI();
//    }

    // Scene hierarchy window
    if (_showHierarchy && _scene)
    {
        if (ImGui::Begin("Hierarchy", &_showHierarchy))
        {
            for (size_t i = 0; i < _scene->GetObjectCount(); ++i)
            {
                SceneObject* obj = _scene->GetObject(i);
                if (!obj) continue;

                bool isSelected = (_scene->GetSelectedObject() == obj);
                if (ImGui::Selectable(obj->GetName().c_str(), isSelected))
                {
                    _scene->SetSelectedObject(obj);
                    _orbitTarget = obj->GetPosition();
                }
            }
        }
        ImGui::End();
    }

    // Inspector window
    if (_showInspector && _scene)
    {
        if (ImGui::Begin("Inspector", &_showInspector))
        {
            SceneObject* selected = _scene->GetSelectedObject();
            if (selected)
            {
                ImGui::Text("Name: %s", selected->GetName().c_str());
                ImGui::Separator();

                // Transform
                glm::vec3 pos = selected->GetPosition();
                if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                {
                    selected->SetPosition(pos);
                }

                glm::vec3 rot = glm::degrees(selected->GetRotation());
                if (ImGui::DragFloat3("Rotation", &rot.x, 1.0f))
                {
                    selected->SetRotation(glm::radians(rot));
                }

                glm::vec3 scale = selected->GetScale();
                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                {
                    selected->SetScale(scale);
                }

                ImGui::Separator();

                // Material
                glm::vec4 baseColor = selected->GetBaseColor();
                if (ImGui::ColorEdit4("Base Color", &baseColor.x))
                {
                    selected->SetBaseColor(baseColor);
                }

                float metallic = selected->GetMetallic();
                if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f))
                {
                    selected->SetMetallic(metallic);
                }

                float roughness = selected->GetRoughness();
                if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f))
                {
                    selected->SetRoughness(roughness);
                }
            }
            else
            {
                ImGui::Text("No object selected");
            }
        }
        ImGui::End();
    }

    // Call derived class GUI
    OnGUI();

    // Render ImGui
    ImGui::Render();
}

bool PrototypeApplication::loadConfig(const char* configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        return false;
    }

    json configJson;
    try
    {
        file >> configJson;
    }
    catch (const json::parse_error& e)
    {
        HS_LOG(error, "Config parse error: %s", e.what());
        return false;
    }
    file.close();

    // Parse configuration
    if (configJson.contains("window"))
    {
        const auto& window = configJson["window"];

        if (window.contains("title"))
        {
            _config.windowTitle = window["title"].get<std::string>();
        }
        if (window.contains("width"))
        {
            _config.windowWidth = window["width"].get<uint32>();
        }
        if (window.contains("height"))
        {
            _config.windowHeight = window["height"].get<uint32>();
        }
        if (window.contains("vsync"))
        {
            _config.vsync = window["vsync"].get<bool>();
        }
        if (window.contains("fullscreen"))
        {
            _config.fullscreen = window["fullscreen"].get<bool>();
        }
    }

    if (configJson.contains("defaultScene"))
    {
        _config.defaultScene = configJson["defaultScene"].get<std::string>();
    }

    if (configJson.contains("assetPath"))
    {
        _config.assetPath = configJson["assetPath"].get<std::string>();
    }

    if (configJson.contains("shaderPath"))
    {
        _config.shaderPath = configJson["shaderPath"].get<std::string>();
    }

    return true;
}

bool PrototypeApplication::createWindow()
{
    // Window creation is platform-specific
    // This is a placeholder - actual implementation depends on Engine::Window

    // The actual window will be created by the main function
    // which passes control to this application

    return true;
}

bool PrototypeApplication::initRHI()
{
    // RHI initialization is handled by Window
    // This is a placeholder for additional RHI setup

    return true;
}

bool PrototypeApplication::initImGui()
{
    // ImGui initialization is handled by the backend
    // (ImGuiExtension::InitializeBackend)

    // Configure ImGui style
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Set dark theme
    ImGui::StyleColorsDark();

    return true;
}

HS_NS_END
