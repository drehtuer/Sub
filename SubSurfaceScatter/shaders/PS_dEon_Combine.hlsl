#ifndef PS_DEON_COMBINE_HLSL
#define PS_DEON_COMBINE_HLSL

#include "_Connectors.hlsl"
#include "_Utilities.hlsl"
#include "_Buffers.hlsl"
#include "_Lighting.hlsl"

#define USEJIMENEZTRANSMISSION 0
#define POINTDISTANCEHACK 1

static const float3 g_GaussWeights[] = {
    float3(0.233f, 0.455f, 0.649f),
    float3(0.100f, 0.336f, 0.344f),
    float3(0.118f, 0.198f, 0.000f),
    float3(0.113f, 0.007f, 0.007f),
    float3(0.358f, 0.004f, 0.000f),
    float3(0.078f, 0.000f, 0.000f)
};

void Main(VS2PS psIn, out PS2OM psOut) {
	float3 N;
	GETNORMAL(N);
    float3 Albedo = UNGAMMA(g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords).xyz);
    float rho_s, m;
    getSurfaceParamsTS(psIn.vertex3d, rho_s, m);
    float3 V = normalize(getCamera() - psIn.vertex3d);
    
    // rgb: color, a: thickness
    float4 blurTap[6];
    blurTap[0] = g_PostDiffuseMap.Sample(g_SamplerLinear, psIn.texCoords); // smallest kernel = unblurred diffuse
    blurTap[1] = g_BlurY1Map.Sample( g_SamplerLinear, psIn.texCoords);
    blurTap[2] = g_BlurY2Map.Sample( g_SamplerLinear, psIn.texCoords);
    blurTap[3] = g_BlurY3Map.Sample( g_SamplerLinear, psIn.texCoords);
    blurTap[4] = g_BlurY4Map.Sample( g_SamplerLinear, psIn.texCoords);
    blurTap[5] = g_BlurY5Map.Sample( g_SamplerLinear, psIn.texCoords);

    float3 diffuseColor = make3(0.0f);
    float3 normConst = make3(0.0f);
    [unroll]
    for(uint i=0; i<6; ++i) {
        diffuseColor += g_GaussWeights[i] * blurTap[i].xyz;
        normConst += g_GaussWeights[i];
    }
        
    diffuseColor /= normConst;
	
	// TSM for light TRANSLUCENCYLIGHT
    float4 TSMCoords = _toShadowTex(psIn.vertex3d, TRANSLUCENCYLIGHT);
    float3 TSM = g_TSM[TRANSLUCENCYLIGHT].Sample(g_SamplerPoint, TSMCoords.xy).xyz; // x: depth, yz: uv
    float4 thickness_mm = -5.0f * g_transmittanceScale * log(float4(blurTap[2].w, blurTap[3].w, blurTap[4].w, blurTap[5].w)); // unpack
	
	#if USEJIMENEZTRANSMISSION
		float s = thickness_mm.b;
		float3 Ts = T(s);

		// backside albedo
		diffuseColor += Ts * g_BlurY5Map.Sample(g_SamplerLinear, TSM.yz).rgb;

	#else
		float4 a_values = float4(0.433f, 0.753f, 1.412f, 2.722f); // some magic attenuation
		float4 inv_a = -1.0f / (2.0f * a_values * a_values);
		float4 fades = exp(thickness_mm * thickness_mm * inv_a); // gauss
	    
		//float2 stretchTap   = float2(1.0f, 1.0f);
		float2 stretchTap = g_StretchMap.Sample(g_SamplerPoint, psIn.texCoords).xy;
		float  textureScale = (g_sceneSize.x + g_sceneSize.y) * 0.1f / (stretchTap.x + stretchTap.y);

		// d'Eon's texture space distance causes ugly texture seams if the mesh is unwrapped in a disadvantage manner
		#if POINTDISTANCEHACK
			// vertex distance works almost the same as (stretched) texture distance without the border problem
			// a geodesic line would be better, but much more expensive to compute
			float pointDistance = 10.0f * distance(TSM.x, LinearDepth(psIn.vertex3d, TRANSLUCENCYLIGHT));
		#else
			// weight by distance in texels, scaled by magic parameters
			float pointDistance = distance(psIn.texCoords, TSM.yz);
		#endif
		
		float blendFactor3 = saturate(textureScale * pointDistance / (a_values.y * 6.0f));
		float blendFactor4 = saturate(textureScale * pointDistance / (a_values.z * 6.0f));
		float blendFactor5 = saturate(textureScale * pointDistance / (a_values.w * 6.0f));

		// backside albedo
		diffuseColor += g_GaussWeights[3]/normConst * fades.y * blendFactor3 * g_BlurY3Map.Sample(g_SamplerLinear, TSM.yz).rgb;
		diffuseColor += g_GaussWeights[4]/normConst * fades.z * blendFactor4 * g_BlurY4Map.Sample(g_SamplerLinear, TSM.yz).rgb;
		diffuseColor += g_GaussWeights[5]/normConst * fades.w * blendFactor5 * g_BlurY5Map.Sample(g_SamplerLinear, TSM.yz).rgb;
	#endif

    diffuseColor *= pow(Albedo, 1.0f - g_blurMix);

    // specular
    float3 L;
    float attenuation;
    float3 specularColor = make3(0.0f);
    [unroll]
    for(uint lightID=0; lightID<NUMLIGHTSOURCES; ++lightID) {
        GetLightAttenuation(lightID, psIn.vertex3d, L, attenuation);
        float shadow = GetShadow(psIn.vertex3d, lightID, g_TSM[lightID]);
        
        specularColor += shadow * g_lightColor[lightID].xyz * attenuation * Specular_KelemenSzirmayKalos(N, L, V, m, rho_s, g_fresnelReflectance);
    }
    
    psOut.pixel = float4(diffuseColor + specularColor, 1.0f);
}

#endif
