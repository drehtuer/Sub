#include "DXLib/MoveableObject.h"

using namespace SubSurfaceScatter;

MoveableObject::MoveableObject()
	: m_isEnabled(true)
{
	D3DXMatrixIdentity(&m_World2ObjectMatrix);
	D3DXMatrixIdentity(&m_Object2WorldMatrix);
	D3DXMatrixIdentity(&m_translationMatrix);
	D3DXMatrixIdentity(&m_rotationGlobalMatrix);
	D3DXMatrixIdentity(&m_rotationSelfMatrix);
}
		
bool MoveableObject::setTranslation(const D3DXVECTOR3 t) {
	D3DXMATRIX tmp;
	D3DXMatrixTranslation(&tmp, t.x, t.y, t.z);
	if(tmp != m_translationMatrix) {
		m_translationMatrix = tmp;
		return true;
	} else
		return false;
}

bool MoveableObject::setRotationSelf(const D3DXQUATERNION Q) {
	D3DXMATRIX tmp;
	D3DXMatrixRotationQuaternion(&tmp, &Q);
	if(tmp != m_rotationSelfMatrix) {
		m_rotationSelfMatrix = tmp;
		return true;
	} else
		return false;
}

void MoveableObject::updateMatrices() {
	m_World2ObjectMatrix = m_rotationGlobalMatrix * m_translationMatrix * m_rotationSelfMatrix;
	D3DXMatrixInverse(&m_Object2WorldMatrix, NULL, &m_World2ObjectMatrix);
}

D3DXMATRIX MoveableObject::getWorld2ObjectMatrix() const {
	return m_World2ObjectMatrix;
}

D3DXMATRIX MoveableObject::getObject2WorldMatrix() const {
	return m_Object2WorldMatrix;
}

D3DXVECTOR4 MoveableObject::getWorldTranslation() const {
	return D3DXVECTOR4(m_World2ObjectMatrix(3, 0), m_World2ObjectMatrix(3, 1), m_World2ObjectMatrix(3, 2), 1.0f);
}

bool MoveableObject::setTranslation(const D3DXMATRIX T) {
	if(T != m_translationMatrix) {
		m_translationMatrix = T;
		return true;
	} else
		return false;
}

bool MoveableObject::setRotationGlobal(const D3DXQUATERNION Q) {
	D3DXMATRIX tmp;
	D3DXMatrixRotationQuaternion(&tmp, &Q);
	if(tmp != m_rotationGlobalMatrix) {
		m_rotationGlobalMatrix = tmp;
		return true;
	} else
		return false;
}

void MoveableObject::setEnabled(const bool enabled) {
	m_isEnabled = enabled;
}

bool MoveableObject::isEnabled() const {
	return m_isEnabled;
}
