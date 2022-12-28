#ifndef SUBSURFACESCATTER_VIRTUALOBJECT_H
#define SUBSURFACESCATTER_VIRTUALOBJECT_H

#include "DXLib/Utilities.h"
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10math.h>
#include <map>
#include <vector>
#include <string>
#include "DXLib/MoveableObject.h"
#include "DXLib/Structs.h"
#include "DXLib/ShaderManager.h"
#include "DXLib/TextureManager.h"

namespace SubSurfaceScatter {

	class DLLE VirtualObject : public MoveableObject {
	public:
		VirtualObject();
		virtual ~VirtualObject();
		virtual HRESULT loadModel() = 0;
		HRESULT init(ID3D11Device *Device, ID3D11DeviceContext *Context);
		UINT getVertexCount(const std::string &Name);
		UINT getFaceCount(const std::string &Name);
		bool isEnabled() const;
		void setEnabled(const bool enabled);
		std::string getName() const;

		virtual ID3D11InputLayout *getInputLayout() const;

		virtual void drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) = 0;
		virtual void drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName) = 0;
		void draw(const std::string &MeshName, const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);

	protected:
		virtual HRESULT loadMesh(const std::string &Filename);

		std::map<std::string, ID3D11Buffer*> m_VertexBuffer;
		std::map<std::string, ID3D11Buffer*> m_IndexBuffer;
		std::map<std::string, UINT> m_vertexCount;
		std::map<std::string, UINT> m_faceCount;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputDescs;

		ID3D11Device *m_Device;
		ID3D11DeviceContext *m_Context;
		std::string m_ObjectName;
		TextureManager *m_TextureManager;
		ShaderManager *m_ShaderManager;

	private:
		bool m_enabled;
	};
};

#endif
