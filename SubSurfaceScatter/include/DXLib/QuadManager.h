#ifndef SUBSURFACESCATTER_QUADMANAGER_H
#define SUBSURFACESCATTER_QUADMANAGER_H

#include "DXLib/Utilities.h"
#include <map>
#include <string>
#include "DXLib/RenderToTexture.h"

namespace SubSurfaceScatter {
	class DLLE QuadManager {
	public:
		virtual ~QuadManager();
		static QuadManager *getInstance();
		void add(const std::string &Name, RenderToTexture *RTT);
		RenderToTexture *get(const std::string &Name);

	private:
		static QuadManager *m_Singleton;
		std::map<std::string, RenderToTexture*> m_Quads;
	};
}

#endif
