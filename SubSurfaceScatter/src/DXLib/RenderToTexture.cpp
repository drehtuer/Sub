#include "DXLib/RenderToTexture.h"

using namespace SubSurfaceScatter;

RenderToTexture::RenderToTexture()
	: m_width(0), m_height(0),
	  m_colorMapFormat(DXGI_FORMAT_UNKNOWN),
	  m_Device(NULL), m_Context(NULL),
	  m_ColorMapSRV(NULL), m_ColorMapRTV(NULL),
	  m_DepthMapSRV(NULL), m_DepthMapDSV(NULL), m_StencilMapSRV(NULL),
	  m_textureArraySize(1),
	  m_hasColorMap(false),
	  m_hasDepthMap(false),
	  m_LiveName("RenderToTexture"),
	  m_isResizable(true),
	  m_mipmapLevel(1)
{
	ZEROMEM(m_Viewport);
}

RenderToTexture::~RenderToTexture() {
	safe_delete(m_ColorMapSRV);
	safe_delete(m_ColorMapRTV);
	safe_delete(m_DepthMapSRV);
	safe_delete(m_DepthMapDSV);
	safe_delete(m_StencilMapSRV);
}

HRESULT RenderToTexture::init(ID3D11Device *Device, ID3D11DeviceContext *Context, const UINT width, const UINT height, const DXGI_FORMAT colorFormat, const UINT options, const std::string &liveName, const int numRenderTargets) {
	HRESULT hr = S_OK;
	m_colorMapFormat = colorFormat;
	m_Device = Device;
	m_Context = Context;
	
	m_textureArraySize = numRenderTargets;
	m_LiveName = liveName;
	if(options & RENDERTOTEXTURE_OPTIONS_USECOLORMAP)
		m_hasColorMap = true;
	if(options & RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP)
		m_hasDepthMap = true;
	if(options & RENDERTOTEXTURE_OPTIONS_USE_MIPMAPS)
		m_mipmapLevel = 3;

	chkHr(resize(width, height));
	// resize needs to run at least once
	if(options & RENDERTOTEXTURE_OPTIONS_ISRESIZABLE)
		m_isResizable = true;

	return hr;
}

HRESULT RenderToTexture::resize(const UINT width, const UINT height) {
	HRESULT hr = S_OK;
	if(!m_isResizable || !m_Device || !m_Context)
		return hr;
	if(width == m_width && height == m_height)
		return S_FALSE;

	m_width = width;
	m_height = height;

	safe_delete(m_ColorMapSRV);
	safe_delete(m_ColorMapRTV);
	safe_delete(m_DepthMapSRV);
	safe_delete(m_DepthMapDSV);
	safe_delete(m_StencilMapSRV);
	
	if(m_hasDepthMap)
		HR(buildDepthMaps());
	if(m_hasColorMap)
		HR(buildColorMaps());

	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (float)width;
	m_Viewport.Height = (float)height;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	return hr;
}

ID3D11ShaderResourceView *RenderToTexture::getColorMap() {
	return m_ColorMapSRV;
}

ID3D11ShaderResourceView *RenderToTexture::getDepthMap() {
	return m_DepthMapSRV;
}

void RenderToTexture::getDepthMapCopy(ID3D11ShaderResourceView *CopyTo) {
	ID3D11Resource *ROriginal = NULL, *RCopy = NULL;
	m_DepthMapSRV->GetResource(&ROriginal);
	CopyTo->GetResource(&RCopy);
	m_Context->CopyResource(RCopy, ROriginal);
}

ID3D11ShaderResourceView *RenderToTexture::getStencilMap() {
	return m_StencilMapSRV;
}

void RenderToTexture::getStencilMapCopy(ID3D11ShaderResourceView *CopyTo) {
	ID3D11Resource *ROriginal = NULL, *RCopy = NULL;
	m_StencilMapSRV->GetResource(&ROriginal);
	CopyTo->GetResource(&RCopy);
	m_Context->CopyResource(RCopy, ROriginal);
}

void RenderToTexture::begin(const D3DXVECTOR4 &ClearColor, const UINT clearFlag, ID3D11DepthStencilView *DSV) {
	ID3D11RenderTargetView *RenderTargets = { m_ColorMapRTV };
	m_Context->RSSetViewports(1, &m_Viewport);
	if(m_hasDepthMap) {
		m_Context->OMSetRenderTargets(1, &RenderTargets, m_DepthMapDSV);
		m_Context->ClearDepthStencilView(m_DepthMapDSV, clearFlag, 1.0f, 0);
	} else {
		m_Context->OMSetRenderTargets(1, &RenderTargets, DSV);
		m_Context->ClearDepthStencilView(DSV, clearFlag, 1.0f, 0);
	}
	if(m_ColorMapRTV)
		m_Context->ClearRenderTargetView(m_ColorMapRTV, ClearColor);
}

