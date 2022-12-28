#ifndef PS_JIMENEZ_LIGHTPASS_HLSL
#define PS_JIMENEZ_LIGHTPASS_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

void Main(VS2PS psIn,
            out float4 DiffuseTarget : SV_Target0,
            out float4 SpecularTarget : SV_Target1,
			out float4 TransmittanceTarget : SV_Target2 ) {
    float3 N = normalize(uncompressVector(g_NormalMap.Sample(g_SamplerPoint, psIn.texCoords).xyz));
    float3 NV = normalize(uncompressVector(g_VertexNormalMap.Sample(g_SamplerPoint, psIn.texCoords).xyz));
    float rho_s, m;
    getSurfaceParamsSS(psIn.texCoords, rho_s, m);
    float3 Albedo = g_AlbedoMap.Sample(g_SamplerLinear, psIn.texCoords).xyz;
    Albedo = pow(Albedo, g_blurMix);
    float3 vertex3d = g_PositionMap.Sample(g_SamplerPoint, psIn.texCoords).xyz;
    float3 V = normalize(getCamera() - vertex3d);
            
    DiffuseTarget = make4(0.0f);
    SpecularTarget = make4(0.0f);
	TransmittanceTarget = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float shadow, depth;
    float4 shadowTexCoords;
    float3 backsideAlbedo, backsideNormal;

    [unroll]
    for(uint lightID = 0; lightID < NUMLIGHTSOURCES; ++lightID) {
        float3 light3d = getLight(lightID);
        float3 l = light3d - vertex3d.xyz;
        float lightDistance = length(l);
        float3 L = l / lightDistance;
        float attenuation = lightAttenuation(g_lightAttenuation[lightID], lightDistance);
        GetShadowBrand(vertex3d, lightID, depth, shadow, backsideAlbedo, backsideNormal, shadowTexCoords);

        // Jimenez et all are clever, they use the front side albedo so no sampling errors
        // for polygons perpendicular to the light source direction. 
        float2 s = distance_through_object2(vertex3d, N, depth, lightID);
        float backsideNdotL = max(dot(backsideNormal, L), 0.0f); // NdotL for backside, Jimenez et all add 0.3f here
		#if 0
			float frontsideNdotL = Diffuse_OrenNayar(N, -L, V, 1.0f); // yes, it is -L
		#else
			float NdotML = max(dot(N, -L), 0.0f);
			float frontsideNdotL = 0.0f;
			if(NdotML > 0.0f)
				frontsideNdotL = (1.0f - g_transmittanceFNL) + g_transmittanceFNL * NdotML; // limited frontside geometry impact on radiance
		#endif

        float3 Ts = s.y * T(g_transmittanceScale * s.x);
        // front side normal is important -> geometry more visible
        float3 Transmittance = frontsideNdotL * Ts * backsideNdotL * backsideAlbedo * g_lightColor[lightID].rgb * attenuation;
        float NdotL = max(dot(N, L), 0.0f);
                
        float rho_d = getRhoD(rho_s, NdotL, m);
		#if ORENNAYAR
			float diffuseReflectance = Diffuse_OrenNayar(N, L, V, m);
		#else
			float diffuseReflectance = Diffuse_Lambertian(N, L);
		#endif

        DiffuseTarget.rgb += Albedo * shadow * rho_d * diffuseReflectance * g_lightColor[lightID].rgb * attenuation;
        
		if(lightID == TRANSLUCENCYLIGHT) { // only one light
			TransmittanceTarget.rgb = Transmittance;
			TransmittanceTarget.a = s.x; // is normalized due to depth/shadow map
		}

        SpecularTarget.rgb += g_lightColor[lightID].xyz * shadow * attenuation * Specular_KelemenSzirmayKalos(N, L, V, m, rho_s, g_fresnelReflectance);
    }
}

#endif
