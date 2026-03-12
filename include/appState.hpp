#pragma once
#include "component.hpp"
#include "camera.hpp"

struct  appState
{
    Camera camera;
    TransformComponent transform;
    glm::vec4 clear_color;
    bool saveImagecurFrame;
    bool saveImageOriginal;
    int image_width;
    int image_height;
    int window_width;
    int window_height;

    appState(int iw,int ih,int ww,int wh){
        image_height = iw;
        image_height = ih;
        window_width = ww;
        window_height = wh;
        clear_color = glm::vec4(0.0f,0.0f,0.0f,0.0f);
        saveImagecurFrame= false;
        saveImageOriginal = false;
    }
};
