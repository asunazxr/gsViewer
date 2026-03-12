#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "appState.hpp"
struct myImGuiData{
    bool showModelImportPopup = false;
    bool showMainMenuBar = true;
    // bool showCameraSetting = false;

};


class myImGui{
public:
    myImGui(GLFWwindow* window,const char* glsl_version);
    ~myImGui() = default;
    void render(appState& state);
private:
    void init(GLFWwindow* window,const char* glsl_version);
    myImGuiData data;
};