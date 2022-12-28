#ifndef SUBSURFACESCATTER_SHADERMANAGER_H
#define SUBSURFACESCATTER_SHADERMANAGER_H

#include "DXLib/Utilities.h"

#include <D3D11.h>

#include <string>
#include <map>
#include <ctime>
#include <vector>

#include "DXLib/Structs.h"

namespace SubSurfaceScatter {
	
	class DLLE ShaderManager {
	public:
		ShaderManager();
		virtual ~ShaderManager();

		static ShaderManager *getInstance();

		void init(ID3D11Device *Device);
		bool isInitialized() const;

		void addShaderMacros(const std::vector<ShaderMacro> &Macros);

		void reloadShaders();

		void createIL(const std::string &LayoutName, const D3D11_INPUT_ELEMENT_DESC layout[], const int layoutSize, ID3DBlob *ShaderBlob);
		void createVS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, ID3DBlob **VSBlob, const std::vector<ShaderMacro> &AddMacros = std::vector<ShaderMacro>());
		void createGS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, const std::vector<ShaderMacro> &AddMacros = std::vector<ShaderMacro>());
		void createPS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, const std::vector<ShaderMacro> &AddMacros = std::vector<ShaderMacro>());
		void createCS(const std::string &ShaderName, const std::wstring &Filename, const std::string &EntryPoint, const std::vector<ShaderMacro> &AddMacros = std::vector<ShaderMacro>());

		ID3D11VertexShader *getVS(const std::string &ShaderName) const;
		ID3D11GeometryShader *getGS(const std::string &ShaderName) const;
		ID3D11PixelShader *getPS(const std::string &ShaderName) const;
		ID3D11ComputeShader *getCS(const std::string &ShaderName) const;
		ID3D11InputLayout *getIL(const std::string &LayoutName) const;
		
	private:
		HRESULT compileShaderFromFile(const std::wstring &Filename, const std::string &EntryPoint, const std::string &ShaderModel, ID3DBlob **ShaderBlob, std::vector<D3D10_SHADER_MACRO> &Macros);
		std::time_t getLastModified(const std::wstring &Filename) const;

		static ShaderManager *m_Singleton;
		
		std::map<std::string, ID3D11InputLayout*> m_IL;
		std::map<std::string, ShaderVS*> m_VS;
		std::map<std::string, ShaderGS*> m_GS;
		std::map<std::string, ShaderPS*> m_PS;
		std::map<std::string, ShaderCS*> m_CS;
		std::map<std::string, std::string> m_ShaderProfiles;

		std::vector<D3D10_SHADER_MACRO> m_ShaderMacros;
		std::vector<ShaderMacro> m_ShaderMacroStructs;

		ID3D11Device *m_Device;

		bool m_isInitialized;
	};
}

#endif
