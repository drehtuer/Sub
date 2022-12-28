#ifndef SUBSURFACESCATTER_LIGHTSOURCE_H
#define SUBSURFACESCATTER_LIGHTSOURCE_H

#include "DXLib/Utilities.h"
#include "DXLib/MoveableObject.h"
#include "DXLib/Structs.h"
#include <D3DX10math.h>

namespace SubSurfaceScatter {
	class DLLE LightSource : public MoveableObject {
	public:
		LightSource();
		
		bool setColor(const D3DXVECTOR3 color);
		bool setAttenuation(const D3DXVECTOR4 attenuation);
		bool setParameters(const float fovy, const float aspect, const float zNear, const float zFar);
		bool setAspectRatio(const float aspect);
		float getZNear() const;
		float getZFar() const;
		bool setFovy(const float fovy);
		bool setZNear(const float zNear);
		bool setZFar(const float zFar);
		D3DXVECTOR3 getColor() const;
		D3DXVECTOR4 getAttenuation() const;
		D3DXMATRIX getWorld2LightProjMatrix() const;
		void updateMatrices();

	private:
		D3DXMATRIX m_projectionMatrix;
		D3DXVECTOR3 m_color;
		D3DXVECTOR4 m_attenuation;
		float m_fovy, m_aspect, m_zNear, m_zFar;
		bool m_isEnabled;
	};
};

#endif
