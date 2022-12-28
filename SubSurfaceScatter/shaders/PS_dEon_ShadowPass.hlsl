#include "_Connectors.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

#ifndef SHADOWPASS
#define SHADOWPASS 0
#include "PS_dEon_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 1
#include "PS_dEon_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 2
#include "PS_dEon_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 3
#include "PS_dEon_ShadowPass.hlsl"
#undef SHADOWPASS

#define SHADOWPASS 4
#endif

void MERGE(Main, SHADOWPASS)(VS2PS psIn, out PS2OM psOut) {
	float depth = LinearDepth(psIn.vertex3d, SHADOWPASS);
	psOut.pixel.r = depth;
    psOut.pixel.gb = psIn.texCoords;
    #if VARIANCESHADOWMAP
		psOut.pixel.a = pow(depth, 2.0f);
    #else
		psOut.pixel.a = 1.0f;
	#endif
}
