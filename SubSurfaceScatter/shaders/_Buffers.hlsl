#ifndef _BUFFERS_HLSL
#define _BUFFERS_HLSL

#include "_Utilities.hlsl"

// translucency light for dEon & Brand
#define TRANSLUCENCYLIGHT 0
#define ORENNAYAR 0
#define VARIANCESHADOWMAP 1
#define CBUFFER_MAX_LIGHTS 5

/*** constant buffers ***/
cbuffer cbPerCamera                    : register(b0) {
    float4x4 g_world2Camera            : packoffset(c0);
    float4x4 g_world2Camera_IT         : packoffset(c4);
    float4x4 g_camera2World            : packoffset(c8);
    float4x4 g_world2CameraProj        : packoffset(c12);
    float4x4 g_cameraProj2World        : packoffset(c16);
    float    g_cameraZNear             : packoffset(c20.x);
    float    g_cameraZFar              : packoffset(c20.y);
    float2   cameraPadding             : packoffset(c20.z);
};

cbuffer cbPerLight                     : register(b1) {
float4x4 g_world2Light[CBUFFER_MAX_LIGHTS]             : packoffset(c0);
#if CBUFFER_MAX_LIGHTS == 1
    float4x4 g_world2LightProj[CBUFFER_MAX_LIGHTS]         : packoffset(c4);
    float4x4 g_light2World[CBUFFER_MAX_LIGHTS]             : packoffset(c8);
    float4   g_lightAttenuation[CBUFFER_MAX_LIGHTS]        : packoffset(c12);
    float4   g_lightColor[CBUFFER_MAX_LIGHTS]              : packoffset(c13); // only xyz
    float4   g_lightZNearFar[CBUFFER_MAX_LIGHTS]           : packoffset(c14); // only xy
#elif CBUFFER_MAX_LIGHTS == 5
    float4x4 g_world2LightProj[CBUFFER_MAX_LIGHTS]         : packoffset(c20);
    float4x4 g_light2World[CBUFFER_MAX_LIGHTS]             : packoffset(c40);
    float4   g_lightAttenuation[CBUFFER_MAX_LIGHTS]        : packoffset(c60);
    float4   g_lightColor[CBUFFER_MAX_LIGHTS]              : packoffset(c65); // only xyz
    float4   g_lightZNearFar[CBUFFER_MAX_LIGHTS]           : packoffset(c70); // only xy
#endif
};

cbuffer cbPerModel                     : register(b2) {
    float4x4 g_object2World            : packoffset(c0);
    float4x4 g_object2World_IT         : packoffset(c4);
    float4x4 g_object2CameraProj       : packoffset(c8);
};

cbuffer cbShaderSettings               : register(b3) {
    uint     g_textureSelected         : packoffset(c0.x);
    float    g_stretchAlpha            : packoffset(c0.y);
    float    g_stretchBeta             : packoffset(c0.z);
    float    g_blurMix                 : packoffset(c0.w);
    uint     g_bloomFilter             : packoffset(c1.x);
    float    g_fresnelReflectance      : packoffset(c1.y);
    float2   g_shadowMapSize           : packoffset(c1.z);
    float    g_shadowMapEpsilon        : packoffset(c2.x);
    float    g_transmittanceScale      : packoffset(c2.y);
    float2   g_sceneSize               : packoffset(c2.z);
    float    g_gammaCorrection         : packoffset(c3.x);
    float    g_minBloom                : packoffset(c3.y);
    float    g_maxBloom                : packoffset(c3.z);
	float    g_transmittanceBlur1      : packoffset(c3.w);
	float    g_transmittanceBlur2      : packoffset(c4.x);
	float    g_transmittanceFNL        : packoffset(c4.y);
    float    g_depthPeelingEpsilon     : packoffset(c4.z);
    float    g_ShaderPadding           : packoffset(c4.w);
};


/*** textures ***/
// textures loaded from files
Texture2D g_AlbedoTex                  : register(MERGE(t, TEXTURE_ALBEDOTEX_ID));
Texture2D g_NormalTex                  : register(MERGE(t, TEXTURE_NORMALTEX_ID));
Texture2D g_SpecTex                    : register(MERGE(t, TEXTURE_SPECTEX_ID));
Texture2D g_Rho_dTex                   : register(MERGE(t, TEXTURE_RHO_DTEX_ID));
Texture2D g_GammaTex                   : register(MERGE(t, TEXTURE_GAMMATEX_ID));

// textures created once on startup
Texture2D g_TransmittanceTex           : register(MERGE(t, TEXTURE_TRANSMITTANCETEX_ID));

