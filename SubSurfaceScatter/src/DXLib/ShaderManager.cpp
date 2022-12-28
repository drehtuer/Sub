#include "DXLib/ShaderManager.h"
#include "D3Dcompiler.h"
#include <D3DX11.h>
#include <boost/filesystem.hpp>

#define MORE_OPTIMIZATIONS 1

using namespace SubSurfaceScatter;

ShaderManager *ShaderManager::m_Singleton = NULL;

ShaderManager::ShaderManager() 
	: m_isInitialized(false),
	  m_Device(NULL)
{
}

ShaderManager::~ShaderManager() {
	safe_delete_map(m_VS);
	safe_delete_map(m_GS);
	safe_delete_map(m_PS);
	safe_delete_map(m_CS);
}

ShaderManager *ShaderManager::getInstance() {
	if(!m_Singleton)
		m_Singleton = new ShaderManager();
	return m_Singleton;
}


void ShaderManager::init(ID3D11Device *Device) {
	m_Device = Device;
	D3D10_SHADER_MACRO DXVersionMacro;
	DXVersionMacro.Name = "DIRECTX_VERSION";
	switch(Device->GetFeatureLevel()) {
	case D3D_FEATURE_LEVEL_10_0:
		m_ShaderProfiles["VS"] = "vs_4_0";
		m_ShaderProfiles["GS"] = "gs_4_0";
		m_ShaderProfiles["PS"] = "ps_4_0";
		m_ShaderProfiles["CS"] = "cs_4_0";
		DXVersionMacro.Definition = "10";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		m_ShaderProfiles["VS"] = "vs_4_1";
		m_ShaderProfiles["GS"] = "gs_4_1";
		m_ShaderProfiles["PS"] = "ps_4_1";
		m_ShaderProfiles["CS"] = "cs_4_1";
		DXVersionMacro.Definition = "10_1";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		m_ShaderProfiles["VS"] = "vs_5_0";
		m_ShaderProfiles["GS"] = "gs_5_0";
		m_ShaderProfiles["PS"] = "ps_5_0";
		m_ShaderProfiles["CS"] = "cs_5_0";
		DXVersionMacro.Definition = "11";
		break;
	default:
		m_ShaderProfiles["VS"] = "vs_4_0";
		m_ShaderProfiles["GS"] = "gs_4_0";
		m_ShaderProfiles["PS"] = "ps_4_0";
		m_ShaderProfiles["CS"] = "cs_4_0";
		break;
	}
	m_ShaderMacros.push_back(DXVersionMacro);
	if(m_Device)
		m_isInitialized = true;
}

bool ShaderManager::isInitialized() const {
	return m_isInitialized;
}


void ShaderManager::addShaderMacros(const std::vector<ShaderMacro> &Macros) {
	m_ShaderMacroStructs = Macros;
	D3D10_SHADER_MACRO M;
	for(size_t i=0; i<m_ShaderMacroStructs.size(); ++i) {
		M.Name = m_ShaderMacroStructs[i].Name.c_str();
		M.Definition = m_ShaderMacroStructs[i].Definition.c_str();
		m_ShaderMacros.push_back(M);
	}
}


void ShaderManager::reloadShaders() {
	if(!isInitialized())
		return;

	for(std::map<std::string, ShaderVS*>::iterator itr=m_VS.begin(); itr!=m_VS.end(); ++itr) {
		if(getLastModified(itr->second->Filename) > itr->second->lastModified) {
			ID3DBlob *Blob = NULL;
			createVS(itr->first, itr->second->Filename, itr->second->EntryPoint, &Blob, itr->second->Macros);
		}
	}
	
	for(std::map<std::string, ShaderGS*>::iterator itr=m_GS.begin(); itr!=m_GS.end(); ++itr) {
		if(getLastModified(itr->second->Filename) > itr->second->lastModified) {
			createGS(itr->first, itr->second->Filename, itr->second->EntryPoint, itr->second->Macros);
		}
	}
	
	for(std::map<std::string, ShaderPS*>::iterator itr=m_PS.begin(); itr!=m_PS.end(); ++itr) {
		if(getLastModified(itr->second->Filename) > itr->second->lastModified) {
			createPS(itr->first, itr->second->Filename, itr->second->EntryPoint, itr->second->Macros);
		}
	}

	for(std::map<std::string, ShaderCS*>::iterator itr=m_CS.begin(); itr!=m_CS.end(); ++itr) {
		if(getLastModified(itr->second->Filename) > itr->second->lastModified) {
			createCS(itr->first, itr->second->Filename, itr->second->EntryPoint, itr->second->Macros);
		}
	}
}


