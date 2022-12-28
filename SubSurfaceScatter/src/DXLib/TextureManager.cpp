#include "DXLib/TextureManager.h"
#include <boost/algorithm/string.hpp>

using namespace SubSurfaceScatter;

TextureManager *TextureManager::m_Singleton = NULL;

TextureManager::TextureManager()
	: m_isInitialized(false),
	  m_Device(NULL),
	  m_Context(NULL)
{

}

TextureManager::~TextureManager() {
	safe_delete_vector(m_SRVs);
	safe_delete_vector(m_Samplers);
	for(UINT i=0; i<m_UAVs.size(); ++i) {
		delete m_UAVs[i];
	}
	m_UAVs.clear();
}

TextureManager *TextureManager::getInstance() {
	if(!m_Singleton)
		m_Singleton = new TextureManager();
	return m_Singleton;
}


void TextureManager::init(ID3D11Device *Device, ID3D11DeviceContext *Context) {
	m_Device = Device;
	m_Context = Context;
	m_isInitialized = true;
}

bool TextureManager::isInitialized() const {
	return m_isInitialized;
}


HRESULT TextureManager::addTexture(const std::string &TextureName, const std::wstring &Filename, D3DX11_IMAGE_LOAD_INFO &ImageInfo) {
	
	if(!m_isInitialized)
		return E_FAIL;

	LOGG << "\tLoading texture '"+TextureName+"'";
	ID3D11ShaderResourceView *SRV = NULL;
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(m_Device, Filename.c_str(), &ImageInfo, NULL, &SRV, NULL);
	if(FAILED(hr)) {
		LOGG << "Could not create SRV '"+TextureName+"' from file '"+wstr2str(Filename)+": "+wstr2str(getErrorMessage());
		return hr;
	}
	d3dLiveName(SRV, TextureName);

	if(ImageInfo.MipLevels > 1)
		m_Context->GenerateMips(SRV);

	std::vector<std::string>::iterator itr = std::find(m_SRVNames.begin(), m_SRVNames.end(), TextureName);
	if(itr != m_SRVNames.end()) {
		size_t index = std::distance(m_SRVNames.begin(), itr);
		safe_delete(m_SRVs[index]);
		m_SRVs[index] = SRV;
	} else {
		m_SRVNames.push_back(TextureName);
		m_SRVs.push_back(SRV);
	}
	return hr;
}

void TextureManager::addTexture(const std::string &TextureName, ID3D11ShaderResourceView *SRV) {
	std::vector<std::string>::iterator itr = std::find(m_SRVNames.begin(), m_SRVNames.end(), TextureName);
	if(itr != m_SRVNames.end()) {
		size_t index = std::distance(m_SRVNames.begin(), itr);
		//safe_delete(m_SRVs[index]); // most likely the same SRV, so no deleting
		m_SRVs[index] = SRV;
	} else {
		m_SRVNames.push_back(TextureName);
		m_SRVs.push_back(SRV);
	}
}

ID3D11ShaderResourceView *TextureManager::getSRV(const std::string &TextureName) const {
	std::vector<std::string>::const_iterator itr = std::find(m_SRVNames.begin(), m_SRVNames.end(), TextureName);
	if(itr != m_SRVNames.end()) {
		size_t index = std::distance(m_SRVNames.begin(), itr);
		return m_SRVs[index];
	} else
		return NULL;
}

std::vector<ID3D11ShaderResourceView*> TextureManager::getSRVs() const {
	return m_SRVs;
}

std::vector<std::string> TextureManager::getTextureNames() const {
	return m_SRVNames;
}

void TextureManager::saveTexture(const std::string &TextureName, ID3D11DeviceContext *Context) const {
	HRESULT hr = S_OK;
	ID3D11Resource *res = NULL;
	ID3D11ShaderResourceView *SRV = getSRV(TextureName);
	SRV->GetResource(&res);
	hr = D3DX11SaveTextureToFile(Context, res, D3DX11_IFF_PNG, str2wstr(TextureName+".png").c_str());
}


