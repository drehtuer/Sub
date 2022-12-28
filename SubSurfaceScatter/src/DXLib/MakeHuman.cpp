#include "DXLib/MakeHuman.h"

using namespace SubSurfaceScatter;

HRESULT MakeHuman::loadModel() {
    HRESULT hr = S_OK;
	
	chkHr(loadMesh("../../data/MakeHuman/MakeHuman.obj"));

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
    
    // Texture 0: MHBodyTex
	ImageInfo.Format = IMAGEBUFFERFORMAT;
    m_TextureManager->addTexture("MHBodyTex", L"../../data/MakeHuman/body_color.png", ImageInfo);

    // Texture 1: MHHeadTex
    m_TextureManager->addTexture("MHHeadTex", L"../../data/MakeHuman/head_color.png", ImageInfo);

    // Texture 2: MHEyesTex
    m_TextureManager->addTexture("MHEyesTex", L"../../data/MakeHuman/eyes_color.png", ImageInfo);

    // Texture 3: MHBoneTex
    m_TextureManager->addTexture("MHBoneTex", L"../../data/bone_color.png", ImageInfo);
    
    // Texture 4: MHBodyNormalTex
    ImageInfo.Format = LINEARBUFFERFORMAT;
    m_TextureManager->addTexture("MHBodyNormalTex", L"../../data/MakeHuman/BodyNormalMapLH.png", ImageInfo);

    // Texture 5: MHHeadNormalTex
    m_TextureManager->addTexture("MHHeadNormalTex", L"../../data/MakeHuman/HeadNormalMapLH.png", ImageInfo);

    // Texture 6: MHFixedSpecTex
    m_TextureManager->addTexture("MHFixedSpecTex", L"../../data/FixedSpec.png", ImageInfo);

    // creating shaders
    ID3DBlob *VSBlob = NULL;
    m_ShaderManager->createVS("VS_Head", L"../../shaders/VS_Head.hlsl", "Main", &VSBlob);
    m_ShaderManager->createIL("IL_"+m_ObjectName, &m_InputDescs[0], (UINT)m_InputDescs.size(), VSBlob);
    //m_ShaderManager->createPS("PS_Skeleton", L"../../shaders/PS_Skeleton.hlsl", "Main");


    setEnabled(true);
    return hr;
}

void MakeHuman::drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("MHBodyTex"));
    m_TextureManager->swap("NormalTex", m_TextureManager->getSRV("MHBodyNormalTex"));
    m_TextureManager->swap("SpecTex", m_TextureManager->getSRV("MHFixedSpecTex"));
    m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
    draw("Body_BodyMesh", VSName, GSName, PSName, CSName);
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("MHHeadTex"));
    m_TextureManager->swap("NormalTex", m_TextureManager->getSRV("MHHeadNormalTex"));
    m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
    draw("Head_HeadMesh", VSName, GSName, PSName, CSName);
}

void MakeHuman::drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) {
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("MHEyesTex"));
    m_TextureManager->swap("NormalTex", m_TextureManager->getSRV("NullTex"));
    m_TextureManager->swap("SpecTex", m_TextureManager->getSRV("NullTex"));
	m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
    draw("Eyes_EyeMesh", VSName, GSName, PSName, CSName);
    draw("Iris_IrisMesh", VSName, GSName, PSName, CSName);
    m_TextureManager->swap("AlbedoTex", m_TextureManager->getSRV("MHBoneTex"));
	m_Context->PSSetShaderResources(0, (UINT)m_TextureManager->getSRVs().size(), &m_TextureManager->getSRVs()[0]);
    draw("Bones_BonesMesh", VSName, GSName, PSName, CSName);
    draw("Teeth_TeethMesh", VSName, GSName, PSName, CSName);
}
