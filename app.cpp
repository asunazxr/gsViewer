#include<glad/glad.h>
#include<glfw/glfw3.h>
#include<iostream>
#include "shader.hpp"
#include "3dgs.hpp"
#include "gui.hpp"
#include "saveImage.hpp"
#include "appState.hpp"
static const char* VERTEX_SHADER_SOURCE =
#include "shader/splat.vs"
;
static const char* FRAGMENT_SHADER_SOURCE =
#include "shader/splat.fs"
;
//------- 函数声明 ----------------------------------------------------------------------//
static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void framebuffer_size_callback(GLFWwindow* window, int w, int h);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
int initGLFW();
void processKey(GLFWwindow* window,float deltaTime);
void processMouse(GLFWwindow* window);
void saveOriginalImage(gs& g, Shader& shader);
void saveCurFrameAsImage(int width,int height);
void computeCenteredViewport(int origW, int origH, int& vpX, int& vpY, int& vpW, int& vpH);

//------- 全局变量 窗口参数 --------------------------------------------------------------//
GLFWwindow* window;

int width = 1200;
int height = 600;
const char* title = "gsRender<ml_sharp>";
//-------- 控制相机和鼠标参数 -------------------------------------------------------------//
float deltaTime = 0.0f;
float scroll = 0.0f;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
bool firstMouse = true;

appState state(1536,2048,width,height);


