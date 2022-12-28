#ifndef SUBSURFACESCATTER_RENDERTOTEXTURE_H
#define SUBSURFACESCATTER_RENDERTOTEXTURE_H

#include "DXLib/Utilities.h"
#include <D3D11.h>
#include <D3DX11.h>
#include <string>

namespace SubSurfaceScatter {

	enum RENDERTOTEXTURE_OPTIONS {
		RENDERTOTEXTURE_OPTIONS_USECOLORMAP = 1,
		RENDERTOTEXTURE_OPTIONS_USEDEPTHMAP = 1 << 1,
		RENDERTOTEXTURE_OPTIONS_ISRESIZABLE = 1 << 2,
		RENDERTOTEXTURE_OPTIONS_USE_MIPMAPS = 1 << 3,
	};


	class DLLE RenderToTexture {
	public:
		RenderToTexture();
		virtual ~RenderToTexture();

		HRESULT init(ID3D11Device *Device, ID3D11DeviceContext *Context, const UINT width, const UINT height, const DXGI_FORMAT colorFormat, const UINT options, const std::string &liveName, const int numRenderTargets = 1);
		ID3D11ShaderResourceView *getColorMap();
		ID3D11ShaderResourceView *getDepthMap();
		void getDepthMapCopy(ID3D11ShaderResourceView *CopyTo);
		ID3D11ShaderResourceView *getStencilMap();
		void getStencilMapCopy(ID3D11ShaderResourceView *CopyTo);
		ID3D11RenderTargetView *getRT();
		ID3D11DepthStencilView *getDSV();
		D3D11_VIEWPORT getVP();
		HRESULT resize(const UINT width, const UINT height);

		void begin(const D3DXVECTOR4 &ClearColor, const UINT clearFlag, ID3D11DepthStencilView *DSV = NULL);
		void end();

		HRESULT saveDepthMap(const std::string &filename, const int index = 0);
		HRESULT saveColorMap(const std::string &filename, const int index = 0);
		HRESULT saveStencilMap(const std::string &filename);

	private:
		HRESULT buildColorMaps();
		HRESULT buildDepthMaps();

		UINT m_width, m_height;
		DXGI_FORMAT m_colorMapFormat;
		ID3D11Device *m_Device;
		ID3D11DeviceContext *m_Context;
		ID3D11ShaderResourceView *m_ColorMapSRV, *m_DepthMapSRV, *m_StencilMapSRV;
		ID3D11RenderTargetView *m_ColorMapRTV;
		ID3D11DepthStencilView *m_DepthMapDSV;
		D3D11_VIEWPORT m_Viewport;
		UINT m_textureArraySize;
		bool m_hasColorMap, m_hasDepthMap, m_isResizable;
		std::string m_LiveName;
		UINT m_mipmapLevel;
	};
};

#endif