void RenderToTexture::end() {
	if(m_ColorMapSRV)
		m_Context->GenerateMips(m_ColorMapSRV);
}

HRESULT RenderToTexture::buildColorMaps() {
	HRESULT hr = S_OK;
	ID3D11Texture2D *ColorMapTexture = NULL;
	D3D11_TEXTURE2D_DESC texDesc;

	ZEROMEM(texDesc);
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.MipLevels = m_mipmapLevel;
	texDesc.ArraySize = m_textureArraySize;
	texDesc.Format = m_colorMapFormat;
	texDesc.SampleDesc.Count = MULTISAMPLINGLEVEL;
	texDesc.SampleDesc.Quality = MULTISAMPLINGQUALITY;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	HR(chkHr(m_Device->CreateTexture2D(&texDesc, NULL, &ColorMapTexture)));
	d3dLiveName(ColorMapTexture, m_LiveName + " color map texture2d");

	HR(chkHr(m_Device->CreateRenderTargetView(ColorMapTexture, NULL, &m_ColorMapRTV)));
	d3dLiveName(m_ColorMapRTV, m_LiveName + " color map RTV texture2d");

	HR(chkHr(m_Device->CreateShaderResourceView(ColorMapTexture, NULL, &m_ColorMapSRV)));
	d3dLiveName(m_ColorMapSRV, m_LiveName + " color map SRV texture2d");

	safe_delete(ColorMapTexture);

	return hr;
}

HRESULT RenderToTexture::buildDepthMaps() {
	HRESULT hr = S_OK;
	ID3D11Texture2D *DepthMapTexture = NULL;
	D3D11_TEXTURE2D_DESC texDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	// depth stencil texture (buffer)
	ZEROMEM(texDesc);
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = m_textureArraySize;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = MULTISAMPLINGLEVEL;
	texDesc.SampleDesc.Quality = MULTISAMPLINGQUALITY;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	HR(chkHr(m_Device->CreateTexture2D(&texDesc, NULL, &DepthMapTexture)));
	d3dLiveName(DepthMapTexture, m_LiveName + " depth map texture2d");

	// depth stencil view
	ZEROMEM(dsvDesc);
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if(m_textureArraySize > 1) {
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.ArraySize = m_textureArraySize;
		dsvDesc.Texture2DArray.FirstArraySlice = 0;
		dsvDesc.Texture2DArray.MipSlice = 0;
	} else {
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
	}	
	HR(chkHr(m_Device->CreateDepthStencilView(DepthMapTexture, &dsvDesc, &m_DepthMapDSV)));
	d3dLiveName(m_DepthMapDSV, m_LiveName + " depth map DSV");

	// shader depth resource view
	ZEROMEM(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	if(m_textureArraySize > 1) {
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = m_textureArraySize;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
	} else {
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}
	HR(chkHr(m_Device->CreateShaderResourceView(DepthMapTexture, &srvDesc, &m_DepthMapSRV)));
	d3dLiveName(m_DepthMapSRV, m_LiveName + " depth map SRV");
	
	// shader stencil resource view
	srvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
	HR(chkHr(m_Device->CreateShaderResourceView(DepthMapTexture, &srvDesc, &m_StencilMapSRV)));
	d3dLiveName(m_StencilMapSRV, m_LiveName + " stencil map SRV");

	safe_delete(DepthMapTexture);
	

	return hr;
}

ID3D11RenderTargetView *RenderToTexture::getRT() {
	return m_ColorMapRTV;
}

ID3D11DepthStencilView *RenderToTexture::getDSV() {
	return m_DepthMapDSV;
}

HRESULT RenderToTexture::saveColorMap(const std::string &filename, const int index) {
	HRESULT hr = E_FAIL;
	if(m_ColorMapSRV) {
		ID3D11Resource *res = NULL;
		m_ColorMapSRV->GetResource(&res);
		hr = D3DX11SaveTextureToFile(m_Context, res, D3DX11_IFF_PNG, str2wstr(filename).c_str());
	}
	return hr;
}

D3D11_VIEWPORT RenderToTexture::getVP() {
	return m_Viewport;
}

HRESULT RenderToTexture::saveDepthMap(const std::string &filename, const int index) {
	HRESULT hr = S_OK;
	ID3D11Resource *res = NULL;
	m_DepthMapSRV->GetResource(&res);
	hr = D3DX11SaveTextureToFile(m_Context, res, D3DX11_IFF_DDS, str2wstr(filename).c_str());
	return hr;
}

HRESULT RenderToTexture::saveStencilMap(const std::string &filename) {
	HRESULT hr = S_OK;
	ID3D11Resource *res = NULL;
	m_StencilMapSRV->GetResource(&res);
	hr = D3DX11SaveTextureToFile(m_Context, res, D3DX11_IFF_DDS, str2wstr(filename).c_str());
	return hr;
}
