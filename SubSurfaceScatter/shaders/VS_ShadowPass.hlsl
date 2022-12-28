#include "_Buffers.hlsl"
#include "_Connectors.hlsl"
#include "_Utilities.hlsl"

#ifndef SHADOWPASSMAIN

#define SHADOWPASSMAIN 0
#include "VS_ShadowPass.hlsl"
#undef SHADOWPASSMAIN

#define SHADOWPASSMAIN 1
#include "VS_ShadowPass.hlsl"
#undef SHADOWPASSMAIN

#define SHADOWPASSMAIN 2
#include "VS_ShadowPass.hlsl"
#undef SHADOWPASSMAIN

#define SHADOWPASSMAIN 3
#include "VS_ShadowPass.hlsl"
#undef SHADOWPASSMAIN

#define SHADOWPASSMAIN 4

#endif

void MERGE(Main, SHADOWPASSMAIN)(IA2VS vsIn, out VS2PS vsOut) {
    float4x4 object2LightProj = mul(g_object2World, g_world2LightProj[SHADOWPASSMAIN]);
    vsOut.vertex3d = mul(float4(vsIn.vertex, 1.0f), g_object2World).xyz;
    vsOut.normal = mul(float4(normalize(vsIn.normal), 0.0f), g_object2World_IT).xyz;
    vsOut.texCoords = flipTexH(vsIn.texCoords);
    vsOut.vertex2d = mul(float4(vsIn.vertex, 1.0f), object2LightProj);
}
