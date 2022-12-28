#ifndef SUBSURFACESCATTER_H
#define SUBSURFACESCATTER_H

#include "DXLib/Utilities.h"
#include <D3DX10math.h>
#include "DXLib/MoveableObject.h"
#include <vector>

namespace SubSurfaceScatter {
	class DLLE Camera : public MoveableObject {
	public:
		Camera();

		bool setPerspective(const float fovy, const float aspect, const float zNear, const float zFar);
		bool setAspectRatio(float aspect);
		
		bool setFovy(const float fovy);
		bool setZNear(const float zNear);
		bool setZFar(const float zFar);
		D3DXMATRIX getProjectionMatrix() const;
		D3DXMATRIX getWorld2CameraProjMatrix() const;
		float getZNear() const;
		float getZFar() const;
		void updateMatrices();

	private:
		D3DXMATRIX m_projectionMatrix;
		float m_fovy, m_aspect, m_zNear, m_zFar;
	};
};

#endif
