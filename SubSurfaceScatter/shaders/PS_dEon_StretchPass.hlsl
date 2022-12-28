#ifndef PS_STRETCHPASS_HLSL
#define PS_STRETCHPASS_HLSL

#include "_Connectors.hlsl"
#include "_Utilities.hlsl"

void Main(VS2PS psIn, out PS2OM psOut) {
    float3 ddu = ddx(psIn.vertex3d);
    float3 ddv = ddy(psIn.vertex3d);
    float stretchU = 1.0f / length(ddu);
    float stretchV = 1.0f / length(ddv);
    psOut.pixel = float4(stretchU, stretchV, 0.0f, 1.0f);
}

#endif
