#ifndef PS_DEON_IRRADIANCEPASS_HLSL
#define PS_DEON_IRRADIANCEPASS_HLSL

#include "_Connectors.hlsl"
#include "_Utilities.hlsl"
#include "_Buffers.hlsl"
#include "_Lighting.hlsl"

// TSM style
void Main(VS2PS psIn, out float4 DiffuseTarget : SV_Target0, out float4 DepthTarget : SV_Target1) {
    float3 N;
	GETNORMAL(N);
    float3 Albedo = UNGAMMA(g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords).xyz);
    Albedo = pow(Albedo, g_blurMix);
    float rho_s, m;
    getSurfaceParamsTS(psIn.texCoords, rho_s, m);
	float3 V = normalize(getCamera() - psIn.vertex3d);
	
	DepthTarget = LinearDepthCam(psIn.vertex3d);
    
    // diffuse
    float shadow, NdotL, rho_d, attenuation;
    float3 L;
    DiffuseTarget = make4(0.0f);
    [unroll]
    for(uint lightID = 0; lightID < NUMLIGHTSOURCES; ++lightID) {
		GetLightAttenuation(lightID, psIn.vertex3d, L, attenuation);
        shadow = GetShadow(psIn.vertex3d, lightID, g_TSM[lightID]);
        NdotL = max(dot(N, L), 0.0f);
        float rho_d = getRhoD(rho_s, NdotL, m);
		#if ORENNAYAR
			float diffuseReflectance = Diffuse_OrenNayar(N, L, V, m);
		#else
			float diffuseReflectance = Diffuse_Lambertian(N, L);
		#endif
		DiffuseTarget.rgb += Albedo * shadow * rho_d * diffuseReflectance * g_lightColor[lightID].rgb * attenuation;
    }
    
    // thickness
    float depth, backFacingEst, thicknessToLight;
    float3 lpos = getLight(TRANSLUCENCYLIGHT);
    float3 LT = normalize(lpos - psIn.vertex3d);
    GetShadowTSM(psIn.vertex3d, TRANSLUCENCYLIGHT, N, depth, shadow, backFacingEst, thicknessToLight);
    float NdotLT = dot(N, LT);
    if(NdotLT > 0.0f) // block light side
        thicknessToLight = 50.0f; // high value -> no transmittance
    float correctedThickness = saturate(-NdotLT) * thicknessToLight; // prevent double contributions
    float finalThickness = lerp(thicknessToLight, correctedThickness, backFacingEst);
    DiffuseTarget.a = exp(-20.0f * finalThickness); // pack
}

#endif
