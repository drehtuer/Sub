#include "_Utilities.hlsl"
#include "_Lighting.hlsl"
#include "_Connectors.hlsl"

#ifndef SHADOWPASS

#define SHADOWPASS 0
#include "PS_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 1
#include "PS_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 2
#include "PS_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 3
#include "PS_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 4

#endif

void MERGE(Main, SHADOWPASS)(VS2PS psIn, out PS2OM psOut) {
	float depth = LinearDepth(psIn.vertex3d, SHADOWPASS);
	#if VARIANCESHADOWMAP
		psOut.pixel = float4(depth, 0.0f, 0.0f, pow(depth, 2.0f));
	#else
		psOut.pixel = make4(depth);
	#endif
}