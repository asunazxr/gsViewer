#pragma once
#include "glad/glad.h"
#include<glm/glm/glm.hpp>
#include<glm/glm/gtc/quaternion.hpp>
#include <algorithm>
#include <thread>
#include <chrono>
#include <vector>
#include<fstream>
#include <filesystem>
#include "tinyply-3.0/source/tinyply.h"

namespace Dataset{
    const float SH_C0 = 0.28209479177387814;
    struct splatPoint{ 
        alignas(16) glm::vec3 position; 
        alignas(16) glm::vec4 color;
        alignas(16) glm::vec3 cov3d_upper;  // a, b, c cov3d_upper = (cov[0][0], cov[0][1], cov[0][2])
        alignas(16) glm::vec3 cov3d_lower;  // d, e, f cov3d_lower = (cov[1][1], cov[1][2], cov[2][2])
    };
    struct RawSplat{
        float pos[3];
        float scale[3];
        uint8_t color[4];
        uint8_t rot[4];
    };

    using SplatBuffer = std::vector<splatPoint>;

    inline void computecov3d(const glm::vec3& scale, const glm::vec4& rot, glm::vec3& upper, glm::vec3& lower){
        glm::mat3 S2(0.0f);
        S2[0][0] = scale.x * scale.x;
        S2[1][1] = scale.y * scale.y;
        S2[2][2] = scale.z * scale.z;
        glm::quat q(rot.x, rot.y, rot.z, rot.w);
        q = glm::normalize(q);
        glm::mat3 R = glm::mat3_cast(q);
        glm::mat3 cov3d = R * S2 * glm::transpose(R);
        // 提取上三角部分，
        upper = glm::vec3(cov3d[0][0], cov3d[0][1], cov3d[0][2]);  // a, b, c
        lower = glm::vec3(cov3d[1][1], cov3d[1][2], cov3d[2][2]);  // d, e, f
    }
    class Data{
        public:
            Data(const std::string& path):path_(path){ num_vertices_ = 0; image_size_[0] = 1356; image_size_[1] = 2048;}
            const SplatBuffer& buffer() const { return buffer_;}
            const int& num_vertices() const { return num_vertices_;}
            const std::vector<int>& sortedIndex() const { return sortedIndex_[activeBuffer_];}
            const std::string& get_ply_path() const { return path_; }
            const unsigned int* get_image_size() const { return image_size_; }
            //主线程排序，卡顿明显
            void sortedByDepth(const glm::mat4& view){
                for(int i = 0; i < num_vertices_; i++){
                    const auto& p = buffer_[i].position;
                    depth_[i] =view[2][0] * p.x + view[2][1] * p.y + view[2][2] * p.z + view[3][2];
                    sortedIndex_[activeBuffer_][i] = i;
                }
                std::sort(sortedIndex_[activeBuffer_].begin(), sortedIndex_[activeBuffer_].end(),[&](int a, int b){
                        return depth_[a] < depth_[b];
                });
            }
            //后台线程排序，不卡顿
            void requestSort(const glm::mat4& view) {
                if (sorting_.load()) return;
                sorting_ = true;
                glm::mat4 viewCopy = view;
                std::thread([this, viewCopy]() mutable {
                    int inactive = 1 - activeBuffer_;
                    for (int i = 0; i < num_vertices_; i++) {
                        const auto& p = buffer_[i].position;
                        depth_[i] = viewCopy[2][0]*p.x + viewCopy[2][1]*p.y + viewCopy[2][2]*p.z + viewCopy[3][2];
                        sortedIndex_[inactive][i] = i;
                    }
                    std::sort(sortedIndex_[inactive].begin(), sortedIndex_[inactive].end(),
                            [&](int a,int b){ return depth_[a] < depth_[b]; });
                    activeBuffer_ = inactive;
                    sorting_ = false;
                    std::cout << "Depth sorting completed in background thread." << std::endl;
                }).detach();
            } 
            bool load() {
                std::filesystem::path filePath(path_);
                std::string extension = filePath.extension().string();
                if(extension == ".ply"){
                    if(loadPly()){
                        std::cout << "PLY file loaded successfully: " << path_ << std::endl;
                        return true;
                    } else {
                        std::cerr << "Failed to load PLY file: " << path_ << std::endl;
                        return false;
                    }
                }else if (extension == ".splat")
                {
                    if(loadSplat()){
                        std::cout << "SPLAT file loaded successfully: " << path_ << std::endl;
                        return true;
                    } else {
                        std::cerr << "Failed to load SPLAT file: " << path_ << std::endl;
                        return false;
                    }
                }else if (extension == ".sog")
                {
                    if(loadSog()){
                        std::cout << "SOG file loaded successfully: " << path_ << std::endl;
                        return true;
                    } else {
                        std::cerr << "Failed to load SOG file: " << path_ << std::endl;
                        return false;
                    }
                }
                else {
                    std::cerr << "Unsupported file format: " << extension << std::endl;
                    return false;
                }
            }

           
        private:
            std::string path_;
            SplatBuffer buffer_;
            std::vector<float> depth_;
            std::vector<int> sortedIndex_[2];
            unsigned int image_size_[2];
            int num_vertices_;
            int activeBuffer_ = 0;
            std::atomic<bool> sorting_ = false;
            bool loadPly() {
                std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
                std::ifstream file_stream(path_, std::ios::binary);
                if (!file_stream) {
                    std::cerr << "Failed to open file: " << path_ << std::endl;
                    return false;
                }
                tinyply::PlyFile splat_ply;
                splat_ply.parse_header(file_stream);
                std::cout << "\t[ply_header] Type: " << (splat_ply.is_binary_file() ? "binary" : "ascii") << std::endl;
                for (const auto & c : splat_ply.get_comments()) std::cout << "\t[ply_header] Comment: " << c << std::endl;
                for (const auto & c : splat_ply.get_info()) std::cout << "\t[ply_header] Info: " << c << std::endl;
                for (const auto & e : splat_ply.get_elements()){
                    std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
                    for (const auto& p : e.properties) {
                        std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
                        if (p.isList) std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
                        std::cout << std::endl;
                    }
                }
                std::shared_ptr<tinyply::PlyData> xyz, f_dc,  opacity, scales, rot, image_size;
                try { 
                    xyz = splat_ply.request_properties_from_element("vertex", { "x", "y", "z" }); 
                    opacity = splat_ply.request_properties_from_element("vertex", { "opacity" });
                    scales = splat_ply.request_properties_from_element("vertex", { "scale_0", "scale_1", "scale_2" });
                    rot = splat_ply.request_properties_from_element("vertex", { "rot_0", "rot_1", "rot_2", "rot_3" });
                    f_dc = splat_ply.request_properties_from_element("vertex", {"f_dc_0", "f_dc_1", "f_dc_2"});
                    image_size = splat_ply.request_properties_from_element("image_size", {"image_size"});
                }
                catch (const std::exception & e) {
                    std::cerr << "tinyply exception: " << e.what() << std::endl; 
                    return false;
                }
                splat_ply.read(file_stream);
                if (image_size) {
                    uint32_t* size_ptr = reinterpret_cast<uint32_t*>(image_size->buffer.get());
                    image_size_[0] = std::max(1u, size_ptr[0]);
                    image_size_[1] = std::max(1u, size_ptr[1]);
                } else {
                    image_size_[0] = 1536;  // 默认值
                    image_size_[1] = 2048;  // 默认值
                }
                std::cout << "Image size from PLY: " << image_size_[0] << " x " << image_size_[1] << std::endl;
                num_vertices_ = xyz->count;
                buffer_.resize(num_vertices_);
                depth_.resize(num_vertices_);
                sortedIndex_[0].resize(num_vertices_);
                sortedIndex_[1].resize(num_vertices_);
                const float *pos_ptr = reinterpret_cast<float*>(xyz->buffer.get());
                const float *op_ptr  = reinterpret_cast<float*>(opacity->buffer.get());
                const float *sc_ptr = reinterpret_cast<float*>(scales->buffer.get());
                const float *rt_ptr = reinterpret_cast<float*>(rot->buffer.get());
                const float *dc_ptr  = reinterpret_cast<float*>(f_dc->buffer.get());
                int threadCount = std::thread::hardware_concurrency();
                int blockSize = (num_vertices_ + threadCount - 1) / threadCount;
                std::vector<std::thread> threads;

                for(int i = 0; i < threadCount; ++i){
                    int start = i * blockSize;
                    int end = std::min(start + blockSize, num_vertices_);
                    threads.emplace_back([&, start, end](){
                        for(int j = start; j < end; ++j){
                            //position(xyz)
                            buffer_[j].position = glm::vec3(pos_ptr[j * 3 + 0], pos_ptr[j * 3 + 1], pos_ptr[j * 3 + 2]);
                            //scale(xyz)
                            float scale_x = std::exp(sc_ptr[j * 3 + 0]);
                            float scale_y = std::exp(sc_ptr[j * 3 + 1]);
                            float scale_z = std::exp(sc_ptr[j * 3 + 2]);
                            glm::vec3 scale( scale_x, scale_y, scale_z);
                            //qual
                            glm::vec4 rot(rt_ptr[j * 4 + 0], rt_ptr[j * 4 + 1], rt_ptr[j * 4 + 2], rt_ptr[j * 4 + 3]);
                            //color(rgba)
                            float opacity = 1.0f / (1.0f + std::exp(-op_ptr[j]));
                            glm::vec3 dc = glm::vec3(dc_ptr[j * 3 + 0], dc_ptr[j * 3 + 1], dc_ptr[j * 3 + 2]) * SH_C0 + glm::vec3(0.5f);
                            buffer_[j].color = glm::vec4(dc,opacity);
                            computecov3d(scale, rot, buffer_[j].cov3d_upper, buffer_[j].cov3d_lower);
                        }
                    });
                }
                for(auto& t : threads) t.join();
                for(int i = 0; i < num_vertices_; ++i)
                    sortedIndex_[activeBuffer_][i] = i;
                printf("=== PLY Load Debug ===\n"); 
                printf("Successfully loaded %s (num: %zu)\n", path_.c_str(), num_vertices_);
                std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = endTime - startTime;
                printf("Time taken to load and process: %.3f seconds\n", elapsed.count());
                return true;
            }

