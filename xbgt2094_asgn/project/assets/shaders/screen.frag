#version 330 core
layout (location = 0) out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D mainTex;
uniform sampler2D scanline;

uniform float time;


vec4 Invert(vec4 layer)
{
    return vec4(1 - layer.rgb, layer.a);
}

vec4 Grayscale(vec4 layer)
{
    float sum = layer.r * 0.299 + layer.g * 0.587 + layer.b * 0.114;

    return vec4(vec3(sum), layer.a);
}

vec4 Mask(vec4 layer, vec4 mask)
{
    return vec4(layer.rgb, mask.r);
}

vec4 Blend(vec4 layer1, vec4 layer2)
{
    vec3 col = (layer2.rgb * layer2.a) + (layer1.rgb * (1.0 - layer2.a));

    return vec4(col, 1);
}

vec2 Tile(vec2 uv, vec2 tiling, vec2 offset)
{
    return uv * tiling + offset;
}

vec2 Spherize(vec2 uv, vec2 center, float strength, vec2 offset)
{
    vec2 delta = uv;
    delta -= center;

    float dotP = dot(delta, delta);
    float dotPow2 = dotP * dotP;

    float deltaOffset = dotPow2 * strength;

    vec2 resultuv = uv;
    resultuv += delta * deltaOffset;
    resultuv += offset;

    return resultuv;
}

vec2 Zoom(vec2 uv, vec2 center, vec2 scale)
{
    vec2 resultuv = uv;
    resultuv -= center;
    resultuv *= scale;
    resultuv += center;
    return resultuv;
}
vec2 Zoom(vec2 uv, vec2 center, float scale)
{
    return Zoom(uv, center, vec2(scale));
}

// Created by Ippokratis in 2016-05-21 (Edited)
vec4 GetVignette(vec2 uv, float strength, vec3 color)
{
    uv *=  1.0 - uv.yx;
    
    float vigPow = uv.x*uv.y * strength; // multiply with strength for intensity
    
    vigPow = pow(vigPow, 0.25); // change pow for modifying the extend of the vignette

    vec4 vig = vec4(vigPow); // the vignette is transparent but the background is opaque white

    vig.a = 1 - vig.a; // invert the opaque and transparent parts

    vec4 coloredVig = vec4(color, vig.a); // colour the opaque part
    
    return coloredVig;
}

vec4 ApplyVignette(vec4 toLayer, vec2 uv, float strength, vec3 color)
{
    vec4 layerVig = GetVignette(uv, strength, color);

    return Blend(toLayer, layerVig);
}


uniform bool EnablePostProcess;

uniform bool EnableSpherize;
uniform float SpherizeStrength; //2
uniform float ZoomScale; //.75

vec2 TryGetUVSpherize(vec2 inputUV)
{
    if(!EnablePostProcess) return inputUV;
    if(!EnableSpherize) return inputUV;

    vec2 uvSpherize = Spherize(inputUV, vec2(0.5), SpherizeStrength, vec2(0));

    uvSpherize = Zoom(uvSpherize, vec2(0.5), ZoomScale);

    return fract(uvSpherize);
}


uniform bool EnableGrayscale;

vec4 TryGrayscale(vec4 inputLayer)
{
    if(!EnablePostProcess) return inputLayer;
    if(!EnableGrayscale) return inputLayer;
    
    return Grayscale(inputLayer);
}


uniform bool EnableScanlines;
uniform int ScanlineTiling; //3
uniform float ScanlineScroll; //0.1

vec4 GetScanlines()
{
    vec2 trySpherizedUV = TryGetUVSpherize(TexCoord);

    vec2 uvScanline = Tile(trySpherizedUV, vec2(ScanlineTiling), vec2(0, time * ScanlineScroll));

    vec4 layerScanline = texture(scanline, uvScanline);

    // remove non black lines
    layerScanline = layerScanline.rgb == vec3(0) ? layerScanline : vec4(layerScanline.rgb, 0);

    return layerScanline;
}

vec4 TryScanlines(vec4 inputLayer)
{
    if(!EnablePostProcess) return inputLayer;
    if(!EnableScanlines) return inputLayer;

    vec4 blendScanline = Blend(inputLayer, GetScanlines());

    return blendScanline;
}


uniform bool EnableVignette;
uniform float VignetteStrength; //15
uniform vec3 VignetteColor; //black

vec4 TryVignette(vec4 inputLayer)
{
    if(!EnablePostProcess) return inputLayer;
    if(!EnableVignette) return inputLayer;

    return ApplyVignette(inputLayer, TexCoord, VignetteStrength, VignetteColor);
}


void main()
{
    vec4 layerBase = texture(mainTex, TryGetUVSpherize(TexCoord));

    vec4 pass1 = TryGrayscale(layerBase);

    vec4 pass2 = TryScanlines(pass1);

    vec4 pass3 = TryVignette(pass2);

    FragColor = pass3;
}