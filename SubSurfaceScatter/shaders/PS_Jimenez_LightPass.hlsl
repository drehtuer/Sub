#ifndef PS_JIMENEZ_LIGHTPASS_HLSL
#define PS_JIMENEZ_LIGHTPASS_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

#define USEVERTEXNORMALFORBACKSIDE 1

void Main(VS2PS psIn,
            out float4 DiffuseTarget : SV_Target0,
            out float4 SpecularTarget : SV_Target1) {
    float3 N = normalize(uncompressVector(g_NormalMap.Sample(g_SamplerPoint, psIn.texCoords).xyz));
	#if USEVERTEXNORMALFORBACKSIDE
		float3 NV = normalize(uncompressVector(g_VertexNormalMap.Sample(g_SamplerPoint, psIn.texCoords).xyz));
	#else
		float3 NV = N;
	#endif
    float rho_s, m;
    getSurfaceParamsSS(psIn.texCoords, rho_s, m);
    float3 Albedo = g_AlbedoMap.Sample(g_SamplerLinear, psIn.texCoords).xyz;
    Albedo = pow(Albedo, g_blurMix);
    float3 vertex3d = g_PositionMap.Sample(g_SamplerPoint, psIn.texCoords).xyz;
    float3 V = normalize(getCamera() - vertex3d);

    DiffuseTarget = make4(0.0f);
    SpecularTarget = make4(0.0f);
    float shadow, depth;
    float4 shadowTexCoords;

    [unroll]
    for(uint lightID = 0; lightID < NUMLIGHTSOURCES; ++lightID) {
        float3 light3d = getLight(lightID);
        float3 l = light3d - vertex3d.xyz;
        float lightDistance = length(l);
        float3 L = l / lightDistance;
        float attenuation = lightAttenuation(g_lightAttenuation[lightID], lightDistance);
        GetShadowJimenez(vertex3d, lightID, depth, shadow, shadowTexCoords);

        float s = g_transmittanceScale * distance_through_object(vertex3d, N, depth, lightID);
        float backsideNdotL = max(0.3f + dot(-NV, L), 0.0f); // vertex normal -> blurred version of texture normal
        float3 Ts = T(s); //g_TransmittanceTex.Sample(g_SamplerLinear, float2(s / TRANSMITTANCETEXSIZE, 0.0f)).rgb;
		#if ORENNAYAR
			float backsideDiffuseReflectance = Diffuse_OrenNayar(N, L, V, m);
		#else
			float backsideDiffuseReflectance = Diffuse_Lambertian(N, L);
		#endif
        float3 Transmittance = Ts * backsideNdotL * g_lightColor[lightID].rgb * attenuation * Albedo.rgb;
        
        float NdotL = max(dot(N, L), 0.0f);
        float rho_d = getRhoD(rho_s, NdotL, m);
        #if ORENNAYAR
			float diffuseReflectance = Diffuse_OrenNayar(N, L, V, m);
		#else
			float diffuseReflectance = Diffuse_Lambertian(N, L);
		#endif
        DiffuseTarget.rgb += Albedo * shadow * rho_d * diffuseReflectance * g_lightColor[lightID].rgb * attenuation;
        DiffuseTarget.rgb += Transmittance;

		// Jimenez et al. suggested adding specular to diffuse and take advantage of sss blur as cheap bloom
        SpecularTarget.rgb += g_lightColor[lightID].xyz * shadow * attenuation * Specular_KelemenSzirmayKalos(N, L, V, m, rho_s, g_fresnelReflectance);
    }
}

#endif