int main(int argc, char* argv[]){
    //------- 解析参数 ------------------------------------------------------------------//
    std::cout<<"Hello gsRender!"<<std::endl;
    std::cout<<"GLFW version:"<<glfwGetVersionString()<<std::endl;
    std::cout<<"argc number:"<<argc<<std::endl;
    for(int i = 0; i < argc; i++){
        std::cout<<"argv["<<i<<"]:"<<argv[i]<<std::endl;
    }
    if(argc < 2){
        std::cout<<"input model path is missing"<<std::endl;
        return -1;
    }
    const std::string input = argv[1];
    //------- 初始化GLFW ----------------------------------------------------------------//
    if(initGLFW() != 0){
        std::cout<<"Failed to initialize GLFW"<<std::endl;
        return -1;
    }
    //------- 加载数据 ------------------------------------------------------------------//
    std::shared_ptr<Dataset::Data> data = std::make_shared<Dataset::Data>(input);
    if(!data->load()){
        std::cout<<"Failed to load data"<<std::endl;
        return -1;
    }
    state.image_width = data->get_image_size()[0];
    state.image_height = data->get_image_size()[1];
    //------- 初始化GUI，上传数据，初始化shader -----------------------------------------//
    myImGui gui(window,"#version 430");
    gs g(data);
    Shader shader(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE, false);
     //------- 渲染 -------------------------------------------------------------------//
    float curFrame = 0.0f;
    float lastFrame = (float)glfwGetTime();
    int vpX, vpY, vpW, vpH;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(state.clear_color.r,state.clear_color.g,state.clear_color.b,state.clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        //交互
        curFrame = (float)glfwGetTime();
        deltaTime = curFrame - lastFrame;
        lastFrame = curFrame;
        processKey(window,deltaTime);
        processMouse(window);
        //渲染视口自适应窗口
        computeCenteredViewport(state.image_width, state.image_height, vpX, vpY, vpW, vpH);
        glViewport(vpX, vpY, vpW, vpH);
        // glfwGetWindowSize(window, &width, &height);
        // state.window_width = width;
        // state.window_height = height;
        state.camera.setPerspective(state.image_width, state.image_height);
        shader.use();
        shader.setFloat("pointScale", state.transform.scale);
        shader.setVec2("viewportSize",glm::vec2(vpW,vpH));
        shader.setMat4("model", state.transform.getModelMatrix());
        shader.setMat4("view", state.camera.getViewMatrix());
        shader.setMat4("projection", state.camera.getPerspective());

        if (state.camera.hasChanged()) {
            g.sortbyDepth(state.camera.getViewMatrix());
        }
        g.Draw();
        
        if(state.saveImageOriginal){
            saveOriginalImage(g, shader);
            state.saveImageOriginal = false;
        }
        if(state.saveImagecurFrame){
            saveCurFrameAsImage(width,height);
            state.saveImagecurFrame = false;
        }
        //gui.render(state.transform,state.camera,state.clear_color,state.saveImageOriginal);
        gui.render(state);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
static void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    //glViewport(0, 0, width, height);
    state.window_width = w;
    state.window_height = h;
    width = w;
    height = h;
}
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    //std::cout<<"xpos:"<<xpos<<" ypos:"<<ypos<<std::endl;
}
int initGLFW(){
    glfwSetErrorCallback(error_callback);
    if(!glfwInit()){
        glfwTerminate();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    window = glfwCreateWindow(width, height, title, NULL, NULL);
   
    if(!window){
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout<<"Failed to initialize GLAD"<<std::endl;
        glfwTerminate();
        return -1;
    }
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(
        GL_ONE_MINUS_DST_ALPHA,
        GL_ONE,
        GL_ONE_MINUS_DST_ALPHA,
        GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    
    return 0;
}
void processKey(GLFWwindow* window,float deltaTime){
    if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS){
        state.camera.ProcessKeyBoard(FORWARD,deltaTime);
    }
    if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS){
        state.camera.ProcessKeyBoard(BACKWARD,deltaTime);
    }
    if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS){
        state.camera.ProcessKeyBoard(LEFT,deltaTime);
    }
    if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS){
        state.camera.ProcessKeyBoard(RIGHT,deltaTime);
    }
}
void processMouse(GLFWwindow* window){
    // Skip camera movement if ImGui wants to capture mouse
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
        return;
    }
    
    if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        double xoffset = xpos - lastMouseX;
        double yoffset = lastMouseY - ypos;  // reversed: y increases downward in screen space
        state.camera.ProcessMouseMovement(xoffset, yoffset);
    }
    
    lastMouseX = xpos;
    lastMouseY = ypos;
    
}
void saveCurFrameAsImage(int width,int height){
    unsigned char* pixels = new unsigned char[width * height * 4];
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    ImgSave::getInstance()->saveImage(std::string("111.png"),pixels,width,height);
}
void saveOriginalImage(gs& g, Shader& shader)
{
    GLuint fbo, fboColorTex, fboDepthRbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &fboColorTex);
    glBindTexture(GL_TEXTURE_2D, fboColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, state.image_width, state.image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorTex, 0);

    glGenRenderbuffers(1, &fboDepthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fboDepthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, state.image_width, state.image_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboDepthRbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout<<"FBO not complete!"<<std::endl;
    }

    // 绑定 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0,state.image_width, state.image_height);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    state.camera.setPerspective(state.image_width, state.image_height);
    shader.use();
    shader.setFloat("pointScale", state.transform.scale);
    shader.setVec2("viewportSize", glm::vec2(state.image_width, state.image_height));
    shader.setMat4("model", state.transform.getModelMatrix());
    shader.setMat4("view", state.camera.getViewMatrix());
    shader.setMat4("projection", state.camera.getPerspective());
    g.Draw();
    // 读取像素
    unsigned char* pixels = new unsigned char[state.image_width * state.image_height * 4];
    glReadPixels(0, 0, state.image_width, state.image_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    std::string path = "11.png";
    ImgSave::getInstance()->saveImage(path,pixels,state.image_width, state.image_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fboColorTex);
    glDeleteRenderbuffers(1, &fboDepthRbo);
    glViewport(0, 0, width, height); // 恢复窗口视口
}
void computeCenteredViewport(int origW, int origH, int& vpX, int& vpY, int& vpW, int& vpH){
    float windowAspect = (float)width / height;
    float imageAspect = (float)origW / origH;
    if (windowAspect > imageAspect) {
        vpH = height;
        vpW = static_cast<int>(height * imageAspect);
        vpX = (width - vpW) / 2;
        vpY = 0;
    } else {
        vpW = width;
        vpH = static_cast<int>(width / imageAspect);
        vpX = 0;
        vpY = (height - vpH) / 2;
    }
}