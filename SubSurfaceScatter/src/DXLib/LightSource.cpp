#include "DXLib/LightSource.h"

using namespace SubSurfaceScatter;

LightSource::LightSource()
	: m_color(D3DXVECTOR3(1.0f, 1.0f, 1.0f)),
	  m_attenuation(D3DXVECTOR4(100.0f, 0.0f, 0.0f, 1.0f)),
	   m_fovy(deg2rad(45.0f)),
	   m_aspect(4/3),
	   m_zNear(200),
	   m_zFar(1000)
{
	D3DXMatrixIdentity(&m_projectionMatrix);
}

D3DXVECTOR3 LightSource::getColor() const {
	return m_color;
}

D3DXVECTOR4 LightSource::getAttenuation() const {
	return m_attenuation;
}

bool LightSource::setColor(const D3DXVECTOR3 color) {
	if(color != m_color) {
		m_color = color;
		return true;
	} else
		return false;
}

bool LightSource::setFovy(const float fovy) {
	float rfovy = deg2rad(fovy);
	if(rfovy != m_fovy) {
		m_fovy = rfovy;
		return true;
	} else
		return false;
}

bool LightSource::setAttenuation(const D3DXVECTOR4 attenuation) {
	if(attenuation != m_attenuation) {
		m_attenuation = attenuation;
		return true;
	} else
		return false;
}

bool LightSource::setAspectRatio(const float aspect) {
	if(aspect != m_aspect) {
		m_aspect = aspect;
		return true;
	} else
		return false;
}

bool LightSource::setParameters(const float fovy, const float aspect, const float zNear, const float zFar) {
	if(setFovy(fovy) || setAspectRatio(aspect) || setZNear(zNear) || setZFar(zFar))
		return true;
	else
		return false;
}

float LightSource::getZNear() const {
	return m_zNear;
}

float LightSource::getZFar() const {
	return m_zFar;
}

bool LightSource::setZNear(const float zNear) {
	if(zNear != m_zNear) {
		m_zNear = zNear;
		return true;
	} else
		return false;
}

bool LightSource::setZFar(const float zFar) {
	if(zFar != m_zFar) {
		m_zFar = zFar;
		return true;
	} else
		return false;
}

D3DXMATRIX LightSource::getWorld2LightProjMatrix() const {
	D3DXMATRIX M;
	D3DXMatrixMultiply(&M, &getWorld2ObjectMatrix(), &m_projectionMatrix);
	return M;
}

void LightSource::updateMatrices() {
	MoveableObject::updateMatrices();
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, m_fovy, m_aspect, m_zNear, m_zFar);
}
