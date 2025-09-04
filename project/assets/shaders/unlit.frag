#version 330 core
layout (location = 0) out vec4 FragColor;

// get from vert shader
in vec3 FragWorldPos; 
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragTangent;

uniform vec3 cameraPosition;

uniform sampler2D DiffuseTexture;

uniform float AlphaClip;
uniform vec3 Tint;
uniform float Opacity;

struct Surface
{
    vec3 worldPos;

    vec3 diffuse;
    float alpha;
};

Surface MakeSurface()
{
    Surface surf;

    surf.worldPos = FragWorldPos;

    vec4 diffuse = texture(DiffuseTexture, TexCoord);
    surf.diffuse = diffuse.rgb;
    surf.alpha = diffuse.a;

    return surf;
}


///////////////////////////////////////////////////////////////////////////////////////////////////


void main()
{
    Surface surf = MakeSurface();

    if(surf.alpha <= AlphaClip) discard;

    surf.diffuse *= Tint;

    FragColor = vec4(surf.diffuse, surf.alpha * Opacity);
}
