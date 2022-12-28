#include "DXLib/Context.h"
#include <cmath>
#include "DXLib/MakeHuman.h"
#include "DXLib/InfiniteScan.h"
#include "DXLib/Testcase.h"
#include "../../extern/Pfm/Pfm.h"
#include <cstdlib>
#include "DXLib/UAVTexture.h"

#define RECALCSHADOWMAPEVERYFRAME 0
#define FRAMEBUFFERFORMAT     LINEARBUFFERFORMAT
#define BATCHMODE             0

#if BATCHMODE
	#define FRAMEBUFFERWIDTH      4096
	#define FRAMEBUFFERHEIGHT     FRAMEBUFFERWIDTH
	#define SHADOWBUFFERWIDTH     1024
	#define SHADOWBUFFERHEIGHT    SHADOWBUFFERWIDTH
#else
	#define FRAMEBUFFERWIDTH      getClientWidth()
	#define FRAMEBUFFERHEIGHT     getClientHeight()
	#define SHADOWBUFFERWIDTH     FRAMEBUFFERWIDTH / 2.0f
	#define SHADOWBUFFERHEIGHT    FRAMEBUFFERHEIGHT / 2.0f
#endif
#define VIEWPORTWIDTH         FRAMEBUFFERWIDTH
#define VIEWPORTHEIGHT        FRAMEBUFFERHEIGHT

using namespace SubSurfaceScatter;

Context::Context(HINSTANCE hInstance)
		: BasicWindow(hInstance),
		  m_Device(NULL), m_Context(NULL),
		  m_d3dSwapChain(NULL), m_d3dRenderTargetView(NULL),
		  m_d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0),
		  m_d3dDepthStencilView(NULL),
		  m_isEnabled(false),
		  m_lightsUpdated(false),
		  m_modelUpdate(false),
		  m_cameraUpdate(false),
		  m_windowSizeUpdate(false),
		  m_shaderMenuUpdate(false),
		  m_algorithmUpdate(false),
		  m_ModelObject(NULL),
		  m_Quads(NULL),
		  m_SaveImageIndex(0),
		  m_ImageCounter(0),
          m_algo(0)
{
	m_TransmittanceScales[0] = 0;
	m_TransmittanceScales[1] = 2;
	m_TransmittanceScales[2] = 0;
	m_TransmittanceScales[3] = 150;
	m_TransmittanceScales[4] = 150;
	HRESULT hr = init();
	chkError(hr);
	if(FAILED(hr)) {
		PostQuitMessage(0);
	}
}

Context::~Context() {
	release();
}

HRESULT Context::init() {
	LOGG<<"Initializing ...";
	HRESULT hr = initD3D();
	if(FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Context::initD3D() {

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( m_hWnd, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
    #if defined(_DEBUG) || defined(DEBUG)
	    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	// swap chain, device, context
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = FRAMEBUFFERWIDTH;
	sd.BufferDesc.Height = FRAMEBUFFERHEIGHT;
	sd.BufferDesc.Format = FRAMEBUFFERFORMAT;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = MULTISAMPLINGLEVEL; // multisampling
	sd.SampleDesc.Quality = MULTISAMPLINGQUALITY;
	sd.Windowed = !isMaximized();
	
	HR(chkHr(D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_HARDWARE, NULL,
			createDeviceFlags,
			featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd,
			&m_d3dSwapChain,
			&m_Device,
			&m_d3dFeatureLevel,
			&m_Context)));
	d3dLiveName(m_d3dSwapChain, "Swap Chain");
	d3dLiveName(m_Device, "DirectX Device");
	d3dLiveName(m_Context, "DirectX Device Context");

	

	// setup camera
	// overwritten by menu, but needs an initial position
	m_Camera.setPerspective(62, (float)getWidth()/getHeight(), 200, 1200);
	m_Camera.setTranslation(D3DXVECTOR3(0, 0, 500));



	// setup light sources
	// overwritten by menu, but needs an initial position
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		LightSource *LS = new LightSource;
		LS->setParameters(62, 4/3, 200, 1000);
		LS->setTranslation(D3DXVECTOR3(0, 0, -600));
		m_LightSources.push_back(LS);
	}


	initTextures();

	initStencils();
	

	// init shaders
	// has to be called before adding application defined shader macros
	m_ShaderManager = ShaderManager::getInstance();
	if(!m_ShaderManager->isInitialized())
		m_ShaderManager->init(m_Device);
	

	// macros for shaders
	std::vector<ShaderMacro> Macros;
	m_TextureManager->getShaderMacros(Macros);
	
	ShaderMacro SM;
	SM.Name = "NUMLIGHTSOURCES";
	SM.Definition = num2str(NUMLIGHTSOURCES);
	Macros.push_back(SM);
	
	SM.Name = "TRANSMITTANCETEXSIZE";
	SM.Definition = num2str(TRANSMITTANCETEXSIZE);
	Macros.push_back(SM);

	SM.Name = "NUMDEPTHPEELING";
	SM.Definition = num2str(NUMDEPTHPEELING);
	Macros.push_back(SM);

	SM.Name = "SRGBA";
	SM.Definition = num2str(!USEHDR);
	Macros.push_back(SM);
	
	m_ShaderManager->addShaderMacros(Macros);
	


	LOGG<<"\tLoading menu";
	m_Menu = Menu::getInstance();
	m_Menu->init(m_Device, FRAMEBUFFERWIDTH, FRAMEBUFFERHEIGHT);

	// resize, render targets, back buffer
	HR(resize());


	// loading a model
	updateModelMesh();



	// constant buffers
	// 0: buffer for cameras
	D3D11_BUFFER_DESC bufferDesc;

	ZEROMEM(bufferDesc);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.ByteWidth = sizeof(cbPerCamera);
	HR(chkHr(m_Device->CreateBuffer(&bufferDesc, NULL, &m_d3dBuffers["cbPerCamera"])));
	d3dLiveName(m_d3dBuffers["cbPerCamera"], "constant buffer for a camera");

	// 1: buffer for lights
	bufferDesc.ByteWidth = sizeof(cbPerLight);
	HR(chkHr(m_Device->CreateBuffer(&bufferDesc, NULL, &m_d3dBuffers["cbPerLight"])));
	d3dLiveName(m_d3dBuffers["cbPerLight"], "constant buffer for a light");
	
	// 2: buffer for models
	bufferDesc.ByteWidth = sizeof(cbPerModel);
	HR(chkHr(m_Device->CreateBuffer(&bufferDesc, NULL, &m_d3dBuffers["cbPerModel"])));
	d3dLiveName(m_d3dBuffers["cbPerModel"], "constant buffer for a model");

	// 3: buffer for shader settings
	bufferDesc.ByteWidth = sizeof(cbShaderSettings);
	HR(chkHr(m_Device->CreateBuffer(&bufferDesc, NULL, &m_d3dBuffers["cbShaderSettings"])));
	d3dLiveName(m_d3dBuffers["cbShaderSettings"], "constant buffer for shader settings");


	initShaders();


	initQuads();

	// light sphere
	for(UINT i=0; i< NUMLIGHTSOURCES; ++i) {
		Sphere *L = new Sphere();
		L->init(m_Device, m_Context, 10, 20, 20);
		L->setShaders("VS_LightSphere"+num2str(i), "", "PS_LightSphere"+num2str(i), "");
		m_LightSpheres.push_back(L);
	}



	// rasterizer states
	// 1: No culling at all
	D3D11_RASTERIZER_DESC rastDesc;
	ZEROMEM(rastDesc);
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	ID3D11RasterizerState *RS = NULL;
	m_Device->CreateRasterizerState(&rastDesc, &RS);
	m_Rasterizers["NoCulling"] = RS;

	// 2: BackfaceCulling
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	RS = NULL;
	m_Device->CreateRasterizerState(&rastDesc, &RS);
	m_Rasterizers["BackfaceCulling"] = RS;

	// 3: BackfaceCulling
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	RS = NULL;
	m_Device->CreateRasterizerState(&rastDesc, &RS);
	m_Rasterizers["Wireframe"] = RS;

	// standard rasterizer
	m_Context->RSSetState(m_Rasterizers["BackfaceCulling"]);



	// set textures and samplers
	std::vector<ID3D11ShaderResourceView*> SRV = m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

	std::vector<ID3D11SamplerState*> Samplers = m_TextureManager->getSamplers();
	m_Context->PSSetSamplers(0, (UINT)Samplers.size(), &Samplers[0]);

	
	// set IL, defined by ModelObject
	m_Context->IASetInputLayout(m_ModelObject->getInputLayout());


	// prerender lookup textures
	LOGG<<"\tRendering lookup textures";

	// make sure an input layout is defined
	// we use only one IL anyways
	m_Context->IASetInputLayout(m_ModelObject->getInputLayout());

	prerenderTextures();

	LOGG<<"Initialization complete";
	m_isEnabled = true;
	
	return hr;
}

void Context::initTextures() {
	// textures
	m_TextureManager = TextureManager::getInstance();
	if(!m_TextureManager->isInitialized())
		m_TextureManager->init(m_Device, m_Context);

	// register textures, actual textures can be added later
	D3DX11_IMAGE_LOAD_INFO ImageInfo;
	ZeroMemory(&ImageInfo, sizeof(ImageInfo));
	ImageInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ImageInfo.CpuAccessFlags = 0;
	ImageInfo.Format = LINEARBUFFERFORMAT;
	ImageInfo.Usage = D3D11_USAGE_IMMUTABLE;
	ImageInfo.MipLevels = 1;
	ImageInfo.Width = D3DX11_DEFAULT;
	ImageInfo.Height = D3DX11_DEFAULT;
	ImageInfo.Depth = D3DX11_DEFAULT;
	ImageInfo.FirstMipLevel = 0;
	ImageInfo.MiscFlags = 0;
	ImageInfo.MipFilter = D3DX11_DEFAULT;
	ImageInfo.Filter = D3DX11_DEFAULT;

	ImageInfo.Format = LINEARBUFFERFORMAT;
	m_TextureManager->addTexture("NullTex", L"../../data/NullTex.png", ImageInfo); // force a texture to null, might be usefull when debugging
	ImageInfo.Format = COLORBUFFERFORMAT;
	m_TextureManager->addTexture("AlbedoTex", NULL); // dummy textures, will load actual textures in these registers
	ImageInfo.Format = LINEARBUFFERFORMAT;
	m_TextureManager->addTexture("NormalTex", NULL);
	m_TextureManager->addTexture("SpecTex", NULL);
	ImageInfo.Format = COLORBUFFERFORMAT;
	m_TextureManager->addTexture("GammaTex", L"../../data/gamma.png", ImageInfo);
	// prerendered textures
	ImageInfo.Format = LINEARBUFFERFORMAT;
	m_TextureManager->addTexture("Rho_dTex", L"../../data/rho_d.png", ImageInfo);
	m_TextureManager->addTexture("TransmittanceTex", NULL);
    m_TextureManager->addTexture("NormalMap", NULL);
	m_TextureManager->addTexture("VertexNormalMap", NULL);
	// runtime textures, need to be unbound at the end of each render cycle
    ImageInfo.Format = HIGHRESBUFFERFORMAT;
	m_TextureManager->addTexture("PositionMap", NULL);
	ImageInfo.Format = LINEARBUFFERFORMAT;
	//m_TextureManager->addTexture("MaterialMap", NULL);
	m_TextureManager->addTexture("SpecMap", NULL);
	m_TextureManager->addTexture("TexCoordMap", NULL);
    ImageInfo.Format = DEPTHONLYBUFFERFORMAT;
	m_TextureManager->addTexture("DepthMap", NULL);
	ImageInfo.Format = COLORBUFFERFORMAT;
    m_TextureManager->addTexture("TransmittanceMap", NULL);
    m_TextureManager->addTexture("TransmittanceBlurXMap", NULL);
    m_TextureManager->addTexture("TransmittanceBlurYMap", NULL);
    m_TextureManager->addTexture("PostDiffuseMap", NULL);
	
	// order is of importance!
	// need to preserve array order
	ImageInfo.Format = COLORBUFFERFORMAT;
    m_TextureManager->addTexture("AlbedoMap", NULL);
    m_TextureManager->addTexture("StretchMap", NULL);
        
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i)
		m_TextureManager->addTexture("ShadowAlbedo"+num2str(i)+"d0", NULL);
	ImageInfo.Format = DEPTHONLYBUFFERFORMAT;
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i)    
		for(UINT j=0; j<NUMDEPTHPEELING; ++j)
			m_TextureManager->addTexture("ShadowMap"+num2str(i)+"d"+num2str(j), NULL);
	ImageInfo.Format = LINEARBUFFERFORMAT;
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		m_TextureManager->addTexture("ShadowNormal"+num2str(i)+"d0", NULL);
		m_TextureManager->addTexture("ShadowNormal2"+num2str(i)+"d0", NULL);
	}
	ImageInfo.Format = LINEARBUFFERFORMAT;
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i)
		m_TextureManager->addTexture("TSM"+num2str(i), NULL);
	ImageInfo.Format = COLORBUFFERFORMAT;
	m_TextureManager->addTexture("DiffuseMap", NULL);
	m_TextureManager->addTexture("SpecularMap", NULL);
	for(UINT i=1; i<NUMBLURPASSES; ++i) {
		m_TextureManager->addTexture("BlurX"+num2str(i)+"Map", NULL);
		m_TextureManager->addTexture("BlurY"+num2str(i)+"Map", NULL);
	}
	m_TextureManager->addTexture("FinalSSSMap", NULL);

	m_TextureManager->addTexture("PPBrightnessMap", NULL);
	m_TextureManager->addTexture("PPBloomXMap", NULL);
	m_TextureManager->addTexture("PPBloomYMap", NULL);
	m_TextureManager->addTexture("PPCombineMap", NULL);
    

	// Samplers
	// 0: linear sampler
	D3D11_SAMPLER_DESC SamplerDesc;
	ZEROMEM(SamplerDesc);
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_TextureManager->addSampler("SamplerLinear", SamplerDesc);

	// 1: point sampler
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	m_TextureManager->addSampler("SamplerPoint", SamplerDesc);
}

