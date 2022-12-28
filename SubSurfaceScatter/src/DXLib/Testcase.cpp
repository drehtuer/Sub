#include "DXLib/Testcase.h"

using namespace SubSurfaceScatter;

#define USEBAKEDTEXTURE 0

HRESULT Testcase::loadModel() {
    HRESULT hr = S_OK;

	chkHr(loadMesh("../../data/Testcase/Test case.obj"));

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
	m_TextureManager->addTexture("TestcaseAlbedoTex", L"../../data/Testcase/TestcaseAlbedo.png", ImageInfo);
	    
    ImageInfo.Format = LINEARBUFFERFORMAT;
    // Texture 1: NormalTex
    m_TextureManager->addTexture("TestcaseNormalTex", L"../../data/Testcase/TestcaseNormals2.png", ImageInfo);
    
    // Texture 2: SpecTex
    m_TextureManager->addTexture("FixedSpecTex", L"../../data/FixedSpec.png", ImageInfo);

	// Texture 3: BoneTex
	m_TextureManager->addTexture("BoneTex", L"../../data/bone_color.png", ImageInfo);

	// creating shaders
    ID3DBlob *VSBlob = NULL;
    m_ShaderManager->createVS("VS_Head", L"../../shaders/VS_Head.hlsl", "Main", &VSBlob);
    m_ShaderManager->createIL("IL_"+m_ObjectName, &m_InputDescs[0], (UINT)m_InputDescs.size(), VSBlob);
	
    setEnabled(true);
    return hr;
}

void Testcase::drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("TestcaseAlbedoTex"));
    m_TextureManager->swap("SpecTex", m_TextureManager->getSRV("FixedSpecTex"));
	m_TextureManager->swap("NormalTex", m_TextureManager->getSRV("TestcaseNormalTex"));
	m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
	draw("Test case", VSName, GSName, PSName, CSName);
}

void Testcase::drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("BoneTex"));
    m_TextureManager->swap("SpecTex", m_TextureManager->getSRV("NullTex"));
	m_TextureManager->swap("NormalTex", m_TextureManager->getSRV("NullTex"));
	m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
	draw("Test bone", VSName, GSName, PSName, CSName);
}
