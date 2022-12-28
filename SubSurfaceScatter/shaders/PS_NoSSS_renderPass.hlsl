#ifndef PS_NOSSS_RENDERPASS_HLSL
#define PS_NOSSS_RENDERPASS_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Lighting.hlsl"
#include "_Utilities.hlsl"

void Main(VS2PS psIn, out PS2OM psOut) {
	float3 N;
	GETNORMAL(N);
	float3 Albedo = UNGAMMA(g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords).xyz);
    float3 V = normalize(getCamera() - psIn.vertex3d);
    float rho_s, m;
    getSurfaceParamsTS(psIn.texCoords, rho_s, m);

    float3 diffuseColor = make3(0.0f);
    float3 specularColor = make3(0.0f);
    float3 L;
    float attenuation;
    [unroll]
    for(uint lightID = 0; lightID < NUMLIGHTSOURCES; ++lightID) {
        GetLightAttenuation(lightID, psIn.vertex3d, L, attenuation);
        float shadow = GetShadow(psIn.vertex3d, lightID);

		#if ORENNAYAR
			float diffuseReflectance = Diffuse_OrenNayar(N, L, V, m);
		#else
			float diffuseReflectance = Diffuse_Lambertian(N, L);
		#endif
        diffuseColor += Albedo * diffuseReflectance * shadow * g_lightColor[lightID].xyz * attenuation;
        specularColor += Specular_KelemenSzirmayKalos(N, L, V, m, rho_s, g_fresnelReflectance) * shadow * g_lightColor[lightID].xyz * attenuation;
    }

    psOut.pixel = float4(diffuseColor + specularColor, 1.0f);
}

#endif
