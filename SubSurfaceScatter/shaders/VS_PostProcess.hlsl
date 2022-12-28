#ifndef VS_POSTPROCESS_HLSL
#define VS_POSTPROCESS_HLSL

#include "_Connectors.hlsl"

void Main(IA2VS vsIn, out VS2PS psOut) {
	vsOut.vertex3d = vsIn.vertex;
	vsOut.normal = vsIn.normal;
	vsOut.texCoords = vsIn.texCoords;
	vsOut.vertex2d = float4(vsIn.vertex, 1.0f);
}

#endif
