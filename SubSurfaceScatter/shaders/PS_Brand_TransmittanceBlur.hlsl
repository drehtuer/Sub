#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"
#include "_Lighting.hlsl"

// include this only once
#ifndef BLURTYPE

#define DEPTHAWAREBLUR 1
#define UPPERBOUND 30
#define SHOWTHICKNESS 0
#define USESTRETCH 0
#define USEGAUSSIANWEIGHTS 0

static const float g_maxdd = 0.0001f;

float getBlurWidth(float dist) {
	// log
	//return g_transmittanceBlur1 * max(log(g_transmittanceBlur2 * (dist + g_maxdd)) + 1.0f, 0.0f) / 1.0f;
	// exp
	return g_transmittanceBlur1 * (exp(g_transmittanceBlur2 * dist) - 1.0f) / (exp(1) - 1.0f);
	// sigmoid
	//return g_transmittanceBlur1 / (1.0f + exp( - g_transmittanceBlur2 * dist + 10.0f));
	// pow
	//return pow(g_transmittanceBlur1 * dist, g_transmittanceBlur2);
}

float gaussian(float val, float sigma) {
	return 1.0f/(sqrt(2*PI)*sigma) * exp(-pow(val/sigma, 2.0f)/2.0f);
}

float3 convolve(float2 texCoords, uint steps, float2 stepWidth, uniform Texture2D Tex, uniform SamplerState TexSampler) {
    float3 sum = make3(0.0f), tmp;
    
    #if DEPTHAWAREBLUR
		// center
		float weight = 1.0f/(2.0f*steps+1.0f);
		#if USEGAUSSIANWEIGHTS
			weight = 1.0f / gaussian(0, 2.0f*steps+1.0f);
		#endif
		float sumGaussian = weight;
		sum += weight * Tex.Sample(TexSampler, texCoords).rgb;

		// left
		float2 coords = texCoords - stepWidth;
		[unroll]
		for(uint l=0; l<UPPERBOUND; ++l) {
			#if USEGAUSSIANWEIGHTS
				weight = 1.0f / gaussian(l+1, 2.0f*steps+1.0f);
			#endif
			tmp = weight * Tex.Sample(TexSampler, coords).rgb;
			if(l < steps) {
				sumGaussian =+ weight;
				sum += tmp;
				coords -= stepWidth;
			}
		}

		// right
		coords = texCoords + stepWidth;
		[unroll]
		for(uint r=0; r<UPPERBOUND; ++r) {
			#if USEGAUSSIANWEIGHTS
				weight = 1.0f / gaussian(r+1, 2.0f*steps+1.0f);
			#endif
			tmp = weight * Tex.Sample(TexSampler, coords).rgb;
			if(r < steps) {
				sumGaussian += weight;
				sum += tmp;
				coords += stepWidth;
			}
		}
		
		#if USEGAUSSIANWEIGHTS
			sum /= sumGaussian;
		#endif
    #else
		float2 coords = texCoords - UPPERBOUND * stepWidth;
		float weight = 1.0f/(2*UPPERBOUND+1);
		[unroll]
		for(uint i=0; i<(2*UPPERBOUND+1); ++i) {
			sum += weight * Tex.Sample(TexSampler, coords).rgb;
			coords += stepWidth;
		}
    #endif
    
    return sum;
}

float stretchX(float depth, uniform float correction) {
	#if USESTRETCH
		return (depth + correction * min(abs(ddx(depth)), g_maxdd));
	#else
		return 1.0f;
	#endif
}

float stretchY(float depth, uniform float correction) {
	#if USESTRETCH
		return (depth + correction * min(abs(ddy(depth)), g_maxdd));
	#else
		return 1.0f;
	#endif
}

float3 convolveX(float2 texCoords : TEXCOORD0,
                 float thickness,
                 uniform Texture2D Tex,
                 uniform SamplerState TexSampler) {

    uint steps = getBlurWidth(thickness);
    float depth = g_DepthMap.Sample(g_SamplerPoint, texCoords).r;
    float2 finalWidth = 1.0f / g_sceneSize.x * stretchX(depth, g_stretchBeta) * float2(1.0f, 0.0f);
    return convolve(texCoords, steps, finalWidth, Tex, TexSampler);
}

float3 convolveY(float2 texCoords : TEXCOORD0,
                 float thickness,
                 uniform Texture2D Tex,
                 uniform SamplerState TexSampler) {

    uint steps = getBlurWidth(thickness);
    float depth = g_DepthMap.Sample(g_SamplerPoint, texCoords).r;
    float2 finalWidth = 1.0f / g_sceneSize.y * stretchY(depth, g_stretchBeta) * float2(0.0f, 1.0f);
    return convolve(texCoords, steps, finalWidth, Tex, TexSampler);
}

// this function is created only once
// mix diffuse and transmittance
void MixMain(VS2PS psIn, out PS2OM psOut) {
	psOut.pixel = float4(g_DiffuseMap.Sample(g_SamplerLinear, psIn.texCoords).rgb + g_TransmittanceBlurYMap.Sample(g_SamplerLinear, psIn.texCoords).rgb, 1.0f);
}

#define BLURTYPE X
#define BLURTEXTURE g_TransmittanceMap
#include "PS_Brand_TransmittanceBlur.hlsl"
#undef BLURTYPE
#undef BLURTEXTURE

#define BLURTYPE Y
#define BLURTEXTURE g_TransmittanceBlurXMap

#endif

// function created 2 times
[earlydepthstencil]
void MERGE(Main, BLURTYPE)(VS2PS psIn, out PS2OM psOut) {
	float thickness = g_TransmittanceMap.Sample(g_SamplerPoint, psIn.texCoords).a;
	#if SHOWTHICKNESS
		thickness = getBlurWidth(thickness);
		psOut.pixel.rgb = make3(thickness);
	#else
		psOut.pixel.rgb = MERGE(convolve, BLURTYPE)(psIn.texCoords,
									thickness,
									BLURTEXTURE,
									g_SamplerLinear);
	#endif
	psOut.pixel.a = 1.0f;
}