void Context::initStencils() {
	// stencils
	// mark skin stencil
	D3D11_DEPTH_STENCIL_DESC StencilDesc;
	StencilDesc.DepthEnable = true;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	StencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	StencilDesc.StencilEnable = true;
	StencilDesc.StencilReadMask = 0xff;
	StencilDesc.StencilWriteMask = 0xff;
	StencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	StencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;

	ID3D11DepthStencilState *DSS = NULL;
	m_Device->CreateDepthStencilState(&StencilDesc, &DSS);
	m_DSSs["MarkPixels"] = DSS;


	// skin select stencil
	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	StencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
	StencilDesc.StencilEnable = true;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	DSS = NULL;
	m_Device->CreateDepthStencilState(&StencilDesc, &DSS);
	m_DSSs["SelectPixels"] = DSS;


	// default quad stencil
	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	StencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
	StencilDesc.StencilEnable = false;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;

	DSS = NULL;
	m_Device->CreateDepthStencilState(&StencilDesc, &DSS);
	m_DSSs["DoNothing"] = DSS;


	// depth peeling stencil
	StencilDesc.DepthEnable = true;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	StencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	StencilDesc.StencilEnable = false;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;

	DSS = NULL;
	m_Device->CreateDepthStencilState(&StencilDesc, &DSS);
	m_DSSs["DepthPeeling"] = DSS;


	// select skin and non skin
	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	StencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
	StencilDesc.StencilEnable = true;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;

	DSS = NULL;
	m_Device->CreateDepthStencilState(&StencilDesc, &DSS);
	m_DSSs["SelectEverythingFrom"] = DSS;
}

