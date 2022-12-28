#include "DXLib/QuadManager.h"

using namespace SubSurfaceScatter;

QuadManager *QuadManager::m_Singleton = NULL;

QuadManager::~QuadManager() {
	safe_delete_map(m_Quads);
}

QuadManager *QuadManager::getInstance() {
	if(!m_Singleton)
		m_Singleton = new QuadManager();
	return m_Singleton;
}

void QuadManager::add(const std::string &Name, RenderToTexture *RTT) {
	m_Quads[Name] = RTT;
}

RenderToTexture *QuadManager::get(const std::string &Name) {
	return m_Quads[Name];
}
