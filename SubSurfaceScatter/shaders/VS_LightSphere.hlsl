#include "_Connectors.hlsl"
#include "_Buffers.hlsl"

#ifndef LIGHTSPHERE

#define LIGHTSPHERE 0
#include "VS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 1
#include "VS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 2
#include "VS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 3
#include "VS_LightSphere.hlsl"
#undef LIGHTSPHERE

#define LIGHTSPHERE 4

#endif

void MERGE(Main, LIGHTSPHERE)(IA2VS vsIn, out VS2PS vsOut) {
	vsOut.vertex3d = mul(float4(vsIn.vertex, 1.0f), g_light2World[LIGHTSPHERE]).xyz;
	vsOut.vertex2d = mul(mul(float4(vsIn.vertex, 1.0f), g_light2World[LIGHTSPHERE]), g_world2CameraProj);
	vsOut.normal = mul(float4(vsIn.normal, 0.0f), g_light2World[LIGHTSPHERE]).xyz;
	vsOut.texCoords = vsIn.texCoords;
}
