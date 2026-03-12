R"(
#version 430 core

in vec4 vColor;
in vec2 vPosition;
in mat2 conic;
out vec4 fragColor;

void main () {
    float c = dot(vPosition,conic * vPosition);
    //float c = dot(vPosition,vPosition);
    if(c > 1) discard;
    float power = -0.5 * c;
    
    float a = vColor.a * exp(power);
    if(a < 0.004) discard;
    fragColor = vec4(vColor.rgb*a , a);
}
)"