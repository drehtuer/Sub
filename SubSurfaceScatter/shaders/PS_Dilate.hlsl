#ifndef PS_DILATE_HLSL
#define PS_DILATE_HLSL

#define BLURWIDTH_X 1 
#define BLURWIDTH_Y 1

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

// inspired by http://developer.download.nvidia.com/SDK/9.5/Samples/MEDIA/projects/dilation.fxproj
// but without the copy&paste error at OUT.*.zw = ...

float4 Dilate(float2 blurSize, float2 texCoords, uniform Texture2D ColorTex, uniform SamplerState ColorTexSampler, uniform Texture2D BorderTex) {
	float2 texPos;
	float4 Samples[9];
	float2 Borders[9];
	texPos = float2(texCoords.x             , texCoords.y             ); // mid mid (aka center)
	Samples[0] = g_DiffuseMap.Sample(ColorTexSampler, texPos);
	Borders[0] = g_DepthMap.Sample(g_SamplerPoint, texPos).r;
	
	texPos = float2(texCoords.x - blurSize.x, texCoords.y + blurSize.y); // top left
	Samples[1] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[1] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 0.7f);
	texPos = float2(texCoords.x             , texCoords.y + blurSize.y); // top mid
	Samples[2] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[2] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 1.0f);
	texPos = float2(texCoords.x + blurSize.x, texCoords.y + blurSize.y); // top right
	Samples[3] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[3] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r.r, 0.7f);
	texPos = float2(texCoords.x - blurSize.x, texCoords.y             ); // mid left
	Samples[4] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[4] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 1.0f);
	texPos = float2(texCoords.x + blurSize.x, texCoords.y             ); // mid right
	Samples[5] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[5] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 1.0f);
	texPos = float2(texCoords.x - blurSize.x, texCoords.y - blurSize.y); // bottom left
	Samples[6] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[6] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 0.7f);
	texPos = float2(texCoords.x             , texCoords.y - blurSize.y); // bottom mid
	Samples[7] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[7] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 1.0f);
	texPos = float2(texCoords.x + blurSize.x, texCoords.y - blurSize.y); // bottom right
	Samples[8] = ColorTex.Sample(ColorTexSampler, texPos);
	Borders[8] = float2(BorderTex.Sample(g_SamplerPoint, texPos).r, 0.7f);
	
	float4 dilated = Samples[0];
	float weight = 0.0f;
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// only dilute in empty areas
	if(Borders[0].r == 0.0f) {
		// check all samples
		[unroll]
		for(uint i=1; i<9; ++i) {
			if(Borders[i].r > 0.0f) { // adjanct to border?
				weight += Borders[i].g;
				result += Borders[i].g * Samples[i];
			}
		}
		if(weight > 0.0f) {
			dilated = result / weight;
		}
	}
	return dilated;
}

void MainDiffuse(VS2PS psIn, out PS2OM psOut) {
	psOut.pixel = Dilate(float2(2.0f, 2.0f) / g_sceneSize, psIn.texCoords, g_DiffuseMap, g_SamplerLinear, g_DepthMap);
}

void MainBrandShadowNormals(VS2PS psIn, out PS2OM psOut) {
	psOut.pixel = Dilate(float2(2.0f, 2.0f) / g_sceneSize, psIn.texCoords, g_ShadowNormal[TRANSLUCENCYLIGHT], g_SamplerPoint, g_ShadowMap[TRANSLUCENCYLIGHT]);
}

#endif
