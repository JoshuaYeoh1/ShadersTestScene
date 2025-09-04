#version 330 core
layout (location = 0) out vec4 FragColor;

// get from vert shader
in vec3 FragWorldPos; 
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragTangent;
in vec4 fragPosLight;

uniform vec3 cameraPosition;

#define MAX_LIGHTS 30

uniform int NUM_DIRECTIONAL_LIGHTS;

struct DirectionalLight
{
    vec3 col;
    vec3 dir;    
};

uniform DirectionalLight DirectionalLights[MAX_LIGHTS];

uniform int NUM_POINT_LIGHTS;

struct PointLight
{
    vec3 col;
    float range;
    vec3 pos;
};

uniform PointLight PointLights[MAX_LIGHTS];

uniform int NUM_SPOT_LIGHTS;

struct SpotLight
{
    vec3 col;
    vec3 dir;
    vec3 pos;
    float range;
    vec2 angles;
};

uniform SpotLight SpotLights[MAX_LIGHTS];


uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D NormalTexture;
uniform sampler2D EmissiveTexture;
uniform sampler2D AOTexture;
uniform sampler2D shadowMap;

uniform float Shininess;
uniform float AlphaClip;

uniform vec3 Tint;
uniform float Opacity;

float square(float n)
{
    return n*n;
}

// HLSL has saturate which clamps values 0-1
// GLSL doesnt have, so make one
float clamp01(float n)
{
    return clamp(n, 0, 1);
}

struct Surface
{
    vec3 worldPos;

    vec3 diffuse;
    float alpha;
    float specular;
    vec3 normal;
    float emissive;
    float ao;

    float shininess;
};

Surface MakeSurface()
{
    Surface surf;

    surf.worldPos = FragWorldPos;

    vec4 diffuse = texture(DiffuseTexture, TexCoord);
    surf.diffuse = diffuse.rgb;
    surf.alpha = diffuse.a;
    surf.specular = texture(SpecularTexture, TexCoord).r;
    vec4 normalTex = texture(NormalTexture, TexCoord);

    vec3 normal = normalize(Normal);
    vec3 tangent = normalize(FragTangent);
    vec3 bitangent = normalize(cross(normal, tangent));

    mat3 TBN = mat3(tangent, bitangent, normal);
    surf.normal = normalize(TBN * (2.0 * normalTex.rgb - 1.0));

    surf.emissive = texture(EmissiveTexture, TexCoord).r;
    surf.ao = texture(AOTexture, TexCoord).r;

    surf.shininess = Shininess;

    return surf;
}


// make the lights
// add directional light support
// embed directional light properties in this shader
// later convert light properties from embedded to uniform variables passed from c++

///////////////////////////////////////////////////////////////////////////////////////////////////

vec3 GetAmbient(vec3 lightCol, float mult)
{
    return lightCol * mult;
}

vec3 GetDiffuse(Surface surf, vec3 lightDir, vec3 lightCol)
{
    vec3 N = surf.normal;
    vec3 L = -lightDir; // reverse the lightDir

    float diffuseEquation = max(0, dot(L, N)); // if light below surface, dont light surface
    vec3 diffuse = lightCol * diffuseEquation;

    return diffuse;
}

vec3 GetSpecularPhong(Surface surf, vec3 lightDir, vec3 lightCol)
{
    // specular phong (more accurate)
    vec3 N = surf.normal;
    vec3 L = -lightDir; // reverse the lightDir
    vec3 V = normalize(cameraPosition - surf.worldPos); // pointing to the viewer/camera
    vec3 R = reflect(-L, N);

    float phongEquation = pow( max(0, dot(V, R)) , surf.shininess); // no negative value before doing pow
    vec3 phong = lightCol * surf.specular * phongEquation;

    return phong;
}

vec3 GetSpecularBlinn(Surface surf, vec3 lightDir, vec3 lightCol)
{
    // specular blinn-phong (faster)
    vec3 N = surf.normal;
    vec3 L = -lightDir; // reverse the lightDir
    vec3 V = normalize(cameraPosition - surf.worldPos); // pointing to the viewer/camera
    vec3 H = normalize(V + L);

    float blinnEquation = pow( max(0, dot(N, H)) , surf.shininess);
    vec3 blinn = lightCol * surf.specular * blinnEquation;

    return blinn;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// SHADOW

uniform bool EnableShadow;
uniform float ShadowStrength;
uniform float ShadowBias; // 0.0005

float GetShadow(vec3 lightDir)
{
    if(!EnableShadow) return 0;
    
    float shadow = 0.0f;

    // perform perspective divide
    vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;

    if(lightCoords.z <= 1)
    {
        lightCoords = (lightCoords + 1) / 2;

        // get depth of current fragment from light's perspective
        float currentDepth = lightCoords.z;
    
        // push down the shadow map to fix acne
        float bias = max(0.025 * (1 - dot(Normal, lightDir)), ShadowBias);

        int sampleRadius = 2;

        vec2 pixelSize = 1 / textureSize(shadowMap, 0);

        for(int y = -sampleRadius; y <= sampleRadius; y++)
        {
            for(int x = -sampleRadius; x <= sampleRadius; x++)
            {
                float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x,y) * pixelSize).r;

                if(currentDepth > closestDepth + bias)
                {
                    shadow += 1;
                }
            }
        }

        shadow /= pow((sampleRadius * 2 + 1), 2);
    }

    return shadow * ShadowStrength;
}   

