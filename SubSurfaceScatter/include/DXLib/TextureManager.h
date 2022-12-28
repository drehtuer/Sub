#ifndef SUBSURFACESCATTER_TEXTUREMANAGER_H
#define SUBSURFACESCATTER_TEXTUREMANAGER_H

#include "DXLib/Utilities.h"
#include <D3D11.h>
#include <D3DX11.h>
#include <string>
#include <vector>
#include "DXLib/Structs.h"
#include "DXLib/UAVTexture.h"

namespace SubSurfaceScatter {

	class DLLE TextureManager {
	public:
		TextureManager();
		virtual ~TextureManager();
		static TextureManager *getInstance();

		void init(ID3D11Device *Device, ID3D11DeviceContext *Context);
		bool isInitialized() const;

		HRESULT addTexture(const std::string &TextureName, const std::wstring &Filename, D3DX11_IMAGE_LOAD_INFO &ImageInfo);
		void addTexture(const std::string &TextureName, ID3D11ShaderResourceView *SRV);
		ID3D11ShaderResourceView *getSRV(const std::string &TextureName) const;
		std::vector<ID3D11ShaderResourceView*> getSRVs() const;
		void getShaderMacros(std::vector<ShaderMacro> &Macros) const;
		std::vector<std::string> getTextureNames() const;
		void saveTexture(const std::string &TextureName, ID3D11DeviceContext *Context) const;
		ID3D11ShaderResourceView* swap(const std::string &TextureReplace, ID3D11ShaderResourceView *SRV_New);

		HRESULT addSampler(const std::string &SamplerName, D3D11_SAMPLER_DESC &SamplerDesc);
		ID3D11SamplerState *getSampler(const std::string &SamplerName) const;
		std::vector<ID3D11SamplerState*> getSamplers() const;


		void addUAV(const std::string &UAVName, UAVTexture *UAV);
		UAVTexture *getUAV(const std::string &UAVName) const;
		std::vector<std::string> getUAVNames() const;
		std::vector<UAVTexture*> getUAVS() const;

	private:
		static TextureManager *m_Singleton;
		ID3D11Device *m_Device;
		ID3D11DeviceContext *m_Context;
		bool m_isInitialized;
		 // vector, because it keeps the input order and offers easy access via .data() or &[0]
		std::vector<std::string> m_SRVNames;
		std::vector<ID3D11ShaderResourceView*> m_SRVs;
		std::vector<std::string> m_SamplerNames;
		std::vector<ID3D11SamplerState*> m_Samplers;
		std::vector<std::string> m_UAVNames;
		std::vector<UAVTexture*> m_UAVs;
	};
}

#endif
