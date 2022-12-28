#include "_Buffers.hlsl"
#include "_Connectors.hlsl"

#ifndef TEXTUREBLUR
static const float g_GaussCurve[] = {0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f};

static const float g_GaussWidth[] = {sqrt(0.0064f), sqrt(0.0484f), sqrt(0.187f), sqrt(0.567f), sqrt(1.99f), sqrt(7.41f)};
//static const float g_GaussWidth[] = {1.0f, 2.0f, 3.388f, 5.607f, 10.857f, 21.1569f};

float4 convolveX(float2 texCoords, uniform float GaussWidth, uniform Texture2D InputTex) {
    float stretch = g_StretchMap.Sample(g_SamplerPoint, texCoords).x;
    float filterWidth = GaussWidth * stretch / g_sceneSize.x;
    float2 coords = texCoords - float2(filterWidth * 3.0f, 0.0f);

    float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float antiBlurGuard = 1.0f;
    [unroll]
    for(uint i=0; i<7; ++i) {
		antiBlurGuard *= (g_DepthMap.Sample(g_SamplerPoint, coords).r ? 1.0f : 0.0f); // expensive anitblur is expensive
        sum += g_GaussCurve[i] * InputTex.Sample(g_SamplerLinear, coords);
        coords += float2(filterWidth, 0.0f);
    }
    return antiBlurGuard ? sum : InputTex.Sample(g_SamplerLinear, texCoords);
}

float4 convolveY(float2 texCoords, uniform float GaussWidth, uniform Texture2D InputTex) {
    float stretch = g_StretchMap.Sample(g_SamplerPoint, texCoords).y;
    float filterWidth = GaussWidth * stretch / g_sceneSize.y;
    float2 coords = texCoords - float2(0.0f, filterWidth * 3.0f);

    float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float antiBlurGuard = 1.0f;
    [unroll]
    for(uint i=0; i<7; ++i) {
		antiBlurGuard *= (g_DepthMap.Sample(g_SamplerPoint, coords).r ? 1.0f : 0.0f); // expensive anitblur is expensive
        sum += g_GaussCurve[i] * InputTex.Sample(g_SamplerLinear, coords);
        coords += float2(0.0f, filterWidth);
    }
    return antiBlurGuard ? sum : InputTex.Sample(g_SamplerLinear, texCoords);
}

#define TEXTUREBLUR 1
#define BLURTYPE X
#define INPUTTEX g_PostDiffuseMap
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX

#define TEXTUREBLUR 2
#define BLURTYPE X
#define INPUTTEX g_BlurY1Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 3
#define BLURTYPE X
#define INPUTTEX g_BlurY2Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 4
#define BLURTYPE X
#define INPUTTEX g_BlurY3Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 5
#define BLURTYPE X
#define INPUTTEX g_BlurY4Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 1
#define BLURTYPE Y
#define INPUTTEX g_BlurX1Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 2
#define BLURTYPE Y
#define INPUTTEX g_BlurX2Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 3
#define BLURTYPE Y
#define INPUTTEX g_BlurX3Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 4
#define BLURTYPE Y
#define INPUTTEX g_BlurX4Map
#include "PS_dEon_Blur.hlsl"
#undef TEXTUREBLUR
#undef BLURTYPE
#undef INPUTTEX


#define TEXTUREBLUR 5
#define BLURTYPE Y
#define INPUTTEX g_BlurX5Map

#endif

void MERGE(MERGE(Main, BLURTYPE), TEXTUREBLUR)(VS2PS psIn, out PS2OM psOut) {
	// FIXME
    psOut.pixel = MERGE(convolve, BLURTYPE)(psIn.texCoords, g_GaussWidth[TEXTUREBLUR-1], INPUTTEX);
}