///////////////////////////////////////////////////////////////////////////////////////////////////

// light template

vec3 MakeLight(vec3 lightCol, vec3 lightDir, float attenuation, Surface surf)
{
    lightDir = normalize(lightDir);

    // ambient
    vec3 ambientContribution = GetAmbient(lightCol, 0.2); // 20 percent

    // diffuse
    vec3 diffuseContribution = GetDiffuse(surf, lightDir, lightCol);

    // specular, choose phong or blinn phong
    vec3 specularContribution = GetSpecularBlinn(surf, lightDir, lightCol);

    // shadow
    float shadow = GetShadow(lightDir); 

    // final result
    vec3 lightContribution = diffuseContribution * (1.0 - shadow) + ambientContribution + specularContribution;
    return lightContribution * attenuation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// directional light

vec3 MakeDirectionalLight(vec3 lightCol, vec3 lightDir, Surface surf)
{
    return MakeLight(lightCol, lightDir, 1, surf);
    // attenuation will always be 1 for directional light
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// point light

float GetAttenuationLegacy(vec3 lightPos)
{
    // legacy opengl
    float d = distance(lightPos, FragWorldPos);
    float c = 1;
    float kl = .5;
    float kq = .1;

    float attenuation = 1 / (c + kl*d + kq*d*d);
    return attenuation;
}

float GetRangeAttenuation(vec3 lightPos, float lightRange)
{
    // unity render pipeline
    float d = distance(lightPos, FragWorldPos);
    float distanceSqr = square(d);

    float inversedLightRangeSqr = lightRange;
    float distInvRangeSqr = square(distanceSqr * inversedLightRangeSqr);

    float attenuation = square(max(0, 1 - distInvRangeSqr));
    return attenuation;
}

vec3 MakePointLight(vec3 lightCol, float lightRange, vec3 lightPos, Surface surf)
{
    vec3 lightDir = normalize(surf.worldPos - lightPos);

    float attenuation = GetRangeAttenuation(lightPos, lightRange);

    return MakeLight(lightCol, lightDir, attenuation, surf);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Spot Lights have colour, position, direction, inner/outer angles, range (attenuation)

float GetSpotAttenuation(vec2 spotAngles, vec3 spotDir, vec3 lightDir)
{
    float dirCosTheta = dot(spotDir, lightDir);

    dirCosTheta *= spotAngles.x; // inner
    dirCosTheta += spotAngles.y; // outer
    dirCosTheta = clamp01(dirCosTheta);

    float spotAttenuation = square(dirCosTheta);
    return spotAttenuation;
}

vec3 MakeSpotLight(vec3 lightCol, vec3 spotDir, vec3 lightPos, float lightRange, vec2 spotAngles, Surface surf)
{
    vec3 lightDir = normalize(surf.worldPos - lightPos);

    float rangeAttenuation = GetRangeAttenuation(lightPos, lightRange);
    float spotAttenuation = GetSpotAttenuation(spotAngles, spotDir, lightDir);
    float attenuation = rangeAttenuation * spotAttenuation;

    return MakeLight(lightCol, lightDir, attenuation, surf);
}


///////////////////////////////////////////////////////////////////////////////////////////////////


void main()
{
    Surface surf = MakeSurface();

    if(surf.alpha <= AlphaClip) discard;

    vec3 directionalLightContribution = vec3(0);

    for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)
    {
        directionalLightContribution += MakeDirectionalLight(DirectionalLights[i].col, DirectionalLights[i].dir, surf);
    }

    vec3 pointLightContribution = vec3(0);

    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    {
        pointLightContribution += MakePointLight(PointLights[i].col, PointLights[i].range, PointLights[i].pos, surf);
    }

    vec3 spotLightContribution = vec3(0);

    for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
    {   
        spotLightContribution += MakeSpotLight(SpotLights[i].col, SpotLights[i].dir, SpotLights[i].pos, SpotLights[i].range, SpotLights[i].angles, surf);
    }

    vec3 nonEmissivePart = surf.diffuse * (directionalLightContribution + pointLightContribution + spotLightContribution);

    nonEmissivePart *= surf.ao;

    vec3 emissivePart = surf.diffuse * surf.emissive;

    vec3 finalCol = nonEmissivePart + emissivePart;

    finalCol *= Tint;

    FragColor = vec4(finalCol, surf.alpha * Opacity);
}
