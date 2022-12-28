#ifndef PS_HABLE_COMBINE_HLSL
#define PS_HABLE_COMBINE_HLSL

#include "_Connectors.hlsl"
#include "_Utilities.hlsl"
#include "_Buffers.hlsl"
#include "_Lighting.hlsl"

#define TRANSLUCENCYLIGHT 1

static const float3 g_blurJitteredWeights[13] = {
	{ 0.220441,  0.437000,  0.635000},
	{ 0.076356,  0.064487,  0.039097},
	{ 0.116515,  0.103222,  0.064912},
	{ 0.064844,  0.086388,  0.062272},
	{ 0.131798,  0.151695,  0.103676},
	{ 0.025690,  0.042728,  0.033003},
	{ 0.048593,  0.064740,  0.046131},
	{ 0.048092,  0.003042,  0.000400},
	{ 0.048845,  0.005406,  0.001222},
	{ 0.051322,  0.006034,  0.001420},
	{ 0.061428,  0.009152,  0.002511},
	{ 0.030936,  0.002868,  0.000652},
	{ 0.073580,  0.023239,  0.009703},	
};

static const float2 g_blurJitteredSamples[13] = {
	{ 0.000000,  0.000000}, // center
	{ 1.633992,  0.036795},
	{ 0.177801,  1.717592},
	{-0.194906,  0.091094},
	{-0.239737, -0.220217},
	{-0.003530, -0.118219},
	{ 1.320107, -0.181542},
	{ 5.970690,  0.253378},
	{-1.089250,  4.958349},
	{-4.015465,  4.156699},
	{-4.063099, -4.110150},
	{-0.638605, -6.297663},
	{ 2.542348, -3.245901},
};

void Main(VS2PS psIn, out PS2OM psOut) {
    float3 N;
	GETNORMAL(N);
	float3 Albedo = UNGAMMA(g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords).xyz);
    float rho_s, m;
    getSurfaceParamsTS(psIn.texCoords, rho_s, m);
    float3 V = normalize(getCamera() - psIn.vertex3d);
    
    float3 diffuseColor = make3(0.0f);
    
    float2 stretch = float2(1.0f, 1.0f);//1/g_StretchMap.Sample(g_SamplerPoint, psIn.texCoords).xy;
    stretch /= g_sceneSize;
    float3 irradiance = g_PostDiffuseMap.Sample(g_SamplerLinear, psIn.texCoords);
	
    float3 diffuseBlurred = make3(0.0f);
	float antiBlurGuard = 1.0f;
	float2 texPos;
    [unroll]
    for(uint i=1; i<=12; ++i) {
		texPos = psIn.texCoords + stretch * g_blurJitteredSamples[i];
		antiBlurGuard *= (g_DepthMap.Sample(g_SamplerPoint, texPos).r ? 1.0f : 0.0f);
        diffuseBlurred += g_blurJitteredWeights[i] * g_PostDiffuseMap.Sample(g_SamplerLinear, texPos).rgb;
    }
	if(antiBlurGuard > 0.0f)
		diffuseColor = g_blurJitteredWeights[0] * irradiance + diffuseBlurred;
	else
		diffuseColor = irradiance;

	diffuseColor *= pow(Albedo, 1.0f - g_blurMix);
    
    // specular
    float3 specularColor = make3(0.0f);
    float3 L;
    float attenuation;
    [unroll]
    for(uint lightID=0; lightID<NUMLIGHTSOURCES; ++lightID) {
        GetLightAttenuation(lightID, psIn.vertex3d, L, attenuation);
        float shadow = GetShadow(psIn.vertex3d, lightID, g_TSM[lightID]);
        
        specularColor += shadow * g_lightColor[lightID].xyz * attenuation * Specular_KelemenSzirmayKalos(N, L, V, m, rho_s, g_fresnelReflectance);
    }
    
    psOut.pixel = float4(diffuseColor + specularColor, 1.0f);
}

#endif
