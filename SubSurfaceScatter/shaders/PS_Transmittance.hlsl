#ifndef PS_TRANSMITTANCE_HLSL
#define PS_TRANSMITTANCE_HLSL

#include "_Connectors.hlsl"
#include "_Lighting.hlsl"

void Main(VS2PS psIn, out PS2OM psOut) {
	psOut.pixel = float4(T(psIn.texCoords.x * TRANSMITTANCETEXSIZE), 1.0f);
}

#endif