// IL
void ShaderManager::createIL(const std::string &LayoutName, const D3D11_INPUT_ELEMENT_DESC layout[], const int layoutSize, ID3DBlob *ShaderBlob) {
	if(!isInitialized())
		return;
	HRESULT hr = S_OK;
	ID3D11InputLayout *IL = NULL;
	hr = m_Device->CreateInputLayout(layout, layoutSize, ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), &IL);
	safe_delete(ShaderBlob);
	if(FAILED(hr)) {
		LOGG << "Could not create Input Layout '"+LayoutName+"': " + wstr2str(getErrorMessage());
		return;
	}
	m_IL[LayoutName] = IL;
}


// VS
void ShaderManager::createVS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, ID3DBlob **VSBlob, const std::vector<ShaderMacro> &AddMacros) {
	if(!isInitialized())
		return;

	LOGG<<"\tLoading VS '"+ShaderName+"'";
	
	ShaderVS *SVS = new ShaderVS();
	SVS->Filename = Filename;
	SVS->EntryPoint = EntryPoint;
	SVS->Profile = m_ShaderProfiles["VS"];
	SVS->lastModified = getLastModified(Filename);
	SVS->Macros = AddMacros;
	SVS->VS = NULL;

	std::map<std::string, ShaderVS*>::iterator oldVS = m_VS.find(ShaderName);

	// shader specific macros
	D3D10_SHADER_MACRO M;
	std::vector<D3D10_SHADER_MACRO> ShaderMacros = m_ShaderMacros;
	for(UINT i=0; i<(UINT)SVS->Macros.size(); ++i) {
		M.Name = SVS->Macros[i].Name.c_str();
		M.Definition = SVS->Macros[i].Definition.c_str();
		ShaderMacros.push_back(M);
	}
	M.Name = NULL;
	M.Definition = NULL;
	ShaderMacros.push_back(M);
	
	SVS->lastHR = compileShaderFromFile(SVS->Filename, SVS->EntryPoint, SVS->Profile, VSBlob, ShaderMacros);
	if(FAILED(SVS->lastHR)) {
		LOGG << "Could not compile vertex shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		// even if creation failed, register previously unregistered shader
		if(oldVS == m_VS.end())
			m_VS[ShaderName] = SVS;
		return;
	}

	SVS->lastHR = m_Device->CreateVertexShader((*VSBlob)->GetBufferPointer(), (*VSBlob)->GetBufferSize(), NULL, &SVS->VS);
	if(FAILED(SVS->lastHR)) {
		LOGG << "Could not create vertex shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		safe_delete(*VSBlob);
		// even if creation failed, register previously unregistered shader
		if(oldVS == m_VS.end())
			m_VS[ShaderName] = SVS;
		return;
	}
	d3dLiveName(SVS->VS, ShaderName);

	if(oldVS != m_VS.end())
		delete oldVS->second;
	m_VS[ShaderName] = SVS;
}


