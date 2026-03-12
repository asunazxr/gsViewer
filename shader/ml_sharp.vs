R"(
#version 430 core
layout(location=0) in vec2 aQuadPos;
struct splatPoint{ 
    vec3 position; 
    vec4 color;
    vec3 cov3d_upper; 
    vec3 cov3d_lower; 
};
layout(std430,binding=0) buffer gaussianBuffer{
    splatPoint gaussians[];
};
layout(std430,binding=1) buffer indexBuffer{
    int index[];
};

uniform float pointScale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewportSize;

out vec2 vPosition;
out vec4 vColor;
out mat2 conic;

void main() {
    // Use view-space camera similar to the referenced method
    splatPoint point = gaussians[index[gl_InstanceID]];
    vec4 cam = view * model * vec4(point.position, 1.0);
    vec4 pos2d = projection * cam;
    pos2d.xyz = pos2d.xyz / pos2d.w;
    //视锥剔除
    if(any(greaterThan(abs(pos2d.xyz),vec3(1.3)))){
        gl_Position = vec4(-100,-100,-100,-1);
        return;
    }

    mat3 cov3d = mat3(
        point.cov3d_upper.x, point.cov3d_upper.y, point.cov3d_upper.z,
        point.cov3d_upper.y, point.cov3d_lower.x, point.cov3d_lower.y,
        point.cov3d_upper.z, point.cov3d_lower.y, point.cov3d_lower.z
    );
    //计算三维雅可比矩阵
    float z = cam.z;
    float fx = projection[0][0] * viewportSize.x * 0.5;
    float fy= projection[1][1] * viewportSize.y * 0.5;
    mat3 J =  mat3(
        fx / z,   0.0, -fx * cam.x / (z * z),
        0,     -fy / z, fy * cam.y / (z * z),
        0,     0,       0
    );
    //计算二维协方差
    mat3 W = mat3(view);
    mat3 T = W*J;
    mat3 cov2d_3 = transpose(T) * cov3d * T;
    mat2 cov2d = mat2(cov2d_3);
    cov2d[0][0] += 0.3;
    cov2d[1][1] += 0.3;

    //二维协方差求逆，用于计算权重
    conic = inverse(cov2d);


    //将四边形中心移动到高斯点，此时高斯点在裁剪空间
    //根据协方差计算四边形大小
    vec2 quadSize = pointScale * 1.2 * vec2(sqrt(cov2d[0][0]),sqrt(cov2d[1][1]));
    //高斯点在屏幕坐标
    vec2 guassCenter = (vec2(pos2d) + 1.0) * 0.5 *viewportSize;


    vec2 vertexPos = guassCenter + aQuadPos * quadSize;
    gl_Position = vec4((vertexPos / viewportSize) * 2.0 - 1.0, pos2d.z, 1.0);

    vColor = point.color;
    vPosition = aQuadPos;
    vPosition = vertexPos - guassCenter;
}
)"