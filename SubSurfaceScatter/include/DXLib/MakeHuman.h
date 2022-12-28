#ifndef SUBSURFACESCATTER_MAKEHUMAN_H
#define SUBSURFACESCATTER_MAKEHUMAN_H

#include "DXLib/Utilities.h"
#include "DXLib/VirtualObject.h"
#include <string>


namespace SubSurfaceScatter {
	class DLLE MakeHuman : public VirtualObject {
	public:
		HRESULT loadModel();
		void drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
		void drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
	};
};

#endif
