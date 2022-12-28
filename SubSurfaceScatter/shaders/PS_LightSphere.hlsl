#ifndef LIGHTSPHERE
#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Lighting.hlsl"

#define LIGHTSPHERE 0
#include "PS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 1
#include "PS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 2
#include "PS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 3
#include "PS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 4

#endif

void MERGE(Main, LIGHTSPHERE)(VS2PS psIn, out PS2OM psOut) {
    float3 light3d = getLight(LIGHTSPHERE);
    float3 l = light3d - float3(0.0f, 0.0f, 0.0f);
    float lightDistance = length(l);
    // no NdotL or NdotV
	psOut.pixel = float4(g_lightColor[LIGHTSPHERE].rgb * lightAttenuation(g_lightAttenuation[LIGHTSPHERE], lightDistance), 1.0f);
}
