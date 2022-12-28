
#include "DXLib/Camera.h"

using namespace SubSurfaceScatter;

Camera::Camera()
	:  m_fovy(deg2rad(45.0f)),
	   m_aspect(4/3),
	   m_zNear(200),
	   m_zFar(1000)
{
	D3DXMatrixIdentity(&m_projectionMatrix);
}

bool Camera::setPerspective(const float fovy, const float aspect, const float zNear, const float zFar) {
	if(setFovy(fovy) || setAspectRatio(aspect) || setZNear(zNear) || setZFar(zFar))
		return true;
	else
		return false;
}

bool Camera::setAspectRatio(const float aspect) {
	if(aspect != m_aspect) {
		m_aspect = aspect;
		return true;
	} else
		return false;
}

D3DXMATRIX Camera::getProjectionMatrix() const {
	return m_projectionMatrix;
}

bool Camera::setFovy(const float fovy) {
	float rfovy = deg2rad(fovy);
	if(rfovy != m_fovy) {
		m_fovy = rfovy;
		return true;
	} else
		return false;
}

D3DXMATRIX Camera::getWorld2CameraProjMatrix() const {
	D3DXMATRIX M;
	D3DXMatrixMultiply(&M, &getWorld2ObjectMatrix(), &m_projectionMatrix);
	return M;
}

float Camera::getZNear() const {
	return m_zNear;
}

float Camera::getZFar() const {
	return m_zFar;
}

bool Camera::setZNear(const float zNear) {
	if(zNear != m_zNear) {
		m_zNear = zNear;
		return true;
	} else
		return false;
}

bool Camera::setZFar(const float zFar) {
	if(zFar != m_zFar) {
		m_zFar = zFar;
		return true;
	} else
		return false;
}

void Camera::updateMatrices() {
	MoveableObject::updateMatrices();
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, m_fovy, m_aspect, m_zNear, m_zFar);
}
