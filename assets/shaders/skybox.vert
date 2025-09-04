#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoord;

// refer to https://learnopengl.com/Advanced-OpenGL/Cubemaps
void main()
{
    // For skybox, the vertex coordinates corresponds to cubemap texture coordinates.
    TexCoord = aPos;

    vec4 clipPosition = projection * view * vec4(aPos, 1.0);
    gl_Position = clipPosition.xyww;
}