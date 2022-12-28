#ifndef _UTILITIES_HLSL
#define _UTILITIES_HLSL

#ifndef MERGE
#define MERGE(a, b) a##b
#endif

#define UNGAMMA(x) pow(x, 1.0f / g_gammaCorrection)
#define GAMMA(x)   pow(x, g_gammaCorrection)
#define LINEAR(x)  (x)

#define GETNORMAL(n) n = g_NormalTex.Sample(g_SamplerPoint, psIn.texCoords).xyz;\
	                 n = mul(float4(normalize(uncompressVector(n)), 0.0f), g_object2World_IT).xyz

#define PI 3.141592653589793238f

float absDepth(float depth, uniform float zNear, uniform float zFar) {
	return zFar / (zFar - zNear) - zNear * zFar / ((zFar - zNear) * depth);
}

float2 make2(float value) {
	return float2(value, value);
}

float3 make3(float value) {
	return float3(value, value, value);
}

float4 make4(float value, float alpha = 1.0f) {
	return float4(value, value, value, alpha);
}

float2 flipTexH(float2 texCoords) {
	return float2(texCoords.x, 1 - texCoords.y);
}

float3 uncompressVector(float3 v) {
	return 2.0f * v - 1.0f; 
}

float3 compressVector(float3 v) {
	return 0.5f * v + 0.5f; 
}

#endif
