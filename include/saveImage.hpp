#pragma once
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <atomic>
#include <thread>
#include <string>

class ImgSave{
public:
    static ImgSave* getInstance(){
        static ImgSave imgsave;
        return &imgsave;
    }
    void saveImage(std::string& path, unsigned char* pixels,int& width, int& height){
        if(saving_.load()) return;
        saving_ = true;
        std::string savePath = path;
        unsigned char* p = pixels;
        int w = width;
        int h =height;
        std::thread([this,savePath,p,w,h](){
            stbi_flip_vertically_on_write(true);
            stbi_write_png(savePath.c_str(), w, h, 4, p, w * 4);
            if(p)
              delete[] p;
            saving_ = false;
        }).detach();
    }
private:
    ImgSave(){}
    ~ImgSave(){}
    ImgSave(const ImgSave & )=delete;
    ImgSave& operator =(const ImgSave&)=delete;
    ImgSave(const ImgSave && )=delete;
    ImgSave& operator =(const ImgSave&&)=delete;
    std::atomic<bool> saving_ = false;
};