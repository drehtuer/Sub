#ifndef PS_QUAD_HLSL
#define PS_QUAD_HLSL

#include "_Connectors.hlsl"
#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

void Main(VS2PS psIn, out PS2OM psOut) {
    //[branch]
    switch(g_textureSelected) {
    case TEXTURE_ALBEDOTEX_ID:
        psOut.pixel = g_AlbedoTex.Sample(g_SamplerLinear, psIn.texCoords);
        break;
//
    //case TEXTURE_NORMALTEX_ID:
        //psOut.pixel = LINEAR(float4(g_NormalTex.Sample(g_SamplerPoint, psIn.texCoords).xyz, 1.0f));
        //break;
//
    //case TEXTURE_SPECTEX_ID:
        //psOut.pixel = LINEAR(g_SpecTex.Sample(g_SamplerPoint, psIn.texCoords));
        //break;
//
    //case TEXTURE_RHO_DTEX_ID:
        //psOut.pixel = LINEAR(g_Rho_dTex.Sample(g_SamplerPoint, psIn.texCoords));
        //break;

    case TEXTURE_GAMMATEX_ID:
        psOut.pixel = g_GammaTex.Sample(g_SamplerPoint, psIn.texCoords);
        break;

    //case TEXTURE_TRANSMITTANCETEX_ID:
        //psOut.pixel = LINEAR(float4(g_TransmittanceTex.Sample(g_SamplerPoint, psIn.texCoords).rrr, 1.0f));
        //break;
        
	case TEXTURE_TEXCOORDMAP_ID:
        psOut.pixel = LINEAR(g_TexCoordMap.Sample(g_SamplerPoint, psIn.texCoords));
        break;
        
    //case TEXTURE_STRETCHMAP_ID:
        //psOut.pixel = LINEAR(g_StretchMap.Sample(g_SamplerPoint, psIn.texCoords));
        //break;

    //case TEXTURE_SHADOWALBEDO0D0_ID:
        //psOut.pixel = GAMMA(g_ShadowAlbedo[0 * NUMDEPTHPEELING + 0].Sample(g_SamplerLinear, psIn.texCoords));
        //break;

    case TEXTURE_SHADOWNORMAL0D0_ID:
        psOut.pixel = LINEAR(float4(uncompressVector(g_ShadowNormal[0 * NUMDEPTHPEELING + 0].Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        break;
        
    case TEXTURE_SHADOWNORMAL20D0_ID:
        psOut.pixel = LINEAR(float4(uncompressVector(g_ShadowNormal2[0 * NUMDEPTHPEELING + 0].Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        break;

    case TEXTURE_SHADOWMAP0D0_ID:
        psOut.pixel = LINEAR(make4(g_ShadowMap[0 * NUMDEPTHPEELING + 0].Sample(g_SamplerPoint, psIn.texCoords).r));
        break;


    //case TEXTURE_SHADOWALBEDO0D1_ID:
        //psOut.pixel = GAMMA(g_ShadowAlbedo[0 * NUMDEPTHPEELING + 1].Sample(g_SamplerLinear, psIn.texCoords));
        //break;

    //case TEXTURE_SHADOWNORMAL0D1_ID:
        //psOut.pixel = LINEAR(float4(uncompressVector(g_ShadowNormal[0 * NUMDEPTHPEELING + 1].Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        //break;
//
    //case TEXTURE_SHADOWMAP0D1_ID:
        //psOut.pixel = LINEAR(make4(g_ShadowMap[0 * NUMDEPTHPEELING + 1].Sample(g_SamplerPoint, psIn.texCoords).r));
        //break;

    //case TEXTURE_SHADOWALBEDO0D2_ID:
        //psOut.pixel = GAMMA(g_ShadowAlbedo[0 * NUMDEPTHPEELING + 2].Sample(g_SamplerLinear, psIn.texCoords));
        //break;
//
    //case TEXTURE_SHADOWNORMAL0D2_ID:
        //psOut.pixel = LINEAR(float4(uncompressVector(g_ShadowNormal[0 * NUMDEPTHPEELING + 2].Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        //break;
//
    //case TEXTURE_SHADOWMAP0D2_ID:
        //psOut.pixel = LINEAR(make4(g_ShadowMap[0 * NUMDEPTHPEELING + 2].Sample(g_SamplerPoint, psIn.texCoords).r));
        //break;
//
    //case TEXTURE_SHADOWALBEDO0D3_ID:
        //psOut.pixel = GAMMA(g_ShadowAlbedo[0 * NUMDEPTHPEELING + 3].Sample(g_SamplerLinear, psIn.texCoords));
        //break;
//
    //case TEXTURE_SHADOWNORMAL0D3_ID:
        //psOut.pixel = LINEAR(float4(uncompressVector(g_ShadowNormal[0 * NUMDEPTHPEELING + 3].Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        //break;
//
    //case TEXTURE_SHADOWMAP0D3_ID:
        //psOut.pixel = LINEAR(make4(g_ShadowMap[0 * NUMDEPTHPEELING + 3].Sample(g_SamplerPoint, psIn.texCoords).r));
        //break;
//
//
    //case TEXTURE_SHADOWALBEDO1D0_ID:
        //psOut.pixel = GAMMA(g_ShadowAlbedo[1 * NUMDEPTHPEELING + 0].Sample(g_SamplerLinear, psIn.texCoords));
        //break;
//
    //case TEXTURE_SHADOWNORMAL1D0_ID:
        //psOut.pixel = LINEAR(float4(uncompressVector(g_ShadowNormal[1 * NUMDEPTHPEELING + 0].Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        //break;
//
    //case TEXTURE_SHADOWMAP1D0_ID:
        //psOut.pixel = LINEAR(make4(g_ShadowMap[1 * NUMDEPTHPEELING + 0].Sample(g_SamplerPoint, psIn.texCoords).r));
        //break;
//

    //case TEXTURE_TSM0_ID:
        //psOut.pixel = LINEAR(g_TSM[0].Sample(g_SamplerPoint, psIn.texCoords));
        //break;
//
    case TEXTURE_DIFFUSEMAP_ID:
        psOut.pixel = GAMMA(g_DiffuseMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_SPECULARMAP_ID:
        psOut.pixel = GAMMA(g_SpecularMap.Sample(g_SamplerPoint, psIn.texCoords));
        break;

    case TEXTURE_DEPTHMAP_ID:
        psOut.pixel = LINEAR(make4(g_DepthMap.Sample(g_SamplerPoint, psIn.texCoords).r));
        break;

    case TEXTURE_POSITIONMAP_ID:
        psOut.pixel = LINEAR(g_PositionMap.Sample(g_SamplerPoint, psIn.texCoords));
        break;

    case TEXTURE_NORMALMAP_ID:
        psOut.pixel = LINEAR(float4(uncompressVector(g_NormalMap.Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        break;

    case TEXTURE_VERTEXNORMALMAP_ID:
        psOut.pixel = LINEAR(float4(uncompressVector(g_VertexNormalMap.Sample(g_SamplerPoint, psIn.texCoords).xyz), 1.0f));
        break;

    case TEXTURE_ALBEDOMAP_ID:
        psOut.pixel = GAMMA(g_AlbedoMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;
//
//
    //case TEXTURE_SPECMAP_ID:
        //psOut.pixel = LINEAR(g_SpecMap.Sample(g_SamplerPoint, psIn.texCoords));
        //break;

    case TEXTURE_BLURX1MAP_ID:
        psOut.pixel = GAMMA(g_BlurX1Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURY1MAP_ID:
        psOut.pixel = GAMMA(g_BlurY1Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURX2MAP_ID:
        psOut.pixel = GAMMA(g_BlurX2Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURY2MAP_ID:
        psOut.pixel = GAMMA(g_BlurY2Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURX3MAP_ID:
        psOut.pixel = GAMMA(g_BlurX3Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURY3MAP_ID:
        psOut.pixel = GAMMA(g_BlurY3Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURX4MAP_ID:
        psOut.pixel = GAMMA(g_BlurX4Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURY4MAP_ID:
        psOut.pixel = GAMMA(g_BlurY4Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURX5MAP_ID:
        psOut.pixel = GAMMA(g_BlurX5Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_BLURY5MAP_ID:
        psOut.pixel = GAMMA(g_BlurY5Map.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_FINALSSSMAP_ID:
        psOut.pixel = GAMMA(g_FinalSSSMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_PPBRIGHTNESSMAP_ID:
        psOut.pixel = GAMMA(g_PPBrightnessMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_PPBLOOMXMAP_ID:
        psOut.pixel = GAMMA(g_PPBloomXMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_PPBLOOMYMAP_ID:
        psOut.pixel = GAMMA(g_PPBloomYMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;

    case TEXTURE_PPCOMBINEMAP_ID:
        psOut.pixel = GAMMA(g_PPCombineMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;

	case TEXTURE_TRANSMITTANCEMAP_ID:
		psOut.pixel = GAMMA(g_TransmittanceMap.Sample(g_SamplerLinear, psIn.texCoords));
		break;

	case TEXTURE_TRANSMITTANCEBLURXMAP_ID:
		psOut.pixel = GAMMA(g_TransmittanceBlurXMap.Sample(g_SamplerLinear, psIn.texCoords));
		break;

	case TEXTURE_TRANSMITTANCEBLURYMAP_ID:
		psOut.pixel = GAMMA(g_TransmittanceBlurYMap.Sample(g_SamplerLinear, psIn.texCoords));
		break;

	case TEXTURE_POSTDIFFUSEMAP_ID:
		psOut.pixel = GAMMA(g_PostDiffuseMap.Sample(g_SamplerLinear, psIn.texCoords));
		break;

    default:
        psOut.pixel = GAMMA(g_PPCombineMap.Sample(g_SamplerLinear, psIn.texCoords));
        break;
    }
}

#endif
