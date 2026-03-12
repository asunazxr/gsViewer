#include<gui.hpp>
#include<string>

myImGui::myImGui(GLFWwindow* window,const char* glsl_version){
    init(window,glsl_version);
}

void myImGui::init(GLFWwindow* window,const char* glsl_version){
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL( window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init(glsl_version);

}
void myImGui::render(appState& state){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //场景主窗口
    ImGui::Begin(" ");
    if(ImGui::TreeNode("scene setting")){
        //set clear_color
        ImGui::Separator();
        ImGui::ColorEdit4("clear color",(float*)&state.clear_color);
        ImGui::Separator();
        ImGui::Text("image size");
        ImGui::DragInt("width",(int*)&state.image_width,1,480,4096);
        ImGui::DragInt("height",(int*)&state.image_height,1,480,4096);
        ImGui::Separator();
        if(ImGui::Button("Save Original Image")){
            state.saveImageOriginal = true;
        }
        ImGui::Separator();
        if(ImGui::Button("Save current frame")){
            state.saveImagecurFrame = true;
        }
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Gaussian model(ml_sharp)")){
        ImGui::Separator();
        ImGui::Text("Translate:");
        ImGui::DragFloat3("position",(float*)&state.transform.position,0.01);
        ImGui::DragFloat3("rotation",(float*)&state.transform.rotation,0.1f,-360.0f,360.0f);
        ImGui::DragFloat("size",(float*)&state.transform.scale,0.01,0.01f,100.0f);
        ImGui::Separator();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("camera setting")){
        ImGui::Separator();
        ImGui::Text("Camera:");
        ImGui::DragFloat3("position",(float*)&state.camera.Position,0.01);
        ImGui::DragFloat("Zoom",(float*)&state.camera.Zoom,1.0f,1.0f,89.0f);
        ImGui::DragFloat("MovementSpeed",(float*)&state.camera.MovementSpeed,0.1f,0.1f,10.0f);
        ImGui::DragFloat("MouseSentitivity",(float*)&state.camera.MouseSentitivity,0.01f,0.01f,0.2f);
        ImGui::DragFloat("near",(float*)&state.camera.near,0.01f,0.01f,200.0f);
        ImGui::DragFloat("far",(float*)&state.camera.far,0.01f,1.0f,200.0f);
        ImGui::Separator();
        ImGui::Text("Camera Control:");
        if(ImGui::Button("Reset Camera")){
            state.camera.reset();
        }
        ImGui::Separator();
        ImGui::TreePop();
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}