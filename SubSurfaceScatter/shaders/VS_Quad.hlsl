#ifndef VS_QUAD_HLSL
#define VS_QUAD_HLSL

#include "_Connectors.hlsl"

void Main(IA2VS vsIn, out VS2PS vsOut) {
	vsOut.vertex3d = vsIn.vertex;
	vsOut.normal = vsIn.normal;
	vsOut.texCoords = vsIn.texCoords;
	vsOut.vertex2d = float4(2*vsIn.vertex, 1.0f);
}
#endif