            bool loadSplat(){
                std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
                std::ifstream infile(path_, std::ios::binary);
                if (!infile.is_open()) {
                    std::cerr << "Failed to open splat file: " << path_ << std::endl;
                    return false;
                }
                infile.seekg(0, std::ios::end);
                size_t fileSize = infile.tellg();
                infile.seekg(0, std::ios::beg);
                //每个顶点  position(float * 3),scale(float * 3),color(uint8_t * 4) rot(uint8_t * 4)
                num_vertices_ = fileSize /  (sizeof(float) * 6 + sizeof(uint8_t) * 8);
                buffer_.resize(num_vertices_);
                depth_.resize(num_vertices_);
                sortedIndex_[0].resize(num_vertices_);
                sortedIndex_[1].resize(num_vertices_);
                std::vector<char> raw(fileSize);
                infile.read(raw.data(), fileSize);
                RawSplat* splats = reinterpret_cast<RawSplat*>(raw.data());
                for(size_t i = 0; i < num_vertices_; ++i){
                    RawSplat& s = splats[i];
                    buffer_[i].position = glm::vec3(s.pos[0], s.pos[1], s.pos[2]);
                    glm::vec3 scaleVec( s.scale[0], s.scale[1], s.scale[2]);
                    float qx = (s.rot[0] - 128.0f) / 128.0f;
                    float qy = (s.rot[1] - 128.0f) / 128.0f;
                    float qz = (s.rot[2] - 128.0f) / 128.0f;
                    float qw = (s.rot[3] - 128.0f) / 128.0f;
                    glm::vec4 rotQuat(qx, qy, qz, qw);
                    buffer_[i].color = glm::vec4(s.color[0] / 255.0f, s.color[1] / 255.0f, s.color[2] / 255.0f, s.color[3] / 255.0f);
                    computecov3d(scaleVec, rotQuat, buffer_[i].cov3d_upper, buffer_[i].cov3d_lower);
                }
                for(size_t i = 0; i < num_vertices_; ++i)
                    sortedIndex_[activeBuffer_][i] = i;
                infile.close();
                printf("=== SPLAT Load Debug ===\n"); 
                printf("Successfully loaded %s (num: %zu)\n", path_.c_str(), num_vertices_);
                std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = endTime - startTime;
                printf("Time taken to load and process: %.3f seconds\n", elapsed.count());
                return true;
            }

            bool loadSog(){
                std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
                //解压.sog文件


                printf("=== SOG Load Debug ===\n"); 
                printf("Successfully loaded %s (num: %zu)\n", path_.c_str(), num_vertices_);
                std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = endTime - startTime;
                printf("Time taken to load and process: %.3f seconds\n", elapsed.count());
                return false;
            }
        };
}



