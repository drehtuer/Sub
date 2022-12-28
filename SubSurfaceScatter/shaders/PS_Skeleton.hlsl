#ifndef PS_Skeleton_HLSL
#define PS_Skeleton_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

[earlydepthstencil]
void Main(VS2PS psIn, out PS2OM psOut) {
	psOut.pixel = make4(0.0f);
	[unroll]
    for(uint lightID = 0; lightID < NUMLIGHTSOURCES; ++lightID) {
		float3 light3d = getLight(lightID);
        float3 l = light3d - psIn.vertex3d.xyz;
        float lightDistance = length(l);
        float3 L = l / lightDistance;
        float attenuation = lightAttenuation(g_lightAttenuation[lightID], lightDistance);
        psOut.pixel += saturate(dot(psIn.normal, L)) * g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords);
    }
	
}

#endif
