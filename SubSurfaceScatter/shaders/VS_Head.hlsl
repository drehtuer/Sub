#ifndef VS_HEAD_HLSL
#define VS_HEAD_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

// functions
void Main(IA2VS vsIn, out VS2PS vsOut) {
	vsOut.vertex3d = mul(float4(vsIn.vertex, 1.0f), g_object2World).xyz;
	vsOut.normal = mul(float4(normalize(vsIn.normal), 0.0f), g_object2World_IT).xyz;
	vsOut.texCoords = flipTexH(vsIn.texCoords);
	vsOut.vertex2d = mul(float4(vsIn.vertex, 1.0f), g_object2CameraProj);
}

#endif