void Context::initQuads() {
	m_Quads = QuadManager::getInstance();

	// quads
	// g-buffer
	m_Quad.init(m_Device, m_Context, 2, 2, 1);
	// position map
	RenderToTexture *QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, HIGHRESBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture PositionMap");
	m_Quads->add("PositionMap", QT);
	// normal map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture NormalMap");
	m_Quads->add("NormalMap", QT);
	// albedo map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture AlbedoMap");
	m_Quads->add("AlbedoMap", QT);
	// spec map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture SpecMap");
	m_Quads->add("SpecMap", QT);
	// vertex normal map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture VertexNormalMap");
	m_Quads->add("VertexNormalMap", QT);
	// material map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture MaterialMap");
	m_Quads->add("MaterialMap", QT);
	// depth map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, DEPTHONLYBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture DepthMap");
	m_Quads->add("DepthMap", QT);

	// deferred shading
	// diffuse map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture DiffuseMap");
	m_Quads->add("DiffuseMap", QT);
	// specular map
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture SpecularMap");
	m_Quads->add("SpecularMap", QT);

	// beckmann texture
	//QT = new RenderToTexture();
	//QT->init(m_Device, m_Context, 512, 512, DEPTHONLYBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture Beckmann");
	//m_Quads->add("BeckmannTexture", QT);

	// prerendered transmittance
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TRANSMITTANCETEXSIZE, 1, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture Transmittance");
	m_Quads->add("TransmittanceTexture", QT);
	
	// shadowmap
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		for(UINT j=0; j<NUMDEPTHPEELING; ++j) {
			QT = new RenderToTexture();
			QT->init(m_Device, m_Context, SHADOWMAPWIDTH, SHADOWMAPHEIGHT, DEPTHONLYBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture ShadowMap"+num2str(i)+"d"+num2str(j));
			m_Quads->add("ShadowMap"+num2str(i)+"d"+num2str(j), QT);
		}
		QT = new RenderToTexture();
		QT->init(m_Device, m_Context, SHADOWMAPWIDTH, SHADOWMAPHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture ShadowAlbedo"+num2str(i)+"d0");
		m_Quads->add("ShadowAlbedo"+num2str(i)+"d0", QT);

		QT = new RenderToTexture();
		QT->init(m_Device, m_Context, SHADOWMAPWIDTH, SHADOWMAPHEIGHT, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture ShadowNormals"+num2str(i)+"d0");
		m_Quads->add("ShadowNormal"+num2str(i)+"d0", QT);
		QT = new RenderToTexture();
		QT->init(m_Device, m_Context, SHADOWMAPWIDTH, SHADOWMAPHEIGHT, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture ShadowNormals"+num2str(i)+"d0");
		m_Quads->add("ShadowNormal2"+num2str(i)+"d0", QT);
		QT = new RenderToTexture();
		QT->init(m_Device, m_Context, SHADOWMAPWIDTH, SHADOWMAPHEIGHT, HIGHRESBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture TSM"+num2str(i));
		m_Quads->add("TSM"+num2str(i), QT);
	}

	// blurXY
	for(UINT i=0; i<NUMBLURPASSES; ++i) {
		// blur x
		QT = new RenderToTexture();
		QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture BlurX"+num2str(i));
		m_Quads->add("BlurX"+num2str(i)+"Map", QT);
		// blur y
		QT = new RenderToTexture();
		QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture BlurY"+num2str(i));
		m_Quads->add("BlurY"+num2str(i)+"Map", QT);
	}

	// final sss mix
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture FinalSSS");
	m_Quads->add("FinalSSSMap", QT);

	// post process
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH/2, TEXTUREHEIGHT/2, LINEARBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture PPBrightness");
	m_Quads->add("PPBrightnessMap", QT);
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH/2, TEXTUREHEIGHT/2, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture PPBloomX");
	m_Quads->add("PPBloomXMap", QT);
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH/2, TEXTUREHEIGHT/2, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture PPBloomY");
	m_Quads->add("PPBloomYMap", QT);
	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH/2, TEXTUREHEIGHT/2, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture PPCombine");
	m_Quads->add("PPCombineMap", QT);

	QT = new RenderToTexture();
	QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, HIGHRESBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP | RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP, "QuadTexture StretchMap");
	m_Quads->add("StretchMap", QT);

    QT = new RenderToTexture();
    QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, HIGHRESBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture TransmittanceMap");
	m_Quads->add("TransmittanceMap", QT);
    QT = new RenderToTexture();
    QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture TransmittanceBlurXMap");
	m_Quads->add("TransmittanceBlurXMap", QT);
    QT = new RenderToTexture();
    QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture TransmittanceBlurYMap");
	m_Quads->add("TransmittanceBlurYMap", QT);
    QT = new RenderToTexture();
    QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, COLORBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture PostDiffuseMap");
	m_Quads->add("PostDiffuseMap", QT);
	QT = new RenderToTexture();
    QT->init(m_Device, m_Context, TEXTUREWIDTH, TEXTUREHEIGHT, HIGHRESBUFFERFORMAT, RENDERTOTEXTURE_OPTIONS_ISRESIZABLE | RENDERTOTEXTURE_OPTIONS_USECOLORMAP, "QuadTexture TexCoordMap");
	m_Quads->add("TexCoordMap", QT);
}

void Context::initShaders() {
	// shaders
	// init quad
	ID3DBlob *Blob = NULL;
	// simple quad shaders
	m_ShaderManager->createVS("VS_Quad", L"../../shaders/VS_Quad.hlsl", "Main", &Blob);
	safe_delete(Blob);
	Blob = NULL;
	m_ShaderManager->createPS("PS_Quad", L"../../shaders/PS_Quad.hlsl", "Main");

	m_ShaderManager->createVS("VS_UnwrapMesh", L"../../shaders/VS_UnwrapMesh.hlsl", "Main", &Blob);
	safe_delete(Blob);
	Blob = NULL;
		
	// deferred shading
	m_ShaderManager->createPS("PS_GeometryPass", L"../../shaders/PS_GeometryPass.hlsl", "Main");
	//m_ShaderManager->createPS("PS_TSGeometryPass", L"../../shaders/PS_TSGeometryPass.hlsl", "Main");
	m_ShaderManager->createPS("PS_Dilate_dEonIrradiance", L"../../shaders/PS_Dilate.hlsl", "MainDiffuse");
	m_ShaderManager->createPS("PS_Dilate_BrandShadowNormals", L"../../shaders/PS_Dilate.hlsl", "MainBrandShadowNormals");

	m_ShaderManager->createPS("PS_Brand_LightPass", L"../../shaders/PS_Brand_LightPass.hlsl", "Main");
    m_ShaderManager->createPS("PS_Brand_TransmittanceBlurX", L"../../shaders/PS_Brand_TransmittanceBlur.hlsl", "MainX");
    m_ShaderManager->createPS("PS_Brand_TransmittanceBlurY", L"../../shaders/PS_Brand_TransmittanceBlur.hlsl", "MainY");
    m_ShaderManager->createPS("PS_Brand_TransmittanceMix", L"../../shaders/PS_Brand_TransmittanceBlur.hlsl", "MixMain");
	m_ShaderManager->createPS("PS_Brand_Combine", L"../../shaders/PS_Brand_Combine.hlsl", "Main");

	// using rho_d map from nvidia human head demo
	// beckmann texture
	//m_ShaderManager->createPS("PS_Beckmann", L"../../shaders/PS_BeckmannTexture.hlsl", "Main");

	// transmittance texture
	m_ShaderManager->createPS("PS_Transmittance", L"../../shaders/PS_Transmittance.hlsl", "Main");

	// shadow map
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		Blob = NULL;
		m_ShaderManager->createVS("VS_ShadowPass"+num2str(i), L"../../shaders/VS_ShadowPass.hlsl", "Main"+num2str(i), &Blob);
		for(UINT j=0; j<NUMDEPTHPEELING; ++j)
			m_ShaderManager->createPS("PS_ShadowPeelingPass"+num2str(i)+"d"+num2str(j), L"../../shaders/PS_ShadowPeelingPass.hlsl", "Main"+num2str(i)+"d"+num2str(j));
		m_ShaderManager->createPS("PS_dEon_ShadowPass"+num2str(i), L"../../shaders/PS_dEon_ShadowPass.hlsl", "Main"+num2str(i));
		m_ShaderManager->createPS("PS_ShadowPass"+num2str(i), L"../../shaders/PS_ShadowPass.hlsl", "Main"+num2str(i));
	}

	// no sss pass
	m_ShaderManager->createPS("PS_NoSSS_renderPass", L"../../shaders/PS_NoSSS_renderPass.hlsl", "Main");
	m_ShaderManager->createPS("PS_Skeleton", L"../../shaders/PS_Skeleton.hlsl", "Main");

	// blur shaders
	for(UINT i=1; i<NUMBLURPASSES; ++i) {
		m_ShaderManager->createPS("PS_dEon_BlurX"+num2str(i), L"../../shaders/PS_dEon_Blur.hlsl", "MainX"+num2str(i));
		m_ShaderManager->createPS("PS_dEon_BlurY"+num2str(i), L"../../shaders/PS_dEon_Blur.hlsl", "MainY"+num2str(i));
	}
	for(UINT i=1; i<4; ++i) {
		m_ShaderManager->createPS("PS_Jimenez_BlurX"+num2str(i), L"../../shaders/PS_Jimenez_Blur.hlsl", "BlurX"+num2str(i));
		m_ShaderManager->createPS("PS_Jimenez_BlurY"+num2str(i), L"../../shaders/PS_Jimenez_Blur.hlsl", "BlurY"+num2str(i));
        m_ShaderManager->createPS("PS_Brand_BlurX"+num2str(i), L"../../shaders/PS_Brand_Blur.hlsl", "BlurX"+num2str(i));
		m_ShaderManager->createPS("PS_Brand_BlurY"+num2str(i), L"../../shaders/PS_Brand_Blur.hlsl", "BlurY"+num2str(i));
	}
	
	// combine blurs
	m_ShaderManager->createPS("PS_Jimenez_Combine", L"../../shaders/PS_Jimenez_Combine.hlsl", "Main");

	// post process
	m_ShaderManager->createPS("PS_PPBrightPass", L"../../shaders/PS_PostProcess.hlsl", "BrightPass");
	m_ShaderManager->createPS("PS_PPBloomX", L"../../shaders/PS_PostProcess.hlsl", "BloomX");
	m_ShaderManager->createPS("PS_PPBloomY", L"../../shaders/PS_PostProcess.hlsl", "BloomY");
	m_ShaderManager->createPS("PS_PPCombine", L"../../shaders/PS_PostProcess.hlsl", "Combine");

	// light sphere
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		Blob = NULL;
		m_ShaderManager->createVS("VS_LightSphere"+num2str(i), L"../../shaders/VS_LightSphere.hlsl", "Main"+num2str(i), &Blob);
		safe_delete(Blob);
		m_ShaderManager->createPS("PS_LightSphere"+num2str(i), L"../../shaders/PS_LightSphere.hlsl", "Main"+num2str(i));
	}

	// dEon
	m_ShaderManager->createPS("PS_dEon_IrradiancePass", L"../../shaders/PS_dEon_IrradiancePass.hlsl", "Main");
	m_ShaderManager->createPS("PS_dEon_StretchPass",    L"../../shaders/PS_dEon_StretchPass.hlsl", "Main");
	m_ShaderManager->createPS("PS_dEon_Combine",    L"../../shaders/PS_dEon_Combine.hlsl", "Main");

	// Hable
	m_ShaderManager->createPS("PS_Hable_Combine", L"../../shaders/PS_Hable_Combine.hlsl", "Main");

	// Jimenez
	m_ShaderManager->createPS("PS_Jimenez_LightPass",    L"../../shaders/PS_Jimenez_LightPass.hlsl", "Main");
}

void Context::prerenderTextures() {
	// transmittance texture
	m_Quads->get("TransmittanceTexture")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
	m_Quad.setShaders("VS_Quad", "", "PS_Transmittance", "");
	m_Quad.draw();
	m_Quads->get("TransmittanceTexture")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("TransmittanceTex", m_Quads->get("TransmittanceTexture")->getColorMap());
}

void Context::doWork() {
	// we are in application loop
	if(m_isEnabled) {
		// update scene parameters
		updateShaderSettings();
		m_cameraUpdate = m_windowSizeUpdate || m_shaderMenuUpdate; // need to keep aspect ratio if window or texture size changes
		m_modelUpdate = false;
		m_lightsUpdated = m_windowSizeUpdate || m_shaderMenuUpdate; // aspect ratio or shadow map size
		if(m_algorithm_old != m_Menu->getParams()->SceneSettings.algorithm) {
			m_algorithmUpdate = true;
			m_algorithm_old = m_Menu->getParams()->SceneSettings.algorithm;
		} else
			m_algorithmUpdate = false;
		updateCameras();
		updateLights();
		updateModels();
		m_windowSizeUpdate = false;

		// rendering & drawing
		clear();
		render();
		swapBuffers();
	}
}

void Context::clear() {
	// cleanup
	m_Context->OMSetDepthStencilState(NULL, 0);
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_Context->OMSetBlendState(0, blendFactor, 0xffffffff);
	m_Context->ClearRenderTargetView(m_d3dRenderTargetView, m_Menu->getParams()->SceneSettings.BackgroundColor);
	m_Context->ClearDepthStencilView(m_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// bind/unbind textures
	std::vector<ID3D11ShaderResourceView*> &SRV = m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::swapBuffers() {
	HRESULT hr = S_OK;
	// swap front and backbuffer
	chkHr(m_d3dSwapChain->Present(0, 0));
}

void Context::resizeQuads() {
	// don't change size of loaded textures
	SceneMenuParams *SMS = &m_Menu->getParams()->SceneSettings;
	for(UINT i=0; i<m_LightSources.size(); ++i) {
		for(UINT j=0; j<NUMDEPTHPEELING; ++j) {
			m_Quads->get("ShadowMap"+num2str(i)+"d"+num2str(j))->resize(    (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.x, (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.y);
		}
		m_Quads->get("ShadowNormal"+num2str(i)+"d0")->resize( (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.x, (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.y);
		m_Quads->get("ShadowNormal2"+num2str(i)+"d0")->resize( (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.x, (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.y);
		m_Quads->get("ShadowAlbedo"+num2str(i)+"d0")->resize( (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.x, (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.y);	
		m_Quads->get("TSM"+num2str(i))->resize((UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.x, (UINT)m_Menu->getParams()->ShaderSettings.ShadowMapSize.y);
	}
		
	m_Quads->get("PositionMap")->resize((UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("NormalMap")->resize(  (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("VertexNormalMap")->resize(  (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("MaterialMap")->resize(  (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("AlbedoMap")->resize(  (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y); // also resizes depth & stencil map
	m_Quads->get("SpecMap")->resize(    (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("DiffuseMap")->resize( (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("DepthMap")->resize(   (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	m_Quads->get("SpecularMap")->resize((UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
    for(UINT i=1; i<NUMBLURPASSES; ++i) {
		m_Quads->get("BlurX"+num2str(i)+"Map")->resize(  (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
		m_Quads->get("BlurY"+num2str(i)+"Map")->resize(  (UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	}
	m_Quads->get("FinalSSSMap")->resize((UINT)SMS->RenderSize.x,     (UINT)SMS->RenderSize.y);
	// post process only on smaller quad
	m_Quads->get("PPBrightnessMap")->resize((UINT)SMS->RenderSize.x/2,   (UINT)SMS->RenderSize.y/2);
	m_Quads->get("PPBloomXMap")->resize((UINT)SMS->RenderSize.x/2,   (UINT)SMS->RenderSize.y/2);
	m_Quads->get("PPBloomYMap")->resize((UINT)SMS->RenderSize.x/2,   (UINT)SMS->RenderSize.y/2);
	m_Quads->get("PPCombineMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
	m_Quads->get("StretchMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
    m_Quads->get("TransmittanceMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
    m_Quads->get("TransmittanceBlurXMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
    m_Quads->get("TransmittanceBlurYMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
    m_Quads->get("PostDiffuseMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
	m_Quads->get("TexCoordMap")->resize((UINT)SMS->RenderSize.x,    (UINT)SMS->RenderSize.y);
}

void Context::unbindQuads() {
	m_TextureManager->addTexture("PositionMap", NULL);
	m_TextureManager->addTexture("NormalMap", NULL);
	m_TextureManager->addTexture("AlbedoMap", NULL);
	m_TextureManager->addTexture("MaterialMap", NULL);
	m_TextureManager->addTexture("VertexNormalMap", NULL);
	m_TextureManager->addTexture("SpecMap", NULL);
	m_TextureManager->addTexture("DepthMap", NULL);
	m_TextureManager->addTexture("DiffuseMap", NULL);
	m_TextureManager->addTexture("SpecularMap", NULL);
	m_TextureManager->addTexture("StretchMap", NULL);
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		for(UINT j=0; j<NUMDEPTHPEELING; ++j) {
			m_TextureManager->addTexture("ShadowMap"+num2str(i)+"d"+num2str(j), NULL);
		}
		m_TextureManager->addTexture("ShadowNormal"+num2str(i)+"d0", NULL);
		m_TextureManager->addTexture("ShadowNormal2"+num2str(i)+"d0", NULL);
		m_TextureManager->addTexture("ShadowAlbedo"+num2str(i)+"d0", NULL);
		m_TextureManager->addTexture("TSM"+num2str(i), NULL);
	}
	for(UINT i=0; i<NUMBLURPASSES; ++i) {
		m_TextureManager->addTexture("BlurX"+num2str(i)+"Map", NULL);
		m_TextureManager->addTexture("BlurY"+num2str(i)+"Map", NULL);
	}
	m_TextureManager->addTexture("FinalSSSMap", NULL);
	m_TextureManager->addTexture("PPBrightnessMap", NULL);
	m_TextureManager->addTexture("PPBloomXMap", NULL);
	m_TextureManager->addTexture("PPBloomYMap", NULL);
	m_TextureManager->addTexture("PPCombineMap", NULL);
	m_TextureManager->addTexture("StretchMap", NULL);
    m_TextureManager->addTexture("TransmittanceMap", NULL);
    m_TextureManager->addTexture("TransmittanceBlurXMap", NULL);
    m_TextureManager->addTexture("TransmittanceBlurYMap", NULL);
    m_TextureManager->addTexture("PostDiffuseMap", NULL);
	m_TextureManager->addTexture("TexCoordMap", NULL);
}

HRESULT Context::saveCurrentImage(const std::string &Filename) {
	HRESULT hr = S_OK;
	if(Filename != "") {
		LOGG<<"Saving current framebuffer as '"+Filename+"'";
		#if USEHDR

			PFM HDRExport = PFM();

			UINT numChannels = 4;
						
			// get resource from SRV
			const std::string TextureName = m_TextureManager->getTextureNames()[m_Menu->getParams()->ShaderSettings.textureSelected];
			ID3D11ShaderResourceView *SRV = m_Quads->get(TextureName)->getColorMap();
			ID3D11Resource *Res = NULL;
			SRV->GetResource(&Res);
			ID3D11Texture2D *Tex = (ID3D11Texture2D*)Res;

			D3D11_TEXTURE2D_DESC TexDesc;
			Tex->GetDesc(&TexDesc);
			TexDesc.Usage          = D3D11_USAGE_STAGING;
			TexDesc.BindFlags      = 0;
			TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			TexDesc.MiscFlags      = 0;

			ID3D11Texture2D *Tmp = NULL;
			chkHr(m_Device->CreateTexture2D(&TexDesc, NULL, &Tmp));

			m_Context->CopyResource(Tmp, Tex);

			UINT numSubRes = 0;
			D3D11_MAPPED_SUBRESOURCE MappedRes;
			chkHr(m_Context->Map(Tmp, numSubRes, D3D11_MAP_READ, 0, &MappedRes));
		
			UINT mipScale = 1;
			for (UINT i = 0; i < numSubRes; i++)
				mipScale *= 2;

			HDRExport.width = TexDesc.Width  / mipScale;
			HDRExport.height = TexDesc.Height / mipScale;

			UINT dstPitch = HDRExport.width * numChannels * sizeof(float);
			HDRExport.pImg = new float[dstPitch * HDRExport.height];
			memset(HDRExport.pImg, 0, dstPitch * HDRExport.height);
			UCHAR *dstData = (UCHAR*)HDRExport.pImg + dstPitch * (HDRExport.height - 1); // last line

			UINT srcPitch = MappedRes.RowPitch;
			UCHAR *srcData = (UCHAR*)MappedRes.pData;

			for (UINT y = 0; y < (UINT)HDRExport.height; ++y) {
				memcpy(dstData, srcData, dstPitch); // copy one line
				dstData -= dstPitch; // bottom to top
				srcData += srcPitch; // top to bottom
			}
			m_Context->Unmap(Tmp, numSubRes);
			safe_delete(Tmp);

			if(!HDRExport.SaveRGBA((Filename+".pfm").c_str()))
				LOGG << "Could not save image as "+Filename+".pfm";
		
		#else
			// store framebuffer
			bool menuStatus = m_Menu->getParams()->SceneSettings.showMenu;
			m_Menu->getParams()->SceneSettings.showMenu = false;
			ID3D11Resource *res = NULL;
			m_d3dRenderTargetView->GetResource(&res);
			hr = D3DX11SaveTextureToFile(m_Context, res, D3DX11_IFF_PNG, str2wstr(Filename+".png").c_str());
			safe_delete(res);
			m_Menu->getParams()->SceneSettings.showMenu = menuStatus;
		#endif
	}
	return hr;
}

std::string Context::updateSceneSettings(const UINT index, const UINT counter, const UINT algo) {
	std::string Filename = "../../data/Renders/";
	std::string Infos = "";
    for(UINT i=1; i<m_Menu->getParams()->Lights.size(); ++i)
        m_Menu->getParams()->Lights[i].showLight = false;
	m_Menu->getParams()->SceneSettings.showMenu = false;
    m_Menu->getParams()->SceneSettings.RenderSize = D3DXVECTOR2((FLOAT)FRAMEBUFFERWIDTH, (FLOAT)FRAMEBUFFERHEIGHT);

    m_Menu->getParams()->ShaderSettings.ShadowMapSize = D3DXVECTOR2((FLOAT)SHADOWBUFFERWIDTH, (FLOAT)SHADOWBUFFERHEIGHT);
    m_Menu->getParams()->SceneSettings.drawNonSkin = true;
    m_Menu->getParams()->ShaderSettings.minBloom = 0.8f;
	m_Menu->getParams()->ShaderSettings.shadowMapEpsilon = 0.0025f;
    m_Menu->getParams()->ShaderSettings.stretchAlpha = 11;
	m_Menu->getParams()->ShaderSettings.stretchBeta = 700;
	std::string algoName;
    UINT a = algo % 5;
    switch(index) {

	case 0: // head back far
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.05f;
        m_Menu->getParams()->modelSelected = 2;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(0.92f, 0.02f, -0.39f, 0.01f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 33.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.5f, 0.02f, 0.86f, 0.02f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 36.0f;
        m_Menu->getParams()->Lights[0].intensity = 520000.0f;
        m_Menu->getParams()->Lights[1].showLight = true;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 35.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 6.0f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 500.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 400;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 200.0f;
			algoName = "Brand";
            break;
        }
        Infos = "Head_Light_behind_far_"+algoName;
        break;

    case 1: // head back near
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.05f;
        m_Menu->getParams()->modelSelected = 2;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(0.92f, 0.02f, -0.39f, 0.01f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(20.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 15.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.5f, 0.02f, 0.86f, 0.02f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 36.0f;
        m_Menu->getParams()->Lights[0].intensity = 520000.0f;
        m_Menu->getParams()->Lights[1].showLight = true;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 6.0f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 500.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 400.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 200.0f;
			algoName = "Brand";
            break;
        }
        Infos = "Head_Light_behind_near_"+algoName;
        break;


    case 2: // head side far
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.05f;
        m_Menu->getParams()->modelSelected = 2;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(0.92f, 0.02f, -0.39f, 0.01f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 33.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.9f, 0.01f, 0.43f, -0.08f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 36.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[1].showLight = false;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 40.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 6.0f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 500.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 400.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 200.0f;
			algoName = "Brand";
            break;
        }
        Infos = "Head_Light_side_far_"+algoName;
        break;

    case 3: // head side near
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.05f;
        m_Menu->getParams()->modelSelected = 2;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(0.92f, 0.02f, -0.39f, 0.01f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 18.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.9f, 0.01f, 0.43f, -0.08f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 37.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[1].showLight = false;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 40.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 6.0f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 500.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 400.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 200.0f;
			algoName = "Brand";
            break;
        }
        Infos = "Head_Light_side_near_"+algoName;
        break;




    case 4: // hand open back near
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 3;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 30.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.0f, 0.0f, 0.1f, 0.0f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 40.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[1].showLight = true;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 2.5f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 200.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 120.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 150.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandOpen_Light_behind_near_"+algoName;
        break;

    case 5: // hand open side near
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 3;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 30.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.86f, -0.01f, 0.5f, -0.1f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 35.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[1].showLight = false;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 2.5f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 200.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 120.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 150.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandOpen_Light_side_near_"+algoName;
        break;



	case 6: // hand open back far
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 3;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 60.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.0f, 0.0f, 0.1f, 0.0f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 60.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[1].showLight = true;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 60.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 2.5f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 200.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 120.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 150.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandOpen_Light_behind_far_"+algoName;
        break;

    case 7: // hand open side far
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 3;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 60.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(0.86f, -0.01f, 0.5f, -0.1f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(10.0f, 25.0f, 500.0f);
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1000.0f;
        m_Menu->getParams()->Lights[0].fovy = 60.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[1].showLight = false;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 60.0f;
        m_Menu->getParams()->Lights[1].intensity = 2000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 2.5f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 200.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 120.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 15.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 150.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandOpen_Light_side_far_"+algoName;
        break;



    case 8: // hand closed back far
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 4;
        m_Menu->getParams()->SceneSettings.lightSphereSize = 35;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(0.04f, -0.94f, 0.33f, -0.1f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, -25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 50.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.12f, 0.99f, 0.01f, 0.02f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(10.0f, 50.0f, -50.0f);
        m_Menu->getParams()->Lights[0].fovy = 135.0f;
        m_Menu->getParams()->Lights[0].intensity = 5000.0f;
        m_Menu->getParams()->Lights[0].zNear = 1.0f;
        m_Menu->getParams()->Lights[0].zFar = 400.0f;
        m_Menu->getParams()->Lights[1].showLight = true;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(-0.14f, -0.85f, 0.48f, 0.17f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 1000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 1.5f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 100.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 40.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 35.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 60.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandClosed_Light_behind_far_"+algoName;
        break;

    case 9: // hand closed back near
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 4;
        m_Menu->getParams()->SceneSettings.lightSphereSize = 35;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(0.04f, -0.94f, 0.33f, -0.1f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, -25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 26.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.12f, 0.99f, 0.01f, 0.02f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(10.0f, 50.0f, -50.0f);
        m_Menu->getParams()->Lights[0].fovy = 135.0f;
        m_Menu->getParams()->Lights[0].intensity = 5000.0f;
        m_Menu->getParams()->Lights[0].zNear = 1.0f;
        m_Menu->getParams()->Lights[0].zFar = 400.0f;
        m_Menu->getParams()->Lights[1].showLight = true;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(-0.14f, -0.85f, 0.48f, 0.17f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 1000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 1.5f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 100.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 40.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 35.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 60.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandClosed_Light_behind_near_"+algoName;
        break;



    case 10: // hand closed side far
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 4;
        m_Menu->getParams()->SceneSettings.lightSphereSize = 35;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(0.04f, -0.94f, 0.33f, -0.1f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, -25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 50.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(-0.11f, 0.83f, -0.51f, -0.19f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[0].fovy = 30.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1200.0f;
        m_Menu->getParams()->Lights[1].showLight = false;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(-0.14f, -0.85f, 0.48f, 0.17f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 1000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 4.0f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 400.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 300.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 20.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 200.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandClosed_Light_side_far_"+algoName;
        break;

    case 11: // hand closed side near
        m_Menu->getParams()->ShaderSettings.depthPeelingEpsilon = 0.0025f;
        m_Menu->getParams()->modelSelected = 4;
        m_Menu->getParams()->SceneSettings.lightSphereSize = 35;
        m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
        m_Menu->getParams()->Cameras[0].RotationGlobal = D3DXQUATERNION(0.04f, -0.94f, 0.33f, -0.1f);
        m_Menu->getParams()->Cameras[0].Translation = D3DXVECTOR3(0.0f, -25.0f, 500.0f);
        m_Menu->getParams()->Cameras[0].fovy = 26.0f;
        m_Menu->getParams()->Lights[0].showLight = true;
        m_Menu->getParams()->Lights[0].enableLight = true;
        m_Menu->getParams()->Lights[0].RotationGlobal = D3DXQUATERNION(-0.11f, 0.83f, -0.51f, -0.19f);
        m_Menu->getParams()->Lights[0].RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
        m_Menu->getParams()->Lights[0].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[0].fovy = 22.0f;
        m_Menu->getParams()->Lights[0].intensity = 120000.0f;
        m_Menu->getParams()->Lights[0].zNear = 200.0f;
        m_Menu->getParams()->Lights[0].zFar = 1200.0f;
        m_Menu->getParams()->Lights[1].showLight = false;
        m_Menu->getParams()->Lights[1].enableLight = true;
        m_Menu->getParams()->Lights[1].RotationGlobal = D3DXQUATERNION(-0.14f, -0.85f, 0.48f, 0.17f);
        m_Menu->getParams()->Lights[1].Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
        m_Menu->getParams()->Lights[1].fovy = 33.0f;
        m_Menu->getParams()->Lights[1].intensity = 1000.0f;
        m_Menu->getParams()->SceneSettings.algorithm = a;
        switch(a) {
		case 0:
			algoName = "NoSSS";
			break;
        case 1:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 4.0f;
			algoName = "dEon";
            break;
		case 2:
			algoName = "Hable";
			break;
        case 3:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 400.0f;
			algoName = "Jimenez";
            break;
        case 4:
            m_Menu->getParams()->ShaderSettings.transmittanceScale = 300.0f;
			m_Menu->getParams()->ShaderSettings.transmittanceBlur1 = 20.0f;
            m_Menu->getParams()->ShaderSettings.transmittanceBlur2 = 200.0f;
			algoName = "Brand";
            break;
        }
        Infos = "HandClosed_Light_side_near_"+algoName;
        break;

	default:
        exit(0);
		break;
	}

	return Filename+Infos;
}

void Context::render() {
	HRESULT hr = S_OK;

	// let's take some images before we switch to the next scene
	#if BATCHMODE
	    if(m_ImageCounter == 5) {
            m_ImageCounter = 0;
            m_algo++;
        }
        if(m_algo == 5) {
            m_algo = 0;
            m_SaveImageIndex++;
        }
	    std::string Filename = updateSceneSettings(m_SaveImageIndex, m_ImageCounter, m_algo);
        m_ImageCounter++;
	#endif

	// check if we need to load a new model
	updateModelMesh();

	SceneMenuParams *SMS = &m_Menu->getParams()->SceneSettings;

	// resize light spheres
	for(UINT i=0; i<m_LightSpheres.size(); ++i)
		m_LightSpheres[i]->setRadius(SMS->lightSphereSize);

	// resize quads
	resizeQuads();
	
	// render passes
	switch(m_Menu->getParams()->SceneSettings.algorithm) {
	case 0: // no sss
		shadowPass();
		NoSSS_renderPass();
		bloomPass();
		break;

	case 1: // dEon
		dEon_shadowPass();
		dEon_textureStretchPass();
		dEon_irradiancePass();
		dEon_dilateIrradiancePass();
		dEon_multiblurPass();
		dEon_combinePass();
		bloomPass();
		break;

	case 2: // Hable
		dEon_shadowPass();
		dEon_textureStretchPass();
		dEon_irradiancePass();
		dEon_dilateIrradiancePass();
		Hable_combinePass();
		bloomPass();
		break;

	case 3: // Jimenez
		// 1: Shadow maps
		shadowPass();

		// 2: g-buffer
		geometryPass();
	
		// 3: light pass in screen space
		// mrt for diffuse & specular light
		Jimenez_lightPass();
		
		// 4: blur xy
		Jimenez_multiblurPass();
	
		// 5: combine blurs
		Jimenez_combinePass();
	
		// 6: post processes
		bloomPass();
		break;

	case 4: // Brand
		// 1: Shadow maps
		Brand_shadowPeelingPass();

		Brand_dilateShadowNormalsPass();

		// 2: g-buffer
		geometryPass();
	
		// 3: light pass in screen space
		// mrt for diffuse & specular light, we blur only the diffuse part
		// also transmittance
		Brand_lightPass();

        // 4: blur transmittance
        Brand_transmittanceBlurPass();
		
		// 4: blur xy
		Brand_multiblurPass();
	
		// 5: combine blurs
		Brand_combinePass();
	
		// 6: post processes
		bloomPass();
		break;

	default:
		break;
	}
	
	// disable stencils
	m_Context->OMSetDepthStencilState(NULL, 0);

	// 7: display result	
	displayQuadPass();
	
	// draw anttweakbar menu
	if(m_Menu->getParams()->SceneSettings.showMenu)
		m_Menu->draw();

	#if BATCHMODE
        if(m_ImageCounter == 4)
	        saveCurrentImage(Filename);
	#endif

	// select textures for unbindung
	unbindQuads();
}

void Context::shadowPass() {
	for(size_t i=0; i<m_LightSources.size(); ++i) {
		if(m_lightsUpdated || m_modelUpdate || m_algorithmUpdate) {
			m_Context->OMSetDepthStencilState(NULL, 0);
			m_Quads->get("ShadowMap"+num2str(i)+"d0")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, m_Quads->get("ShadowAlbedo"+num2str(i)+"d0")->getDSV());

			// actual drawing
			if(m_Menu->getParams()->SceneSettings.drawSkin) {
				m_ModelObject->drawSkin("VS_ShadowPass"+num2str(i), "", "PS_ShadowPass"+num2str(i), "");
			}
			if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
				m_ModelObject->drawNonSkin("VS_ShadowPass"+num2str(i), "", "PS_ShadowPass"+num2str(i), "");
			}

			m_Quads->get("ShadowMap"+num2str(i)+"d0")->end();
			resetOMTargetsAndViewport();
		}
		m_TextureManager->addTexture("ShadowMap"+num2str(i)+"d0", m_Quads->get("ShadowMap"+num2str(i)+"d0")->getColorMap());
	}
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Brand_shadowPeelingPass() {
       // has something changed?
	if(m_lightsUpdated || m_modelUpdate || m_algorithmUpdate) {
		float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
		m_Context->RSSetState(m_Rasterizers["NoCulling"]);
		m_Context->OMSetDepthStencilState(m_DSSs["DepthPeeling"], 0);

		for(UINT light=0; light<NUMLIGHTSOURCES; ++light) {
			for(UINT peel=0; peel<NUMDEPTHPEELING; ++peel) {

				// set MRT
				ID3D11RenderTargetView **MRT_Shadow = new ID3D11RenderTargetView*[3];
				if(peel == 0) { // albedo and normals only needed for first layer
					MRT_Shadow[0] = m_Quads->get("ShadowAlbedo"+num2str(light)+"d0")->getRT(); // 0
					MRT_Shadow[1] = m_Quads->get("ShadowNormal"+num2str(light)+"d0")->getRT(); // 1
				} else {
					MRT_Shadow[0] = NULL; // 0
					MRT_Shadow[1] = NULL; // 1
				}
				MRT_Shadow[2] = m_Quads->get("ShadowMap"+num2str(light)+"d"+num2str(peel))->getRT(); // 2
				UINT numMRT = 3;

				// clear MRT
				m_Context->OMSetRenderTargets(numMRT, MRT_Shadow, m_Quads->get("ShadowAlbedo"+num2str(light)+"d0")->getDSV());
				m_Context->RSSetViewports(1, &m_Quads->get("ShadowAlbedo"+num2str(light)+"d0")->getVP());
                for(UINT i=0; i<numMRT; ++i) {
					if(peel != 0 && i == 2) {// paint last shadow map white, except for the first peel step
						if(MRT_Shadow[i]) {
							m_Context->ClearRenderTargetView(MRT_Shadow[i], white);
                        }
                    } else {
						if(MRT_Shadow[i]) {
							m_Context->ClearRenderTargetView(MRT_Shadow[i], m_Menu->getParams()->SceneSettings.BackgroundColor);
                        }
                    }
                }
				m_Context->ClearDepthStencilView(m_Quads->get("ShadowAlbedo"+num2str(light)+"d0")->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
				// actual drawing
				if(m_Menu->getParams()->SceneSettings.drawSkin) {
					m_ModelObject->drawSkin("VS_ShadowPass"+num2str(light), "", "PS_ShadowPeelingPass"+num2str(light)+"d"+num2str(peel), "");
				}
				if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
					m_ModelObject->drawNonSkin("VS_ShadowPass"+num2str(light), "", "PS_ShadowPeelingPass"+num2str(light)+"d"+num2str(peel), "");
				}
	
				// cleanup
				ID3D11RenderTargetView **NullRT = new ID3D11RenderTargetView*[numMRT];
				for(UINT i=0; i<numMRT; ++i)
					NullRT[i] = NULL;
				m_Context->OMSetRenderTargets(numMRT, NullRT, NULL);
				safe_delete_array(NullRT);
				resetOMTargetsAndViewport();
				// store textures
				m_TextureManager->addTexture("ShadowMap"+num2str(light)+"d"+num2str(peel), m_Quads->get("ShadowMap"+num2str(light)+"d"+num2str(peel))->getColorMap());

				std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
				m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

				delete [] MRT_Shadow;
			}
			m_TextureManager->addTexture("ShadowAlbedo"+num2str(light)+"d0", m_Quads->get("ShadowAlbedo"+num2str(light)+"d0")->getColorMap());
			m_TextureManager->addTexture("ShadowNormal"+num2str(light)+"d0", m_Quads->get("ShadowNormal"+num2str(light)+"d0")->getColorMap());
		}
		m_Context->RSSetState(m_Rasterizers["BackfaceCulling"]);
	} else {
		for(UINT light=0; light<NUMLIGHTSOURCES; ++light) {
			for(UINT peel=0; peel<NUMDEPTHPEELING; ++peel) {
				m_TextureManager->addTexture("ShadowMap"+num2str(light)+"d"+num2str(peel), m_Quads->get("ShadowMap"+num2str(light)+"d"+num2str(peel))->getColorMap());
			}
			m_TextureManager->addTexture("ShadowAlbedo"+num2str(light)+"d0", m_Quads->get("ShadowAlbedo"+num2str(light)+"d0")->getColorMap());
			m_TextureManager->addTexture("ShadowNormal"+num2str(light)+"d0", m_Quads->get("ShadowNormal"+num2str(light)+"d0")->getColorMap());
		}
		std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	}
}

void Context::geometryPass() {
	// set MRT
	ID3D11RenderTargetView *MRT_Geometry[] = {
		m_Quads->get("PositionMap")->getRT(),
		m_Quads->get("VertexNormalMap")->getRT(),
		m_Quads->get("NormalMap")->getRT(),
		m_Quads->get("AlbedoMap")->getRT(),
		m_Quads->get("SpecMap")->getRT(),
		m_Quads->get("DepthMap")->getRT(),
		// material map?
	};
	UINT numMRT = ARRAYSIZE(MRT_Geometry);
	// clear MRT
	m_Context->OMSetRenderTargets(numMRT, MRT_Geometry, m_Quads->get("AlbedoMap")->getDSV());
	m_Context->RSSetViewports(1, &m_Quads->get("AlbedoMap")->getVP());
	for(UINT i=0; i<numMRT; ++i)
		m_Context->ClearRenderTargetView(MRT_Geometry[i], m_Menu->getParams()->SceneSettings.BackgroundColor);
	m_Context->ClearDepthStencilView(m_Quads->get("AlbedoMap")->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_Context->OMSetDepthStencilState(m_DSSs["MarkPixels"], 1); // skin is marked with 1
		m_ModelObject->drawSkin("VS_Head", "", "PS_GeometryPass", "");
	}
	if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
		m_Context->OMSetDepthStencilState(m_DSSs["MarkPixels"], 2); // non skin is marked with 2

		m_ModelObject->drawNonSkin("VS_Head", "", "PS_GeometryPass", "");
	}
	
	// cleanup
	ID3D11RenderTargetView **NullRT = new ID3D11RenderTargetView*[numMRT];
	for(UINT i=0; i<numMRT; ++i)
		NullRT[i] = NULL;
	m_Context->OMSetRenderTargets(numMRT, NullRT, NULL);
	safe_delete_array(NullRT);
	resetOMTargetsAndViewport();
	// store textures
	m_TextureManager->addTexture("PositionMap", m_Quads->get("PositionMap")->getColorMap());
	m_TextureManager->addTexture("VertexNormalMap", m_Quads->get("VertexNormalMap")->getColorMap());
	m_TextureManager->addTexture("NormalMap", m_Quads->get("NormalMap")->getColorMap());
	m_TextureManager->addTexture("AlbedoMap", m_Quads->get("AlbedoMap")->getColorMap());
	m_TextureManager->addTexture("SpecMap", m_Quads->get("SpecMap")->getColorMap());
	m_TextureManager->addTexture("DepthMap", m_Quads->get("DepthMap")->getColorMap());

	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::TS_geometryPass() {
	m_Context->RSSetState(m_Rasterizers["NoCulling"]);
	m_Context->OMSetDepthStencilState(NULL, 0);
	// set MRT
	ID3D11RenderTargetView *MRT_Geometry[] = {
		m_Quads->get("PositionMap")->getRT(),
		m_Quads->get("VertexNormalMap")->getRT(),
		m_Quads->get("NormalMap")->getRT(),
		m_Quads->get("TexCoordMap")->getRT(),
		m_Quads->get("DepthMap")->getRT(),
		// material map?
	};
	UINT numMRT = ARRAYSIZE(MRT_Geometry);
	// clear MRT
	m_Context->OMSetRenderTargets(numMRT, MRT_Geometry, m_Quads->get("PositionMap")->getDSV());
	m_Context->RSSetViewports(1, &m_Quads->get("PositionMap")->getVP());
	for(UINT i=0; i<numMRT; ++i)
		m_Context->ClearRenderTargetView(MRT_Geometry[i], m_Menu->getParams()->SceneSettings.BackgroundColor);
	m_Context->ClearDepthStencilView(m_Quads->get("PositionMap")->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_ModelObject->drawSkin("VS_UnwrapMesh", "", "PS_TSGeometryPass", "");
	}
	/*if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
		m_ModelObject->drawNonSkin("VS_UnwrapMesh", "", "PS_TSGeometryPass", "");
	}*/
	
	// cleanup
	ID3D11RenderTargetView **NullRT = new ID3D11RenderTargetView*[numMRT];
	for(UINT i=0; i<numMRT; ++i)
		NullRT[i] = NULL;
	m_Context->OMSetRenderTargets(numMRT, NullRT, NULL);
	safe_delete_array(NullRT);
	resetOMTargetsAndViewport();
	// store textures
	m_TextureManager->addTexture("PositionMap", m_Quads->get("PositionMap")->getColorMap());
	m_TextureManager->addTexture("VertexNormalMap", m_Quads->get("VertexNormalMap")->getColorMap());
	m_TextureManager->addTexture("NormalMap", m_Quads->get("NormalMap")->getColorMap());
	m_TextureManager->addTexture("TexCoordMap", m_Quads->get("TexCoordMap")->getColorMap());
	m_TextureManager->addTexture("DepthMap", m_Quads->get("DepthMap")->getColorMap());

	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	m_Context->RSSetState(m_Rasterizers["BackfaceCulling"]);
}

void Context::Jimenez_lightPass() {
	ID3D11RenderTargetView *MRT_Light[] = {
		m_Quads->get("DiffuseMap")->getRT(),
		m_Quads->get("SpecularMap")->getRT(),
	};
	UINT numMRT = ARRAYSIZE(MRT_Light);
	// clear MRT
	m_Context->OMSetRenderTargets(numMRT, MRT_Light, m_Quads->get("AlbedoMap")->getDSV());
	m_Context->RSSetViewports(1, &m_Quads->get("DiffuseMap")->getVP());
	m_Context->OMSetDepthStencilState(m_DSSs["SelectEverythingFrom"], 1); // draw everything
	for(UINT i=0; i<numMRT; ++i)
		m_Context->ClearRenderTargetView(MRT_Light[i], m_Menu->getParams()->SceneSettings.BackgroundColor);
	m_Context->ClearDepthStencilView(m_d3dDepthStencilView, 0, 1.0f, 0);

	// draw
	m_Quad.setShaders("VS_Quad", "", "PS_Jimenez_LightPass", "");
	m_Quad.draw();

	// cleanup
	ID3D11RenderTargetView **NullRT = new ID3D11RenderTargetView*[numMRT];
	for(UINT i=0; i<numMRT; ++i)
		NullRT[i] = NULL;
	m_Context->OMSetRenderTargets(numMRT, NullRT, NULL);
	safe_delete_array(NullRT);
	resetOMTargetsAndViewport();
	// store textures
	m_TextureManager->addTexture("DiffuseMap", m_Quads->get("DiffuseMap")->getColorMap());
	m_TextureManager->addTexture("SpecularMap", m_Quads->get("SpecularMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Jimenez_multiblurPass() {
	std::string i_str;
	std::vector<ID3D11ShaderResourceView*> SRV;
	m_Context->OMSetDepthStencilState(m_DSSs["SelectPixels"], 1); // blur only skin
	for(UINT i=1; i<4; ++i) { // skip first blur, not evaluated later
		i_str = num2str(i);
		m_Quads->get("BlurX"+i_str+"Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_Jimenez_BlurX"+i_str, "");
			m_Quad.draw();
		m_Quads->get("BlurX"+i_str+"Map")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("BlurX"+i_str+"Map", m_Quads->get("BlurX"+i_str+"Map")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

		m_Quads->get("BlurY"+i_str+"Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_Jimenez_BlurY"+i_str, "");
			m_Quad.draw();
		m_Quads->get("BlurY"+i_str+"Map")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("BlurY"+i_str+"Map", m_Quads->get("BlurY"+i_str+"Map")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	}
}

void Context::Jimenez_combinePass() {
	m_Quads->get("FinalSSSMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
		m_Context->OMSetDepthStencilState(m_DSSs["SelectEverythingFrom"], 1);
		m_Quad.setShaders("VS_Quad", "", "PS_Jimenez_Combine", "");
		m_Quad.draw();
		m_Context->OMSetDepthStencilState(m_DSSs["MarkPixel"], 3); // draw light sources
		for(UINT i=0; i<(UINT)m_LightSpheres.size(); ++i) {
			if(m_Menu->getParams()->Lights[i].enableLight)
				m_LightSpheres[i]->draw();
		}
	m_Quads->get("FinalSSSMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("FinalSSSMap", m_Quads->get("FinalSSSMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Brand_lightPass() {
	ID3D11RenderTargetView *MRT_Light[] = {
		m_Quads->get("DiffuseMap")->getRT(), // pre-diffuse map (no transmittance, will be added later)
		m_Quads->get("SpecularMap")->getRT(),
        m_Quads->get("TransmittanceMap")->getRT(),
	};
	UINT numMRT = ARRAYSIZE(MRT_Light);
	// clear MRT
	m_Context->OMSetRenderTargets(numMRT, MRT_Light, m_Quads->get("AlbedoMap")->getDSV());
	m_Context->RSSetViewports(1, &m_Quads->get("AlbedoMap")->getVP());
	m_Context->OMSetDepthStencilState(m_DSSs["SelectEverythingFrom"], 1); // draw everything
	for(UINT i=0; i<numMRT; ++i)
		m_Context->ClearRenderTargetView(MRT_Light[i], m_Menu->getParams()->SceneSettings.BackgroundColor);
	m_Context->ClearDepthStencilView(m_d3dDepthStencilView, 0, 1.0f, 0);
    
	// draw
	m_Quad.setShaders("VS_Quad", "", "PS_Brand_LightPass", "");
	m_Quad.draw();

	// cleanup
	ID3D11RenderTargetView **NullRT = new ID3D11RenderTargetView*[numMRT];
	for(UINT i=0; i<numMRT; ++i)
		NullRT[i] = NULL;
	m_Context->OMSetRenderTargets(numMRT, NullRT, NULL);
	safe_delete_array(NullRT);
	resetOMTargetsAndViewport();
	// store textures
	m_TextureManager->addTexture("DiffuseMap", m_Quads->get("DiffuseMap")->getColorMap());
	m_TextureManager->addTexture("SpecularMap", m_Quads->get("SpecularMap")->getColorMap());
    m_TextureManager->addTexture("TransmittanceMap", m_Quads->get("TransmittanceMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
    m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Brand_transmittanceBlurPass() {
	m_Context->OMSetDepthStencilState(m_DSSs["SelectPixels"], 1);
    m_Quads->get("TransmittanceBlurXMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
		m_Quad.setShaders("VS_Quad", "", "PS_Brand_TransmittanceBlurX", "");
		m_Quad.draw();
	m_Quads->get("TransmittanceBlurXMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("TransmittanceBlurXMap", m_Quads->get("TransmittanceBlurXMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

    m_Quads->get("TransmittanceBlurYMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
		m_Quad.setShaders("VS_Quad", "", "PS_Brand_TransmittanceBlurY", "");
		m_Quad.draw();
	m_Quads->get("TransmittanceBlurYMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("TransmittanceBlurYMap", m_Quads->get("TransmittanceBlurYMap")->getColorMap());
	SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

    // mix diffuse and transmittance
	m_Context->OMSetDepthStencilState(m_DSSs["SelectEverythingFrom"], 1);
    m_Quads->get("PostDiffuseMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
		m_Quad.setShaders("VS_Quad", "", "PS_Brand_TransmittanceMix", "");
		m_Quad.draw();
	m_Quads->get("PostDiffuseMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("PostDiffuseMap", m_Quads->get("PostDiffuseMap")->getColorMap());
	SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Brand_multiblurPass() {
	std::string i_str;
	std::vector<ID3D11ShaderResourceView*> SRV;
	m_Context->OMSetDepthStencilState(m_DSSs["SelectPixels"], 1); // blur only skin
	for(UINT i=1; i<4; ++i) { // use first blur to mix transmittance and diffuse
		i_str = num2str(i);
		m_Quads->get("BlurX"+i_str+"Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_Brand_BlurX"+i_str, "");
			m_Quad.draw();
		m_Quads->get("BlurX"+i_str+"Map")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("BlurX"+i_str+"Map", m_Quads->get("BlurX"+i_str+"Map")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

		m_Quads->get("BlurY"+i_str+"Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_Brand_BlurY"+i_str, "");
			m_Quad.draw();
		m_Quads->get("BlurY"+i_str+"Map")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("BlurY"+i_str+"Map", m_Quads->get("BlurY"+i_str+"Map")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	}
}

void Context::Brand_combinePass() {
	m_Quads->get("FinalSSSMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, 0, m_Quads->get("AlbedoMap")->getDSV());
		m_Context->OMSetDepthStencilState(m_DSSs["SelectEverythingFrom"], 1);
		m_Quad.setShaders("VS_Quad", "", "PS_Brand_Combine", "");
		m_Quad.draw();
		m_Context->OMSetDepthStencilState(m_DSSs["MarkPixel"], 3); // draw light sources
		for(UINT i=0; i<(UINT)m_LightSpheres.size(); ++i) {
			if(m_Menu->getParams()->Lights[i].enableLight)
				m_LightSpheres[i]->draw();
		}
	m_Quads->get("FinalSSSMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("FinalSSSMap", m_Quads->get("FinalSSSMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::dEon_irradiancePass() {
	m_Context->RSSetState(m_Rasterizers["NoCulling"]);
	m_Context->OMSetDepthStencilState(NULL, 0);
	ID3D11RenderTargetView *MRT_Geometry[] = {
		m_Quads->get("DiffuseMap")->getRT(),
		m_Quads->get("DepthMap")->getRT(),
	};
	UINT numMRT = ARRAYSIZE(MRT_Geometry);
	// clear MRT
	m_Context->OMSetRenderTargets(numMRT, MRT_Geometry, m_Quads->get("DiffuseMap")->getDSV());
	m_Context->RSSetViewports(1, &m_Quads->get("DiffuseMap")->getVP());
	for(UINT i=0; i<numMRT; ++i)
		m_Context->ClearRenderTargetView(MRT_Geometry[i], m_Menu->getParams()->SceneSettings.BackgroundColor);
	m_Context->ClearDepthStencilView(m_Quads->get("DiffuseMap")->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_ModelObject->drawSkin("VS_UnwrapMesh", "", "PS_dEon_IrradiancePass", "");
	}
	
	ID3D11RenderTargetView **NullRT = new ID3D11RenderTargetView*[numMRT];
	for(UINT i=0; i<numMRT; ++i)
		NullRT[i] = NULL;
	m_Context->OMSetRenderTargets(numMRT, NullRT, NULL);
	safe_delete_array(NullRT);
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("DiffuseMap", m_Quads->get("DiffuseMap")->getColorMap());
	m_TextureManager->addTexture("DepthMap", m_Quads->get("DepthMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	m_Context->RSSetState(m_Rasterizers["BackfaceCulling"]);
}

void Context::dEon_dilateIrradiancePass() {
	ID3D11DepthStencilView *DSV = m_Quads->get("DiffuseMap")->getDSV();
	m_Context->OMSetDepthStencilState(NULL, 0);
	std::vector<ID3D11ShaderResourceView*> SRV;
	m_Quads->get("PostDiffuseMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH, DSV);
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_Quad.setShaders("VS_Quad", "", "PS_Dilate_dEonIrradiance", "");
		m_Quad.draw();
	}
	
	m_Quads->get("PostDiffuseMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("PostDiffuseMap", m_Quads->get("PostDiffuseMap")->getColorMap());
	SRV = m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Brand_dilateShadowNormalsPass() {
	ID3D11DepthStencilView *DSV = m_Quads->get("ShadowAlbedo"+num2str(TRANSLUCENCYLIGHT)+"d0")->getDSV();
	m_Context->OMSetDepthStencilState(NULL, 0);
	std::vector<ID3D11ShaderResourceView*> SRV;
	m_Quads->get("ShadowNormal2"+num2str(TRANSLUCENCYLIGHT)+"d0")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH, DSV);
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_Quad.setShaders("VS_Quad", "", "PS_Dilate_BrandShadowNormals", "");
		m_Quad.draw();
	}
	
	m_Quads->get("ShadowNormal2"+num2str(TRANSLUCENCYLIGHT)+"d0")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("ShadowNormal2"+num2str(TRANSLUCENCYLIGHT)+"d0", m_Quads->get("ShadowNormal2"+num2str(TRANSLUCENCYLIGHT)+"d0")->getColorMap());
	SRV = m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::dEon_shadowPass() {
	m_Context->OMSetDepthStencilState(NULL, 0);
	for(UINT light=0; light<NUMLIGHTSOURCES; ++light) {
		if(m_lightsUpdated || m_modelUpdate || m_algorithmUpdate) {
			
			m_Quads->get("TSM"+num2str(light))->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
			// actual drawing
			if(m_Menu->getParams()->SceneSettings.drawSkin) {
				m_ModelObject->drawSkin("VS_ShadowPass"+num2str(light), "", "PS_dEon_ShadowPass"+num2str(light), "");
			}
			if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
				m_ModelObject->drawNonSkin("VS_ShadowPass"+num2str(light), "", "PS_dEon_ShadowPass"+num2str(light), "");
			}
			
			m_Quads->get("TSM"+num2str(light))->end();
			resetOMTargetsAndViewport();
		}
		m_TextureManager->addTexture("TSM"+num2str(light), m_Quads->get("TSM"+num2str(light))->getColorMap());
	}
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::dEon_textureStretchPass() {
	m_Context->RSSetState(m_Rasterizers["NoCulling"]);
	m_Quads->get("StretchMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, NULL);
	m_Context->OMSetDepthStencilState(NULL, 0);

	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_ModelObject->drawSkin("VS_UnwrapMesh", "", "PS_dEon_StretchPass", "");
	}
		
	m_Quads->get("StretchMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("StretchMap", m_Quads->get("StretchMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	m_Context->RSSetState(m_Rasterizers["BackfaceCulling"]);
}

void Context::dEon_multiblurPass() {
	ID3D11DepthStencilView *DSV = m_Quads->get("DiffuseMap")->getDSV();
	m_Context->OMSetDepthStencilState(NULL, 0);
	std::vector<ID3D11ShaderResourceView*> SRV;
	for(UINT i=1; i<NUMBLURPASSES; ++i) {
		// X
		m_Quads->get("BlurX"+num2str(i)+"Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DSV);
		if(m_Menu->getParams()->SceneSettings.drawSkin) {
			m_Quad.setShaders("VS_Quad", "", "PS_dEon_BlurX"+num2str(i), "");
			m_Quad.draw();
		}
		
		m_Quads->get("BlurX"+num2str(i)+"Map")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("BlurX"+num2str(i)+"Map", m_Quads->get("BlurX"+num2str(i)+"Map")->getColorMap());
		SRV = m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

		// Y
		m_Quads->get("BlurY"+num2str(i)+"Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DSV);
		if(m_Menu->getParams()->SceneSettings.drawSkin) {
			m_Quad.setShaders("VS_Quad", "", "PS_dEon_BlurY"+num2str(i), "");
			m_Quad.draw();
		}
		
		m_Quads->get("BlurY"+num2str(i)+"Map")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("BlurY"+num2str(i)+"Map", m_Quads->get("BlurY"+num2str(i)+"Map")->getColorMap());
		SRV = m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	}
}

void Context::dEon_combinePass() {
	ID3D11DepthStencilView *DSV = m_Quads->get("DiffuseMap")->getDSV();
	m_Quads->get("FinalSSSMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DSV);
	m_Context->OMSetDepthStencilState(NULL, 0);

	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_ModelObject->drawSkin("VS_Head", "", "PS_dEon_Combine", "");
	}
	if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
		m_ModelObject->drawNonSkin("VS_Head", "", "PS_Skeleton", "");
	}
	m_Context->OMSetDepthStencilState(m_DSSs["MarkPixel"], 3); // draw light sources
	for(UINT i=0; i<(UINT)m_LightSpheres.size(); ++i) {
		if(m_Menu->getParams()->Lights[i].enableLight)
			m_LightSpheres[i]->draw();
	}
	
	m_Quads->get("FinalSSSMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("FinalSSSMap", m_Quads->get("FinalSSSMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Hable_blurPass() {
	ID3D11DepthStencilView *DSV = m_Quads->get("DiffuseMap")->getDSV();
	m_Context->OMSetDepthStencilState(NULL, 0);
	std::vector<ID3D11ShaderResourceView*> SRV;
	m_Quads->get("BlurY5Map")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DSV);
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_Quad.setShaders("VS_Quad", "", "PS_Hable_Blur", "");
		m_Quad.draw();
	}
		
	m_Quads->get("BlurY5Map")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("BlurY5Map", m_Quads->get("BlurY5Map")->getColorMap());
	SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::Hable_combinePass() {
	ID3D11DepthStencilView *DSV = m_Quads->get("DiffuseMap")->getDSV();
	m_Quads->get("FinalSSSMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DSV);
	m_Context->OMSetDepthStencilState(NULL, 0);

	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_ModelObject->drawSkin("VS_Head", "", "PS_Hable_Combine", "");
	}
	if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
		m_ModelObject->drawNonSkin("VS_Head", "", "PS_Skeleton", "");
	}
	m_Context->OMSetDepthStencilState(m_DSSs["MarkPixel"], 3); // draw light sources
	for(UINT i=0; i<(UINT)m_LightSpheres.size(); ++i) {
		if(m_Menu->getParams()->Lights[i].enableLight)
			m_LightSpheres[i]->draw();
	}
	
	m_Quads->get("FinalSSSMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("FinalSSSMap", m_Quads->get("FinalSSSMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::NoSSS_renderPass() {
	//m_Context->RSSetState(m_Rasterizers["Wireframe"]);
	ID3D11DepthStencilView *DSV = m_Quads->get("DiffuseMap")->getDSV();
	m_Quads->get("FinalSSSMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DSV);
	// actual drawing
	if(m_Menu->getParams()->SceneSettings.drawSkin) {
		m_ModelObject->drawSkin("VS_Head", "", "PS_NoSSS_renderPass", "");
	}
	if(m_Menu->getParams()->SceneSettings.drawNonSkin) {
		m_ModelObject->drawNonSkin("VS_Head", "", "PS_Skeleton", "");
	}
	m_Context->OMSetDepthStencilState(m_DSSs["MarkPixel"], 3); // draw light sources
	for(UINT i=0; i<(UINT)m_LightSpheres.size(); ++i) {
		if(m_Menu->getParams()->Lights[i].enableLight)
			m_LightSpheres[i]->draw();
	}
		
	m_Quads->get("FinalSSSMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("FinalSSSMap", m_Quads->get("FinalSSSMap")->getColorMap());
	std::vector<ID3D11ShaderResourceView*> SRV;
	SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	//m_Context->RSSetState(m_Rasterizers["BackfaceCulling"]);
}

void Context::bloomPass() {
	//m_Context->OMSetDepthStencilState(m_DSSs["SelectEverythingFrom"], 1);
	m_Context->OMSetDepthStencilState(NULL, 0);
	std::vector<ID3D11ShaderResourceView*> SRV;
	if(m_Menu->getParams()->ShaderSettings.bloomFilter) {
		// brightness pass
		m_Quads->get("PPBrightnessMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH, m_Quads->get("PPBloomXMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_PPBrightPass", "");
			m_Quad.draw();
		m_Quads->get("PPBrightnessMap")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("PPBrightnessMap", m_Quads->get("PPBrightnessMap")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

		// bloom x
		m_Context->OMSetDepthStencilState(m_DSSs["DoNothing"], 0);
		m_Quads->get("PPBloomXMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH, m_Quads->get("PPBloomXMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_PPBloomX", "");
			m_Quad.draw();
		m_Quads->get("PPBloomXMap")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("PPBloomXMap", m_Quads->get("PPBloomXMap")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);

		// bloom y
		m_Quads->get("PPBloomYMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH, m_Quads->get("PPBloomXMap")->getDSV());
			m_Quad.setShaders("VS_Quad", "", "PS_PPBloomY", "");
			m_Quad.draw();
		m_Quads->get("PPBloomYMap")->end();
		resetOMTargetsAndViewport();
		m_TextureManager->addTexture("PPBloomYMap", m_Quads->get("PPBloomYMap")->getColorMap());
		SRV =  m_TextureManager->getSRVs();
		m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
	}

	// combine
	m_Context->OMSetDepthStencilState(m_DSSs["DoNothing"], 0);
	m_Quads->get("PPCombineMap")->begin(m_Menu->getParams()->SceneSettings.BackgroundColor, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, m_Quads->get("AlbedoMap")->getDSV());
		m_Quad.setShaders("VS_Quad", "", "PS_PPCombine", "");
		m_Quad.draw();
	m_Quads->get("PPCombineMap")->end();
	resetOMTargetsAndViewport();
	m_TextureManager->addTexture("PPCombineMap", m_Quads->get("PPCombineMap")->getColorMap());
	SRV =  m_TextureManager->getSRVs();
	m_Context->PSSetShaderResources(0, (UINT)SRV.size(), &SRV[0]);
}

void Context::displayQuadPass() {
	m_Quad.setShaders("VS_Quad", "", "PS_Quad", "");
	m_Quad.draw();
}

void Context::updateCameras() {
	HRESULT hr = S_OK;

	CameraMenuParams CMP = m_Menu->getParams()->Cameras[m_Menu->getParams()->cameraSelected];
	m_cameraUpdate |= m_Camera.setRotationSelf(CMP.RotationSelf);
	m_cameraUpdate |= m_Camera.setTranslation(CMP.Translation);
	m_cameraUpdate |= m_Camera.setRotationGlobal(CMP.RotationGlobal);
	m_cameraUpdate |= m_Camera.setFovy(CMP.fovy);
	m_cameraUpdate |= m_Camera.setZNear(CMP.zNear);
	m_cameraUpdate |= m_Camera.setZFar(CMP.zFar);

	if(m_cameraUpdate) {
		m_Camera.updateMatrices();

		// set camera buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		chkHr(m_Context->Map(m_d3dBuffers["cbPerCamera"], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		cbPerCamera *perCameraBuffer = (cbPerCamera*)MappedResource.pData;

		// world space -> camera space
		D3DXMatrixTranspose(&perCameraBuffer->world2Camera, &m_Camera.getWorld2ObjectMatrix());
		// world space -> camera space (inverse transposed)
		perCameraBuffer->world2Camera_IT = m_Camera.getObject2WorldMatrix();
		// camera space -> world space
		D3DXMatrixTranspose(&perCameraBuffer->camera2World, &m_Camera.getObject2WorldMatrix());
		// world space -> camera space -> camera proj
		D3DXMatrixTranspose(&perCameraBuffer->world2CameraProj, &m_Camera.getWorld2CameraProjMatrix());
		D3DXMATRIX ProjInv;
		// camera proj -> camera space -> world space
		D3DXMatrixInverse(&ProjInv, NULL, &m_Camera.getWorld2CameraProjMatrix());
		D3DXMatrixTranspose(&perCameraBuffer->cameraProj2World, &ProjInv);
		perCameraBuffer->cameraZNear = m_Camera.getZNear();
		perCameraBuffer->cameraZFar = m_Camera.getZFar();
		m_Context->Unmap(m_d3dBuffers["cbPerCamera"], 0);

		m_Context->VSSetConstantBuffers(0, 1, &m_d3dBuffers["cbPerCamera"]);
		m_Context->PSSetConstantBuffers(0, 1, &m_d3dBuffers["cbPerCamera"]);
	}
}

void Context::updateLights() {
	HRESULT hr = S_OK;

	for(size_t i=0; i<m_LightSources.size(); ++i) {
		LightMenuParams LMP = m_Menu->getParams()->Lights[i];
		m_lightsUpdated |= m_LightSources[i]->setRotationSelf(LMP.RotationSelf);
		m_lightsUpdated |= m_LightSources[i]->setTranslation(LMP.Translation);
		m_lightsUpdated |= m_LightSources[i]->setRotationGlobal(LMP.RotationGlobal);
		m_lightsUpdated |= m_LightSources[i]->setColor(LMP.Color);
		m_lightsUpdated |= m_LightSources[i]->setAttenuation(D3DXVECTOR4(LMP.intensity, LMP.a0, LMP.a1, LMP.a2));
		m_lightsUpdated |= m_LightSources[i]->setFovy(LMP.fovy);
		m_lightsUpdated |= m_LightSources[i]->setZNear(LMP.zNear);
		m_lightsUpdated |= m_LightSources[i]->setZFar(LMP.zFar);
		if(m_LightSources[i]->isEnabled() != LMP.enableLight) {
			m_lightsUpdated |= true;
			m_LightSources[i]->setEnabled(LMP.enableLight);
		} // (true or false) = true
	}
	#if RECALCSHADOWMAPEVERYFRAME
		// debugging test, always run shadow pass
		m_lightsUpdated = true;
	#endif

	if(m_lightsUpdated) {
		// set light buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		chkHr(m_Context->Map(m_d3dBuffers["cbPerLight"], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		cbPerLight *perLightBuffer = (cbPerLight*)MappedResource.pData;

		for(size_t i=0; i<m_LightSources.size(); ++i) {
			m_LightSources[i]->updateMatrices();

			// world space -> light space
			D3DXMatrixTranspose(&perLightBuffer->world2Light[i], &m_LightSources[i]->getWorld2ObjectMatrix());
			// world space -> light space -> proj space
			D3DXMatrixTranspose(&perLightBuffer->world2LightProj[i], &m_LightSources[i]->getWorld2LightProjMatrix());
			// light space -> world space
			D3DXMatrixTranspose(&perLightBuffer->light2World[i], &m_LightSources[i]->getObject2WorldMatrix());
			if(!m_Menu->getParams()->Lights[i].showLight)
				perLightBuffer->attenuation[i] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
			else
				perLightBuffer->attenuation[i] = m_LightSources[i]->getAttenuation();
								
			perLightBuffer->color[i] = D3DXVECTOR4(m_LightSources[i]->getColor(), 0.0f);
			perLightBuffer->zNearFar[i] = D3DXVECTOR4(m_LightSources[i]->getZNear(), m_LightSources[i]->getZFar(), 0.0f, 0.0f);
		}
		m_Context->Unmap(m_d3dBuffers["cbPerLight"], 0);

		m_Context->VSSetConstantBuffers(1, 1, &m_d3dBuffers["cbPerLight"]);
		m_Context->PSSetConstantBuffers(1, 1, &m_d3dBuffers["cbPerLight"]);
	}
}

void Context::updateModels() {
	HRESULT hr = S_OK;

	ModelMenuParams MMP = m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected];
	m_modelUpdate |= m_ModelObject->setRotationSelf(MMP.RotationSelf);
	m_modelUpdate |= m_ModelObject->setTranslation(MMP.Translation);
	m_modelUpdate |= m_ModelObject->setRotationGlobal(MMP.RotationGlobal);

	if(m_modelUpdate || m_cameraUpdate) {
		m_ModelObject->updateMatrices();

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		chkHr(m_Context->Map(m_d3dBuffers["cbPerModel"], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

		cbPerModel *perModelBuffer = (cbPerModel*)MappedResource.pData;
		D3DXMatrixTranspose(&perModelBuffer->object2World, &m_ModelObject->getObject2WorldMatrix());
		perModelBuffer->object2World_IT = m_ModelObject->getWorld2ObjectMatrix(); // invers transposed
		// do the full projection matrix for each model once on the cpu and not for every vertex in the VS
		// need to check weather the camera has been updated, too
		D3DXMatrixMultiplyTranspose(&perModelBuffer->object2CameraProj, &m_ModelObject->getObject2WorldMatrix(), &m_Camera.getWorld2CameraProjMatrix());

		m_Context->Unmap(m_d3dBuffers["cbPerModel"], 0);

		m_Context->VSSetConstantBuffers(2, 1, &m_d3dBuffers["cbPerModel"]);
		m_Context->PSSetConstantBuffers(2, 1, &m_d3dBuffers["cbPerModel"]);
	}
}

void Context::updateShaderSettings() {
	HRESULT hr = S_OK;

	ShaderMenuParams &SHMP = m_Menu->getParams()->ShaderSettings;
	SceneMenuParams &SCMP = m_Menu->getParams()->SceneSettings;
    if(shaderMenuUpdated(SHMP, SCMP)) {
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		chkHr(m_Context->Map(m_d3dBuffers["cbShaderSettings"], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		cbShaderSettings *shaderSettings = (cbShaderSettings*)MappedResource.pData;
		shaderSettings->textureSelected = SHMP.textureSelected;
		shaderSettings->stretchAlpha = SHMP.stretchAlpha;
		shaderSettings->stretchBeta = SHMP.stretchBeta;
		shaderSettings->blurMix = SHMP.blurMix;
		shaderSettings->fresnelReflectance = SHMP.fresnelReflectance;
		shaderSettings->bloomFilter = SHMP.bloomFilter;
		shaderSettings->shadowMapSize = SHMP.ShadowMapSize;
		shaderSettings->shadowMapEpsilon = SHMP.shadowMapEpsilon;
		shaderSettings->transmittanceScale = SHMP.transmittanceScale;
		shaderSettings->gammaCorrection = 1.0f/SHMP.gammaCorrection;
		shaderSettings->minBloom = SHMP.minBloom;
		shaderSettings->maxBloom = SHMP.maxBloom;
        shaderSettings->transmittanceBlur1 = SHMP.transmittanceBlur1;
        shaderSettings->transmittanceBlur2 = SHMP.transmittanceBlur2;
		shaderSettings->transmittanceFNL = SHMP.transmittanceFNL;
		shaderSettings->depthEpplingEpsilon = SHMP.depthPeelingEpsilon;
		shaderSettings->sceneSize.x = SCMP.RenderSize.x;
		shaderSettings->sceneSize.y = SCMP.RenderSize.y;
		m_Context->Unmap(m_d3dBuffers["cbShaderSettings"], 0);

		m_Context->PSSetConstantBuffers(3, 1, &m_d3dBuffers["cbShaderSettings"]);
		m_shaderMenuUpdate = true;
	} else {
		m_shaderMenuUpdate = false;
	}
}

HRESULT Context::resize() {
	HRESULT hr = S_OK;
	m_windowSizeUpdate = false;

	if(getWidth() < 100 || getHeight() < 100)
		return hr;

	// release old views
	safe_delete(m_d3dRenderTargetView);
	safe_delete(m_d3dDepthStencilView);

	// resize swap chain
	HR(chkHr(m_d3dSwapChain->ResizeBuffers(1, FRAMEBUFFERWIDTH, FRAMEBUFFERHEIGHT, FRAMEBUFFERFORMAT, 0)));

	ID3D11Texture2D *BackBuffer = NULL;
	HR(chkHr(m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer))));
	d3dLiveName(BackBuffer, "Back Buffer");

	HR(chkHr(m_Device->CreateRenderTargetView(BackBuffer, NULL, &m_d3dRenderTargetView)));
	d3dLiveName(m_d3dRenderTargetView, "Render Target View");

	safe_delete(BackBuffer);

	// create depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthStencilDesc.Width = FRAMEBUFFERWIDTH;
	depthStencilDesc.Height = FRAMEBUFFERHEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.MiscFlags = 0;
	
	ID3D11Texture2D *DepthStencilBuffer = NULL;
	HR(chkHr(m_Device->CreateTexture2D(&depthStencilDesc, NULL, &DepthStencilBuffer)));
	d3dLiveName(DepthStencilBuffer, "Depth Stencil Buffer Texture");

	HR(chkHr(m_Device->CreateDepthStencilView(DepthStencilBuffer, NULL, &m_d3dDepthStencilView)));
	d3dLiveName(m_d3dDepthStencilView, "Depth Stencil View");

	resetOMTargetsAndViewport();
	safe_delete(DepthStencilBuffer);

	m_windowSizeUpdate |= m_Camera.setAspectRatio(getClientWidth() / (float) getClientHeight());
	for(UINT i=0; i<(UINT)m_LightSources.size(); ++i)
		m_windowSizeUpdate |= m_LightSources[i]->setAspectRatio(getClientWidth() / (float) getClientHeight());
	m_Menu->setSize(getWidth(), getHeight());

	return hr;
}

void Context::resetOMTargetsAndViewport() {
	m_Context->OMSetRenderTargets(1, &m_d3dRenderTargetView, m_d3dDepthStencilView);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (float)VIEWPORTWIDTH;
	vp.Height = (float)VIEWPORTHEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_Context->RSSetViewports(1, &vp);
}

void Context::release() {
	
	m_isEnabled = false;

	unbindQuads();

	safe_delete_vector(m_LightSpheres);
	delete m_ModelObject;
	m_ModelObject = NULL;
	
	safe_delete_map(m_Rasterizers);
	
	delete m_ShaderManager;
	delete m_TextureManager;
	delete m_Quads;

	if(m_Context) {
		m_Context->ClearState();
		m_Context->Flush();
	}
		
	safe_delete_map(m_d3dBuffers);
	safe_delete_map(m_DSSs);
	
	for(size_t i=0; i<m_LightSources.size(); ++i) {
		delete m_LightSources[i];
		m_LightSources[i] = NULL;
	}

	safe_delete(m_d3dRenderTargetView);
	safe_delete(m_d3dDepthStencilView);
	safe_delete(m_d3dSwapChain);
	safe_delete(m_Context);
	safe_delete(m_Device);
}

bool Context::shaderMenuUpdated(const ShaderMenuParams &SHMP, const SceneMenuParams &SCMP) {
	if(SCMP.algorithm != m_SCMP_old.algorithm) {
        m_Menu->getParams()->ShaderSettings.transmittanceScale = m_TransmittanceScales[SCMP.algorithm];
        
		m_SCMP_old.algorithm = SCMP.algorithm;
	}

    m_TransmittanceScales[SCMP.algorithm] = m_Menu->getParams()->ShaderSettings.transmittanceScale;

	if(SHMP.bloomFilter != m_SHMP_old.bloomFilter ||
			SHMP.blurMix != m_SHMP_old.blurMix ||
			SHMP.fresnelReflectance != m_SHMP_old.fresnelReflectance ||
			SHMP.shadowMapEpsilon != m_SHMP_old.shadowMapEpsilon ||
			SHMP.ShadowMapSize != m_SHMP_old.ShadowMapSize ||
			SHMP.stretchAlpha != m_SHMP_old.stretchAlpha ||
			SHMP.stretchBeta != m_SHMP_old.stretchBeta ||
			SHMP.textureSelected != m_SHMP_old.textureSelected ||
			SHMP.transmittanceScale != m_SHMP_old.transmittanceScale ||
			SHMP.gammaCorrection != m_SHMP_old.gammaCorrection ||
			SHMP.minBloom != m_SHMP_old.minBloom ||
			SHMP.maxBloom != m_SHMP_old.maxBloom ||
            SHMP.transmittanceBlur1 != m_SHMP_old.transmittanceBlur1 ||
            SHMP.transmittanceBlur2 != m_SHMP_old.transmittanceBlur2 ||
			SHMP.transmittanceFNL != m_SHMP_old.transmittanceFNL ||
			SHMP.depthPeelingEpsilon != m_SHMP_old.depthPeelingEpsilon ||

			SCMP.RenderSize != m_SCMP_old.RenderSize
			) {
		m_SHMP_old.bloomFilter = SHMP.bloomFilter;
		m_SHMP_old.blurMix = SHMP.blurMix;
		m_SHMP_old.fresnelReflectance = SHMP.fresnelReflectance;
		m_SHMP_old.shadowMapEpsilon = SHMP.shadowMapEpsilon;
		m_SHMP_old.ShadowMapSize = SHMP.ShadowMapSize;
		m_SHMP_old.stretchAlpha = SHMP.stretchAlpha;
		m_SHMP_old.stretchBeta = SHMP.stretchBeta;
		m_SHMP_old.textureSelected = SHMP.textureSelected;
		m_SHMP_old.transmittanceScale = SHMP.transmittanceScale;
		m_SHMP_old.gammaCorrection = SHMP.gammaCorrection;
		m_SHMP_old.minBloom = SHMP.minBloom;
		m_SHMP_old.maxBloom = SHMP.maxBloom;
        m_SHMP_old.transmittanceBlur1 = SHMP.transmittanceBlur1;
        m_SHMP_old.transmittanceBlur2 = SHMP.transmittanceBlur2;
		m_SHMP_old.transmittanceFNL = SHMP.transmittanceFNL;
		m_SHMP_old.depthPeelingEpsilon = SHMP.depthPeelingEpsilon;

		m_SCMP_old.RenderSize = SCMP.RenderSize;

		return true;
	} else
		return false;
}

void Context::updateModelMesh() {
	// lazy evaluation ftw
	if(!m_ModelObject || m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].Name != m_ModelObject->getName()) {
		LOGG<<"\tLoading model '"+m_Menu->getParams()->Models[m_Menu->getParams()->modelSelected].Name+"' ...";
		delete m_ModelObject;
		switch(m_Menu->getParams()->modelSelected) {
		case MODEL_MAKEHUMAN:
			m_ModelObject = new MakeHuman();
			break;

		case MODEL_LEEPERRYSMITH:
			m_ModelObject = new InfiniteScan();
			break;

		case MODEL_TESTCASE:
			m_ModelObject = new Testcase();
			break;

		default:
			LOGG << "\tModel " + num2str(m_Menu->getParams()->modelSelected) + " not supported!";
			PostQuitMessage(E_INVALIDARG);
		}
		m_ModelObject->init(m_Device, m_Context);
		m_ModelObject->loadModel();
		LOGG<<"\tModel loaded.";
	}
}
