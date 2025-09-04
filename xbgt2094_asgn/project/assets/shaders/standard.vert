#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aTangent;

// send to frag shader
out vec3 FragWorldPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 FragTangent;
out vec4 fragPosLight;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float time;
uniform float BreathingSpeed;

uniform mat4 lightProjection;


float Wave(float amp, float freq, float axis, float xOffset, float yOffset)
{
    return amp * sin(freq * (axis + xOffset)) + yOffset;
}

void main()
{
	vec3 pos = aPos;
	vec3 dir = normalize(pos);

	if(BreathingSpeed > 0)
	{
		float breathAnim = Wave(0.05, BreathingSpeed, 0, time, 0);

		pos += dir * breathAnim;
	}

	vec4 worldPos = model * vec4(pos, 1.0);
	FragWorldPos = worldPos.xyz;

	// to fix normal to point at the correct direction,
	// the normal needs to be multiplied with a normal matrix
	// which is the transpose of the inverse of the upper 3x3 part of the model matrix
	mat3 normalMatrix = mat3(transpose(inverse(model)));
	Normal = normalMatrix * aNormal;
	FragTangent = normalMatrix * aTangent;

	TexCoord = aTexCoord;

	fragPosLight = lightProjection * worldPos;

	gl_Position = projection * view * worldPos;
}