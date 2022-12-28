#ifndef SUBSURFACESCATTER_DXLIB_QUAD_H
#define SUBSURFACESCATTER_DXLIB_QUAD_H

#include "DXLib/Utilities.h"
#include <D3D11.h>
#include "DXLib/Structs.h"
#include <string>

namespace SubSurfaceScatter {
	class DLLE Quad  {
	public:
		Quad();
		virtual ~Quad();

		// this function is based on Frank D. Luna's book  'Introduction To 3D Game Programming With Direct3D 10'
		void init(ID3D11Device *Device, ID3D11DeviceContext *Context, const DWORD pointsX, const DWORD pointsY, const float cellSpaceing);
		void setShaders(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
		void draw();

	private:
		ID3D11DeviceContext *m_Context;
		ID3D11Buffer *m_InputBuffer, *m_IndexBuffer;
		DWORD m_vertexCount, m_faceCount;
		std::string m_VSName, m_GSName, m_PSName, m_CSName;
	};
}

#endif