// GS
void ShaderManager::createGS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, const std::vector<ShaderMacro> &AddMacros) {
	if(!isInitialized())
		return;

	LOGG<<"\tLoading GS '"+ShaderName+"'";
	
	ShaderGS *SGS = new ShaderGS();
	SGS->Filename = Filename;
	SGS->EntryPoint = EntryPoint;
	SGS->Profile = m_ShaderProfiles["GS"];
	SGS->lastModified = getLastModified(Filename);
	SGS->Macros = AddMacros;
	SGS->GS = NULL;

	std::map<std::string, ShaderGS*>::iterator oldGS = m_GS.find(ShaderName);

	// shader specific macros
	D3D10_SHADER_MACRO M;
	std::vector<D3D10_SHADER_MACRO> ShaderMacros = m_ShaderMacros;
	for(UINT i=0; i<(UINT)SGS->Macros.size(); ++i) {
		M.Name = SGS->Macros[i].Name.c_str();
		M.Definition = SGS->Macros[i].Definition.c_str();
		ShaderMacros.push_back(M);
	}
	M.Name = NULL;
	M.Definition = NULL;
	ShaderMacros.push_back(M);

	ID3DBlob *GSBlob = NULL;
	SGS->lastHR = compileShaderFromFile(SGS->Filename, SGS->EntryPoint, SGS->Profile, &GSBlob, ShaderMacros);
	if(FAILED(SGS->lastHR)) {
		LOGG << "Could not compile geometry shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		if(oldGS == m_GS.end())
			m_GS[ShaderName] = SGS;
		return;
	}

	SGS->lastHR = m_Device->CreateGeometryShader(GSBlob->GetBufferPointer(), GSBlob->GetBufferSize(), NULL, &SGS->GS);
	if(FAILED(SGS->lastHR)) {
		LOGG << "Could not create geometry shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		safe_delete(GSBlob);
		if(oldGS == m_GS.end())
			m_GS[ShaderName] = SGS;
		return;
	}
	d3dLiveName(SGS->GS, ShaderName);

	
	if(oldGS != m_GS.end())
		delete oldGS->second;
	m_GS[ShaderName] = SGS;
}

// PS
void ShaderManager::createPS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, const std::vector<ShaderMacro> &AddMacros) {
	if(!isInitialized())
		return;

	LOGG<<"\tLoading PS '"+ShaderName+"'";
	
	ShaderPS *SPS = new ShaderPS();
	SPS->Filename = Filename;
	SPS->EntryPoint = EntryPoint;
	SPS->Profile = m_ShaderProfiles["PS"];
	SPS->lastModified = getLastModified(Filename);
	SPS->Macros = AddMacros;
	SPS->PS = NULL;

	std::map<std::string, ShaderPS*>::iterator oldPS = m_PS.find(ShaderName);

	// shader specific macros
	D3D10_SHADER_MACRO M;
	std::vector<D3D10_SHADER_MACRO> ShaderMacros = m_ShaderMacros;
	for(UINT i=0; i<(UINT)SPS->Macros.size(); ++i) {
		M.Name = SPS->Macros[i].Name.c_str();
		M.Definition = SPS->Macros[i].Definition.c_str();
		ShaderMacros.push_back(M);
	}
	M.Name = NULL;
	M.Definition = NULL;
	ShaderMacros.push_back(M);

	ID3DBlob *PSBlob = NULL;
	SPS->lastHR = compileShaderFromFile(SPS->Filename, SPS->EntryPoint, SPS->Profile, &PSBlob, ShaderMacros);
	if(FAILED(SPS->lastHR)) {
		LOGG << "Could not compile pixel shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		if(oldPS == m_PS.end())
			m_PS[ShaderName] = SPS;
		return;
	}

	SPS->lastHR = m_Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), NULL, &SPS->PS);
	if(FAILED(SPS->lastHR)) {
		LOGG << "Could not create pixel shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		safe_delete(PSBlob);
		if(oldPS == m_PS.end())
			m_PS[ShaderName] = SPS;
		return;
	}
	d3dLiveName(SPS->PS, ShaderName);

	
	if(oldPS != m_PS.end())
		delete oldPS->second;
	m_PS[ShaderName] = SPS;
}

