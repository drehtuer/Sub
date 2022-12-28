#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Connectors.hlsl"
#include "_Lighting.hlsl"

#define USEVERTEXNORMALS 0

#if !defined(SHADOWPASS) && !defined(PEELINGPASS)

    struct ShadowOut {
        float4 AlbedoTarget : SV_Target0;
        float4 NormalTarget : SV_Target1;
        float4 DepthTarget  : SV_Target2;
    };
    
    #define SHADOWPASS 0
    #include "PS_ShadowPeelingPass.hlsl"
    #undef SHADOWPASS
    #undef PEELINGPASS

    #define SHADOWPASS 1
    #include "PS_ShadowPeelingPass.hlsl"
    #undef SHADOWPASS
    #undef PEELINGPASS

    #define SHADOWPASS 2
    #include "PS_ShadowPeelingPass.hlsl"
    #undef SHADOWPASS
    #undef PEELINGPASS

    #define SHADOWPASS 3
    #include "PS_ShadowPeelingPass.hlsl"
    #undef SHADOWPASS
    #undef PEELINGPASS

    #define SHADOWPASS 4
#endif

#ifndef PEELINGPASS

    #define PEELINGPASS 0
    #include "PS_ShadowPeelingPass.hlsl"
    #undef PEELINGPASS

    #define PEELINGPASS 1
    #include "PS_ShadowPeelingPass.hlsl"
    #undef PEELINGPASS

    #ifndef PEELINGPASS
        #define PEELINGPASS 2
        #if PEELINGPASS < NUMDEPTHPEELING
            #include "PS_ShadowPeelingPass.hlsl"
            #undef PEELINGPASS
        #endif
    #endif

    #ifndef PEELINGPASS
        #define PEELINGPASS 3
        #if PEELINGPASS < NUMDEPTHPEELING
            #include "PS_ShadowPeelingPass.hlsl"
            #undef PEELINGPASS
        #endif
    #endif

    #ifndef PEELINGPASS
        #define PEELINGPASS 4
        #if PEELINGPASS < NUMDEPTHPEELING
            #include "PS_ShadowPeelingPass.hlsl"
            #undef PEELINGPASS
        #endif
    #endif

    #ifndef PEELINGPASS
        #define PEELINGPASS 5
        #if PEELINGPASS < NUMDEPTHPEELING
            #include "PS_ShadowPeelingPass.hlsl"
            #undef PEELINGPASS
        #endif
    #endif

#endif

#if PEELINGPASS == 0
    // step 0, normal shadow map
    [earlydepthstencil]
    void MERGE(MERGE(Main, SHADOWPASS), MERGE(d, PEELINGPASS))(VS2PS psIn, out ShadowOut psOut) {
		#if USEVERTEXNORMALS
			psOut.NormalTarget = float4(compressVector(psIn.normal), 0.0f);
		#else
			float3 N;
			GETNORMAL(N);
			psOut.NormalTarget = float4(compressVector(N), 0.0f);
		#endif
        psOut.AlbedoTarget = UNGAMMA(g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords));
        float depth = LinearDepth(psIn.vertex3d, SHADOWPASS);
		#if VARIANCESHADOWMAP
			psOut.DepthTarget = float4(depth, 0.0f, 0.0f, pow(depth, 2.0f));
		#else
			psOut.DepthTarget = make4(depth);
		#endif
    }
#else
    // step 1, don't draw pixels that were already in d0
    void MERGE(MERGE(Main, SHADOWPASS), MERGE(d, PEELINGPASS))(VS2PS psIn, out ShadowOut psOut) {
        float4 shadowTexCoords = _toShadowTex(psIn.vertex3d, SHADOWPASS);
        // some other variable
        if(LinearDepth(psIn.vertex3d, SHADOWPASS) <= g_depthPeelingEpsilon + g_ShadowMap[SHADOWPASS * NUMDEPTHPEELING + (PEELINGPASS - 1)].Sample(g_SamplerPoint, shadowTexCoords.xy).r) {
            discard; // fragment already in previous layer
        } else {
            psOut.NormalTarget = make4(0.0f); // save some texture accesses
            psOut.AlbedoTarget = make4(0.0f); // save some texture accesses
            float depth = LinearDepth(psIn.vertex3d, SHADOWPASS);
			#if VARIANCESHADOWMAP
				psOut.DepthTarget = float4(depth, 0.0f, 0.0f, pow(depth, 2.0f));
			#else
				psOut.DepthTarget = make4(depth);
			#endif
        }
    }
#endif
