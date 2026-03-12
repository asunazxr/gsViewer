R"(
#version 430

layout(location = 0) in vec2 aQuadPos;

struct Splat {
    vec3 position;
    vec4 color;
    vec3 cov3d_upper;
    vec3 cov3d_lower;
};

layout(std430, binding = 0) buffer GaussianBuffer {
    Splat gaussians[];
};

layout(std430, binding = 1) buffer IndexBuffer {
    int indices[];
};

uniform float pointScale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewportSize;

out vec4 vColor;
out vec2 vQuadPos;

void main()
{
    Splat s = gaussians[indices[gl_InstanceID]];

    vec4 cam = view * model * vec4(s.position, 1.0);
    vec4 clip = projection * cam;

    vec3 ndc = clip.xyz / clip.w;

    // Frustum culling
    if(any(greaterThan(abs(ndc), vec3(1.3))))
    {
        gl_Position = vec4(-100,-100,-100,1);
        return;
    }

    float z = -cam.z;

    float fx = projection[0][0] * viewportSize.x * 0.5;
    float fy = projection[1][1] * viewportSize.y * 0.5;

    mat3 J = mat3(
        fx/z, 0.0, -(fx * cam.x)/(z*z),
        0.0, -fy/z, (fy * cam.y)/(z*z),
        0.0, 0.0, 0.0
    );

    mat3 cov3d = mat3(
        s.cov3d_upper.x, s.cov3d_upper.y, s.cov3d_upper.z,
        s.cov3d_upper.y, s.cov3d_lower.x, s.cov3d_lower.y,
        s.cov3d_upper.z, s.cov3d_lower.y, s.cov3d_lower.z
    );

    mat3 view3 = mat3(view);

    mat3 T = transpose(view3) * J;

    mat3 cov = transpose(T) * cov3d * T;

    float a = cov[0][0];
    float b = cov[0][1];
    float c = cov[1][1];

    float mid = 0.5 * (a + c);
    float radius = length(vec2(0.5*(a-c), b));

    float lambda1 = mid + radius;
    float lambda2 = mid - radius;

    if(lambda2 < 0.0)
    {
        gl_Position = vec4(-100,-100,-100,1);
        return;
    }

    vec2 v = normalize(vec2(b, lambda1 - a));


    vec2 majorAxis = pointScale * sqrt(2.0 * lambda1) * v;
    vec2 minorAxis = pointScale * sqrt(2.0 * lambda2) * vec2(v.y, -v.x);

    vec2 center = ndc.xy;

    vec2 offset =
        aQuadPos.x * majorAxis / viewportSize * 2.0 +
        aQuadPos.y * minorAxis / viewportSize * 2.0;

    gl_Position = vec4(center + offset, 0.0, 1.0);

    vColor = s.color;
    vQuadPos = aQuadPos;
}
)"
