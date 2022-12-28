#ifndef SUBSURFACESCATTER_SPHERE_H
#define SUBSURFACESCATTER_SPHERE_H

#include "DXLib/Utilities.h"
#include <D3D11.h>
#include <vector>
#include "DXLib/Structs.h"
#include <D3DX10math.h>
#include <string>

namespace SubSurfaceScatter {
	class DLLE Sphere {
	public:
		Sphere();
		virtual ~Sphere();

		// this function is based on Frank D. Luna's book  'Introduction To 3D Game Programming With Direct3D 10'
		void init(ID3D11Device *Device, ID3D11DeviceContext *Context, const float radius, const UINT slices, const UINT stacks);
		void setShaders(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
		void draw();
		bool setRadius(const float radius);

	private:
		void buildStacks(const float radius, std::vector<Point3dVNT> &Vertices, std::vector<DWORD> &Indices);

		UINT m_slices, m_stacks;
		float m_radius;
		ID3D11Device *m_Device;
		ID3D11DeviceContext *m_Context;
		ID3D11Buffer *m_InputBuffer, *m_IndexBuffer;
		DWORD m_vertexCount, m_faceCount;
		std::string m_VSName, m_GSName, m_PSName, m_CSName;
	};
}

#endif
