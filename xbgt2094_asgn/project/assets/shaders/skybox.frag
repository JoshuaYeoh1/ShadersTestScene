#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoord;

uniform mat4 projection;
uniform samplerCube skybox;

void main()
{    
    vec3 result = texture(skybox, TexCoord).rgb;
    FragColor = texture(skybox, TexCoord);
    BrightColor = vec4(0.0);
}