// CS
void ShaderManager::createCS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, const std::vector<ShaderMacro> &AddMacros) {
	if(!isInitialized())
		return;

	LOGG<<"\tLoading CS '"+ShaderName+"'";
	
	ShaderCS *SCS = new ShaderCS();
	SCS->Filename = Filename;
	SCS->EntryPoint = EntryPoint;
	SCS->Profile = m_ShaderProfiles["CS"];
	SCS->lastModified = getLastModified(Filename);
	SCS->Macros = AddMacros;
	SCS->CS = NULL;

	std::map<std::string, ShaderCS*>::iterator oldCS = m_CS.find(ShaderName);

	// shader specific macros
	D3D10_SHADER_MACRO M;
	std::vector<D3D10_SHADER_MACRO> ShaderMacros = m_ShaderMacros;
	for(UINT i=0; i<(UINT)SCS->Macros.size(); ++i) {
		M.Name = SCS->Macros[i].Name.c_str();
		M.Definition = SCS->Macros[i].Definition.c_str();
		ShaderMacros.push_back(M);
	}
	M.Name = NULL;
	M.Definition = NULL;
	ShaderMacros.push_back(M);

	ID3DBlob *CSBlob = NULL;
	SCS->lastHR = compileShaderFromFile(SCS->Filename, SCS->EntryPoint, SCS->Profile, &CSBlob, ShaderMacros);
	if(FAILED(SCS->lastHR)) {
		LOGG << "Could not compile compute shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		if(oldCS == m_CS.end())
			m_CS[ShaderName] = SCS;
		return;
	}

	SCS->lastHR = m_Device->CreateComputeShader(CSBlob->GetBufferPointer(), CSBlob->GetBufferSize(), NULL, &SCS->CS);
	if(FAILED(SCS->lastHR)) {
		LOGG << "Could not create compute shader '" + ShaderName + "' from file '" + wstr2str(Filename) + "': " + wstr2str(getErrorMessage());
		safe_delete(CSBlob);
		if(oldCS == m_CS.end())
			m_CS[ShaderName] = SCS;
		return;
	}
	d3dLiveName(SCS->CS, ShaderName);

	
	if(oldCS != m_CS.end())
		delete oldCS->second;
	m_CS[ShaderName] = SCS;
}


HRESULT ShaderManager::compileShaderFromFile(const std::wstring &Filename, const std::string &EntryPoint, const std::string &ShaderModel, ID3DBlob **ShaderBlob, std::vector<D3D10_SHADER_MACRO> &Macros) {
	if(!isInitialized())
		return S_OK;;

	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	#if defined _DEBUG
		shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; // | D3DCOMPILE_WARNINGS_ARE_ERRORS;
	#else
		#if MORE_OPTIMIZATIONS
			shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		#else
			shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
		#endif
	#endif

	ID3DBlob *ErrorBlob = NULL;
	hr = D3DX11CompileFromFile(
			Filename.c_str(),
			&Macros[0],
			NULL,
			EntryPoint.c_str(),
			ShaderModel.c_str(),
			shaderFlags,
			0,
			NULL,
			ShaderBlob,
			&ErrorBlob,
			NULL);
	if(FAILED(hr)) {
		if(ErrorBlob) {
			LOGG << "Shader compilation error: " + std::string((char*)ErrorBlob->GetBufferPointer());
		}
		safe_delete(ErrorBlob);
		return hr;
	}
	safe_delete(ErrorBlob);

	return hr;
}


ID3D11VertexShader *ShaderManager::getVS(const std::string &ShaderName) const {
	if(!isInitialized())
		return NULL;
	std::map<std::string, ShaderVS*>::const_iterator itr = m_VS.find(ShaderName);
	if(itr != m_VS.end())
		return itr->second->VS;
	else
		return NULL;
}

ID3D11GeometryShader *ShaderManager::getGS(const std::string &ShaderName) const {
	if(!isInitialized())
		return NULL;
	std::map<std::string, ShaderGS*>::const_iterator itr = m_GS.find(ShaderName);
	if(itr != m_GS.end())
		return itr->second->GS;
	else
		return NULL;
}

ID3D11PixelShader *ShaderManager::getPS(const std::string &ShaderName) const {
	if(!isInitialized())
		return NULL;
	std::map<std::string, ShaderPS*>::const_iterator itr = m_PS.find(ShaderName);
	if(itr != m_PS.end())
		return itr->second->PS;
	else
		return NULL;
}

ID3D11ComputeShader *ShaderManager::getCS(const std::string &ShaderName) const {
	if(!isInitialized())
		return NULL;
	std::map<std::string, ShaderCS*>::const_iterator itr = m_CS.find(ShaderName);
	if(itr != m_CS.end())
		return itr->second->CS;
	else
		return NULL;
}

ID3D11InputLayout *ShaderManager::getIL(const std::string &LayoutName) const {
	if(!isInitialized())
		return NULL;
	std::map<std::string, ID3D11InputLayout*>::const_iterator itr = m_IL.find(LayoutName);
	if(itr != m_IL.end())
		return itr->second;
	else
		return NULL;
}


std::time_t ShaderManager::getLastModified(const std::wstring &Filename) const {
	std::string P(wstr2str(Filename));
	boost::filesystem::path &path = boost::filesystem::path(P);
	return boost::filesystem::last_write_time(path);
}
