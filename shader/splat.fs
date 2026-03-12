R"(
#version 430

in vec4 vColor;
in vec2 vQuadPos;

out vec4 fragColor;

void main()
{
    float r2 = dot(vQuadPos, vQuadPos);

    if(r2 > 4.0)
        discard;

    float weight = exp(-r2);

    float alpha = weight * vColor.a;

    fragColor = vec4(vColor.rgb * alpha, alpha);
}
)"
