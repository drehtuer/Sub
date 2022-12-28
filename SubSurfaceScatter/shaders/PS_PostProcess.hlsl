#ifndef PS_POSTPROCESS_HLSL
#define PS_POSTPROCESS_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

#define MAXBLURSTEPS 7
#define BLOOMWIDTH 3

void BrightPass(VS2PS psIn, out PS2OM psOut) {
    float3 FinalSSS = g_FinalSSSMap.Sample(g_SamplerLinear, psIn.texCoords).rgb;
    float L = toLuminance(FinalSSS); // rgb to luminance

    psOut.pixel = float4(FinalSSS * smoothstep(g_minBloom, g_maxBloom, L), 1.0f);
}


//const static float g_gaussWeights[MAXBLURSTEPS] = {0.12149, 0.14204, 0.156, 0.16095, 0.156, 0.14204, 0.12149};
static const float g_gaussWeights[] = {0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f};

void BloomX(VS2PS psIn, out PS2OM psOut) {
    float2 steps = float2(BLOOMWIDTH / g_sceneSize.x , 0.0f);
    float2 texCoords = psIn.texCoords - 3*steps;
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for(uint i=0; i<MAXBLURSTEPS; ++i) {
        sum += g_gaussWeights[i] * g_PPBrightnessMap.Sample(g_SamplerLinear, texCoords + i*steps);
    }
    psOut.pixel = float4(sum, 1.0f);
}

void BloomY(VS2PS psIn, out PS2OM psOut) {
    float2 steps = float2(0.0f, BLOOMWIDTH / g_sceneSize.y);
    float2 texCoords = psIn.texCoords - 3*steps;
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    [unroll]
    for(uint i=0; i<MAXBLURSTEPS; ++i) {
        sum += g_gaussWeights[i] * g_PPBloomXMap.Sample(g_SamplerLinear, texCoords + i*steps);
    }
    psOut.pixel = float4(sum, 1.0f);
}

void Combine(VS2PS psIn, out PS2OM psOut) {
    // final output -> gamma correction
    psOut.pixel = saturate(g_FinalSSSMap.Sample(g_SamplerLinear, psIn.texCoords) + g_PPBloomYMap.Sample(g_SamplerLinear, psIn.texCoords));
}

#endif
