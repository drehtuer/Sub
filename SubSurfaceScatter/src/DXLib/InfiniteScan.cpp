#include "DXLib/InfiniteScan.h"

#define USELOWRESMESH 0

using namespace SubSurfaceScatter;

HRESULT InfiniteScan::loadModel() {
    
	HRESULT hr = S_OK;
	
	#if USELOWRESMESH
		chkHr(loadMesh("../../data/Infinite_Scan_Ver0.1/Lee Perry-Smith_low.obj"));
		m_ObjectName = "Lee Perry-Smith";
	#else
		chkHr(loadMesh("../../data/Infinite_Scan_Ver0.1/Lee Perry-Smith.obj"));
	#endif

	// loading textures
    D3DX11_IMAGE_LOAD_INFO ImageInfo;
    ZeroMemory(&ImageInfo, sizeof(ImageInfo));
    ImageInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    ImageInfo.CpuAccessFlags = 0;
    ImageInfo.Format = IMAGEBUFFERFORMAT;
    ImageInfo.Usage = D3D11_USAGE_IMMUTABLE;
    ImageInfo.MipLevels = 1;
    ImageInfo.Width = D3DX11_DEFAULT;
    ImageInfo.Height = D3DX11_DEFAULT;
    ImageInfo.Depth = D3DX11_DEFAULT;
    ImageInfo.FirstMipLevel = 0;
    ImageInfo.MiscFlags = 0;
    ImageInfo.MipFilter = D3DX11_DEFAULT;
	ImageInfo.Filter = D3DX11_DEFAULT;
    
    // Texture 0: AlbedoTex
    m_TextureManager->addTexture("ISAlbedoTex", L"../../data/Infinite_Scan_Ver0.1/Images/Map-COL.jpg", ImageInfo);
    
    ImageInfo.Format = LINEARBUFFERFORMAT;
    // Texture 1: NormalTex
    m_TextureManager->addTexture("ISNormalTex", L"../../data/Infinite_Scan_Ver0.1/Images/Infinite-Level_02_World_NoSmoothUV.jpg", ImageInfo);
    
    // Texture 2: SpecTex
    m_TextureManager->addTexture("FixedSpecTex", L"../../data/FixedSpec.png", ImageInfo);

	// creating shaders
    ID3DBlob *VSBlob = NULL;
    m_ShaderManager->createVS("VS_Head", L"../../shaders/VS_Head.hlsl", "Main", &VSBlob);
    m_ShaderManager->createIL("IL_"+m_ObjectName, &m_InputDescs[0], (UINT)m_InputDescs.size(), VSBlob);
	
    setEnabled(true);
    return hr;
}

void InfiniteScan::drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("ISAlbedoTex"));
    m_TextureManager->swap("NormalTex", m_TextureManager->getSRV("ISNormalTex"));
    m_TextureManager->swap("SpecTex", m_TextureManager->getSRV("FixedSpecTex"));
	m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
	draw("Lee Perry-Smith", VSName, GSName, PSName, CSName);
}

void InfiniteScan::drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
	// all skin, no bones
}