void TextureManager::getShaderMacros(std::vector<ShaderMacro> &Macros) const {
	// textures
	for(size_t i=0; i<m_SRVNames.size(); ++i) {
		ShaderMacro S;
		S.Name = "TEXTURE_" + boost::to_upper_copy(m_SRVNames[i]) + "_ID";
		S.Definition = num2str(i);
		Macros.push_back(S);
	}
	for(size_t i=0; i<m_UAVNames.size(); ++i) {
		ShaderMacro S;
		S.Name = "RWTEXTURE_" + boost::to_upper_copy(m_UAVNames[i]) + "_ID";
		S.Definition = num2str(i + m_SRVNames.size());
		Macros.push_back(S);
	}

	// samplers
	for(size_t i=0; i<m_SamplerNames.size(); ++i) {
		ShaderMacro S;
		S.Name = "SAMPLER_" + boost::to_upper_copy(m_SamplerNames[i]) + "_ID";
		S.Definition = num2str(i);
		Macros.push_back(S);
	}
}

ID3D11ShaderResourceView* TextureManager::swap(const std::string &TextureReplace, ID3D11ShaderResourceView *SRV_New) {
	std::vector<std::string>::iterator itr = std::find(m_SRVNames.begin(), m_SRVNames.end(), TextureReplace);
	if(itr != m_SRVNames.end()) {
		size_t indexOld = std::distance(m_SRVNames.begin(), itr);
		ID3D11ShaderResourceView *SRV_Old = m_SRVs[indexOld];
		m_SRVs[indexOld] = SRV_New;
		return SRV_Old;
	}
	return NULL;
}



HRESULT TextureManager::addSampler(const std::string &SamplerName, D3D11_SAMPLER_DESC &SamplerDesc) {

	if(!m_isInitialized)
		return E_FAIL;

	ID3D11SamplerState *Sampler = NULL;
	HRESULT hr = m_Device->CreateSamplerState(&SamplerDesc, &Sampler);
	if(FAILED(hr)) {
		LOGG << "Could not create SamplerState '"+SamplerName+"': "+wstr2str(getErrorMessage());
		return hr;
	}
	d3dLiveName(Sampler, SamplerName);

	std::vector<std::string>::iterator itr = std::find(m_SamplerNames.begin(), m_SamplerNames.end(), SamplerName);
	if(itr != m_SamplerNames.end()) {
		size_t index = std::distance(m_SamplerNames.begin(), itr);
		safe_delete(m_Samplers[index]);
		m_Samplers[index] = Sampler;
	} else {
		m_SamplerNames.push_back(SamplerName);
		m_Samplers.push_back(Sampler);
	}
	return hr;
}

ID3D11SamplerState *TextureManager::getSampler(const std::string &SamplerName) const {
	std::vector<std::string>::const_iterator itr = std::find(m_SamplerNames.begin(), m_SamplerNames.end(), SamplerName);
	if(itr != m_SamplerNames.end()) {
		size_t index = std::distance(m_SamplerNames.begin(), itr);
		return m_Samplers[index];
	} else
		return NULL;
}

std::vector<ID3D11SamplerState*> TextureManager::getSamplers() const {
	return m_Samplers;
}

void TextureManager::addUAV(const std::string &UAVName, UAVTexture *UAV) {
	m_UAVNames.push_back(UAVName);
	m_UAVs.push_back(UAV);
}

UAVTexture *TextureManager::getUAV(const std::string &UAVName) const {
	std::vector<std::string>::const_iterator itr = std::find(m_UAVNames.begin(), m_UAVNames.end(), UAVName);
	if(itr != m_UAVNames.end()) {
		size_t index = std::distance(m_UAVNames.begin(), itr);
		return m_UAVs[index];
	} else
		return NULL;
}

std::vector<std::string> TextureManager::getUAVNames() const {
	return m_UAVNames;
}

std::vector<UAVTexture*> TextureManager::getUAVS() const {
	return m_UAVs;
}
