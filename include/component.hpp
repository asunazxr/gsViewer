#pragma once
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/quaternion.hpp>

//模型变换组件
struct TransformComponent
{
    glm::vec3 position{0.0f,0.0f,0.0f};
    glm::vec3 rotation{0.0f,0.0f,0.0f};
    float scale = 1.3f;

    TransformComponent(const glm::vec3& p = glm::vec3(0.0f,0.0f,0.0f),const glm::vec3& r = glm::vec3(0.0f,180.0f,180.0f),const float& s = 1.3f){
        position = p;
        rotation = r;
        scale = s;
    }
    TransformComponent& operator&=(const TransformComponent& t){
        if(this != &t){
            position = t.position;
            rotation = t.rotation;
            scale = t.scale;
        }
        return *this;
    }
    glm::mat4 getModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        glm::quat q = glm::quat(glm::radians(rotation));
        model *= glm::mat4_cast(q);
        model = glm::translate(model, position);
        return model;
    }

    void transform(const glm::vec3& transform){ position += transform; }
    void rotate(float degrees,const glm::vec3 axis){ rotation += degrees * axis; }
    void scaling(const float& scaling){ scale = scaling; }
};