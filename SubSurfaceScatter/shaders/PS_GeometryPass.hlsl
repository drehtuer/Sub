#ifndef PS_GEOMETRYPASS_HLSL
#define PS_GEOMETRYPASS_HLSL

#include "_Connectors.hlsl"
#include "_Lighting.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

struct GeometryTargets {
    float4 PositionTarget     : SV_Target0;
    float4 VertexNormalTarget : SV_Target1;
    float4 NormalTarget       : SV_Target2;
    float4 AlbedoTarget       : SV_Target3;
    float4 SpecTarget         : SV_Target4;
    float4 DepthTarget        : SV_Target5;
};

[earlydepthstencil]
void Main(VS2PS psIn, out GeometryTargets GT) {
    // position
    GT.PositionTarget = float4(psIn.vertex3d, 1.0f);

    // vertex normal
    GT.VertexNormalTarget = float4(compressVector(normalize(psIn.normal)), 0.0f);

    // normal
    // N needs to be compressed so it can be stored in an UINT texture
    // fallback to vertex normal if no normal texture is existing
    float3 N = g_NormalTex.Sample(g_SamplerPoint, psIn.texCoords).xyz;
    if(length(N) > 0) {
        // transform to world space
		N = mul(float4(normalize(uncompressVector(N)), 0.0f), g_object2World_IT).xyz;
    } else {
        N = psIn.normal; // already in world space, thanks to the vertex shader
    }
    GT.NormalTarget = float4(compressVector(N), 0.0f);

    // albedo
    float3 Albedo = UNGAMMA(g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords).rgb);
    GT.AlbedoTarget = float4(Albedo, 1.0f);

    // spec
    float4 Spec = g_SpecTex.Sample(g_SamplerPoint, psIn.texCoords);
    GT.SpecTarget = Spec;

    // linear depth, scaled to [0, 1]
    GT.DepthTarget = make4(LinearDepthCam(psIn.vertex3d));
}
#endif
