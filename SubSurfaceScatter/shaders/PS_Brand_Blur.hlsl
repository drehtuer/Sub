#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

#ifndef BLURTYPE
static const float g_GaussWidth[] = {sqrt(0.0064f), sqrt(0.0516f - 0.0064), sqrt(0.2719f - 0.0516f), sqrt(2.0062f - 0.2719f)};
//static const float g_GaussWidth[] = {sqrt(0.0064f), sqrt(0.0484f - 0.0064f), sqrt(0.187f - 0.0484f), sqrt(0.567f - 0.187f), sqrt(1.99f - 0.567f), sqrt(7.41f - 1.99f)};
static const float g_GaussCurve[] = {0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f};
static const float g_maxdd = 0.001f;

float stretchX(float depth, uniform float sssLevel, uniform float correction) {
    return sssLevel / (depth + correction * min(abs(ddx(depth)), g_maxdd));
}

float stretchY(float depth, uniform float sssLevel, uniform float correction) {
    return sssLevel / (depth + correction * min(abs(ddy(depth)), g_maxdd));
}

float3 convolveX(float2 texCoords : TEXCOORD0,
                 uniform float gaussWidth,
                 uniform Texture2D Tex,
                 uniform SamplerState TexSampler,
                 float depth,
                 uniform float sssLevel,
                 uniform float correction) {

    float2 finalWidth = gaussWidth * stretchX(depth, sssLevel, correction) / g_sceneSize.x * float2(1.0f, 0.0f);
    
    float2 coords = texCoords - finalWidth;
    float3 sum = make3(0.0f);
    
    [unroll]
    for(int i=0; i<7; ++i) {
        sum += g_GaussCurve[i] * Tex.Sample(TexSampler, coords).rgb;
        coords += finalWidth / 3.0f;
    }
    return sum;
}

float3 convolveY(float2 texCoords : TEXCOORD0,
                 uniform float gaussWidth,
                 uniform Texture2D Tex,
                 uniform SamplerState TexSampler,
                 float depth,
                 uniform float sssLevel,
                 uniform float correction) {

    float2 finalWidth = gaussWidth * stretchY(depth, sssLevel, correction) / g_sceneSize.y * float2(0.0f, 1.0f);

    float2 coords = texCoords - finalWidth;
    float3 sum = make3(0.0f);
    
    [unroll]
    for(int i=0; i<7; ++i) {
        sum += g_GaussCurve[i] * Tex.Sample(TexSampler, coords).rgb;
        coords += finalWidth / 3.0f;
    }
    return sum;
}

#define BLURTYPE X
#define BLURID 1
#define BLURTEXTURE g_PostDiffuseMap
#include "PS_Jimenez_Blur.hlsl"
#undef BLURTYPE
#undef BLURID
#undef BLURTEXTURE

#define BLURTYPE X
#define BLURID 2
#define BLURTEXTURE g_BlurY1Map
#include "PS_Jimenez_Blur.hlsl"
#undef BLURTYPE
#undef BLURID
#undef BLURTEXTURE

#define BLURTYPE X
#define BLURID 3
#define BLURTEXTURE g_BlurY2Map
#include "PS_Jimenez_Blur.hlsl"
#undef BLURTYPE
#undef BLURID
#undef BLURTEXTURE

#define BLURTYPE Y
#define BLURID 1
#define BLURTEXTURE g_BlurX1Map
#include "PS_Jimenez_Blur.hlsl"
#undef BLURTYPE
#undef BLURID
#undef BLURTEXTURE

#define BLURTYPE Y
#define BLURID 2
#define BLURTEXTURE g_BlurX2Map
#include "PS_Jimenez_Blur.hlsl"
#undef BLURTYPE
#undef BLURID
#undef BLURTEXTURE

#define BLURTYPE Y
#define BLURID 3
#define BLURTEXTURE g_BlurX3Map


#endif

[earlydepthstencil]
void MERGE(Blur, MERGE(BLURTYPE, BLURID))(VS2PS psIn, out PS2OM psOut) {
    psOut.pixel.rgb = MERGE(convolve, BLURTYPE)(psIn.texCoords,
                                g_GaussWidth[BLURID],
                                BLURTEXTURE,
                                g_SamplerLinear,
                                g_DepthMap.Sample(g_SamplerPoint, psIn.texCoords).r,
                                g_stretchAlpha,
                                g_stretchBeta);
    psOut.pixel.a = 1.0f;
}