// shadow map
// light0: peel peel peel peel light1: peel peel peel peel light2 ....
Texture2D g_ShadowAlbedo[NUMLIGHTSOURCES] : register(MERGE(t, TEXTURE_SHADOWALBEDO0D0_ID));
Texture2D g_ShadowMap[NUMLIGHTSOURCES * NUMDEPTHPEELING]   : register(MERGE(t, TEXTURE_SHADOWMAP0D0_ID));
Texture2D g_ShadowNormal[NUMLIGHTSOURCES] : register(MERGE(t, TEXTURE_SHADOWNORMAL0D0_ID));
Texture2D g_ShadowNormal2[NUMLIGHTSOURCES]: register(MERGE(t, TEXTURE_SHADOWNORMAL20D0_ID));

Texture2D g_TSM[NUMLIGHTSOURCES]       : register(MERGE(t, TEXTURE_TSM0_ID));

// g-buffer
Texture2D g_PositionMap                : register(MERGE(t, TEXTURE_POSITIONMAP_ID));
Texture2D g_NormalMap                  : register(MERGE(t, TEXTURE_NORMALMAP_ID));
Texture2D g_VertexNormalMap            : register(MERGE(t, TEXTURE_VERTEXNORMALMAP_ID));
Texture2D g_AlbedoMap                  : register(MERGE(t, TEXTURE_ALBEDOMAP_ID));
Texture2D g_SpecMap                    : register(MERGE(t, TEXTURE_SPECMAP_ID));
Texture2D g_DepthMap                   : register(MERGE(t, TEXTURE_DEPTHMAP_ID));
//Texture2D g_MaterialMap                : register(MERGE(t, TEXTURE_MATERIALMAP_ID));
Texture2D g_TexCoordMap                : register(MERGE(t, TEXTURE_TEXCOORDMAP_ID));

// deferred lighting maps
Texture2D g_DiffuseMap                 : register(MERGE(t, TEXTURE_DIFFUSEMAP_ID));
Texture2D g_SpecularMap                : register(MERGE(t, TEXTURE_SPECULARMAP_ID));

// blur maps
//Texture2D g_BlurX0Map                  : register(MERGE(t, TEXTURE_BLURX0MAP_ID));
Texture2D g_BlurX1Map                  : register(MERGE(t, TEXTURE_BLURX1MAP_ID));
Texture2D g_BlurX2Map                  : register(MERGE(t, TEXTURE_BLURX2MAP_ID));
Texture2D g_BlurX3Map                  : register(MERGE(t, TEXTURE_BLURX3MAP_ID));
Texture2D g_BlurX4Map                  : register(MERGE(t, TEXTURE_BLURX4MAP_ID));
Texture2D g_BlurX5Map                  : register(MERGE(t, TEXTURE_BLURX5MAP_ID));
//Texture2D g_BlurY0Map                  : register(MERGE(t, TEXTURE_BLURY0MAP_ID));
Texture2D g_BlurY1Map                  : register(MERGE(t, TEXTURE_BLURY1MAP_ID));
Texture2D g_BlurY2Map                  : register(MERGE(t, TEXTURE_BLURY2MAP_ID));
Texture2D g_BlurY3Map                  : register(MERGE(t, TEXTURE_BLURY3MAP_ID));
Texture2D g_BlurY4Map                  : register(MERGE(t, TEXTURE_BLURY4MAP_ID));
Texture2D g_BlurY5Map                  : register(MERGE(t, TEXTURE_BLURY5MAP_ID));

// blur combination
Texture2D g_FinalSSSMap                : register(MERGE(t, TEXTURE_FINALSSSMAP_ID));

// post process
Texture2D g_PPBrightnessMap            : register(MERGE(t, TEXTURE_PPBRIGHTNESSMAP_ID));
Texture2D g_PPBloomXMap                : register(MERGE(t, TEXTURE_PPBLOOMXMAP_ID));
Texture2D g_PPBloomYMap                : register(MERGE(t, TEXTURE_PPBLOOMYMAP_ID));
Texture2D g_PPCombineMap               : register(MERGE(t, TEXTURE_PPCOMBINEMAP_ID));

// d'Eon
Texture2D g_StretchMap                 : register(MERGE(t, TEXTURE_STRETCHMAP_ID));

// Brand
Texture2D g_TransmittanceMap           : register(MERGE(t, TEXTURE_TRANSMITTANCEMAP_ID));
Texture2D g_TransmittanceBlurXMap      : register(MERGE(t, TEXTURE_TRANSMITTANCEBLURXMAP_ID));
Texture2D g_TransmittanceBlurYMap      : register(MERGE(t, TEXTURE_TRANSMITTANCEBLURYMAP_ID));
Texture2D g_PostDiffuseMap              : register(MERGE(t, TEXTURE_POSTDIFFUSEMAP_ID));


/*** Samplers ***/
SamplerState g_SamplerLinear           : register(MERGE(s, SAMPLER_SAMPLERLINEAR_ID));
SamplerState g_SamplerPoint            : register(MERGE(s, SAMPLER_SAMPLERPOINT_ID));

#endif
