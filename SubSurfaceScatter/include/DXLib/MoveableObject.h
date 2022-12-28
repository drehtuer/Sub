#ifndef SUBSURFACESCATTER_MOVEABLEOBJECT_H
#define SUBSURFACESCATTER_MOVEABLEOBJECT_H

#include "DXLib/Utilities.h"
#include "D3DX10math.h"

namespace SubSurfaceScatter {
	class DLLE MoveableObject {
	public:
		MoveableObject();
		
		bool setRotationSelf(const D3DXQUATERNION Q);
		bool setTranslation(const D3DXVECTOR3 t);
		bool setTranslation(const D3DXMATRIX T);
		bool setRotationGlobal(const D3DXQUATERNION Q);
		void setEnabled(const bool enabled);
		bool isEnabled() const;

		virtual void updateMatrices();

		D3DXMATRIX getWorld2ObjectMatrix() const;
		D3DXMATRIX getObject2WorldMatrix() const;
		D3DXVECTOR4 getWorldTranslation() const;

	private:
		D3DXMATRIX m_Object2WorldMatrix, m_World2ObjectMatrix, m_rotationSelfMatrix, m_translationMatrix, m_rotationGlobalMatrix;
		bool m_isEnabled;
	};
};

#endif
