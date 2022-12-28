#ifndef PS_JIMENEZ_COMBINE_HLSL
#define PS_JIMENEZ_COMBINE_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

static const float3 g_GaussWeights[] = {
        {1.0f,    1.0f,    1.0f   },
        {0.3251f, 0.45f,   0.3583f},
        {0.34f,   0.1864f, 0.0f   },
        {0.46f,   0.0f,    0.0402f},
};

void Main(VS2PS psIn, out PS2OM psOut) {
    float3 Albedo = g_AlbedoMap.Sample(g_SamplerLinear, psIn.texCoords).xyz;
    
    float3 blurTap[4];
    blurTap[0] = g_DiffuseMap.Sample(g_SamplerLinear, psIn.texCoords).rgb; // no need to blur with the smalles kernel
    blurTap[1] = g_BlurY1Map.Sample(g_SamplerLinear, psIn.texCoords).rgb;
    blurTap[2] = g_BlurY2Map.Sample(g_SamplerLinear, psIn.texCoords).rgb;
    blurTap[3] = g_BlurY3Map.Sample(g_SamplerLinear, psIn.texCoords).rgb;

    float3 diffuseColor = make3(0.0f);
    float3 normConst = make3(0.0f);
    [unroll]
    for(uint i=0; i<4; ++i) {
        diffuseColor += g_GaussWeights[i] * blurTap[i];
        normConst += g_GaussWeights[i];
    }
    diffuseColor /= normConst;

    diffuseColor *= pow(Albedo, 1.0f - g_blurMix);

    // specular
    float3 specularColor = g_SpecularMap.Sample(g_SamplerLinear, psIn.texCoords).rgb;

    psOut.pixel = float4(diffuseColor + specularColor, 1.0f);
}

#endif
