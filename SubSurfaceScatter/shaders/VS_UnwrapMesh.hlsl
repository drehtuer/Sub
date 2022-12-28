#ifndef VS_UNWRAPMESH_HLSL
#define VS_UNWRAPMESH_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

// functions
void Main(IA2VS vsIn, out VS2PS vsOut) {
	vsOut.vertex3d = mul(float4(vsIn.vertex, 1.0f), g_object2World).xyz;
	vsOut.normal = mul(float4(vsIn.normal, 0.0f), g_object2World_IT).xyz;
	vsOut.texCoords = float2(vsIn.texCoords.x, 1.0f - vsIn.texCoords.y);
	vsOut.vertex2d = float4(2.0f * vsIn.texCoords.x - 1.0f, 2.0f * vsIn.texCoords.y - 1.0f, 0.0f, 1.0f);
}

#endif
