#ifndef SUBSURFACESCATTER_InfiniteScan_H
#define SUBSURFACESCATTER_InfiniteScan_H

#include "DXLib/Utilities.h"
#include "DXLib/VirtualObject.h"
#include <string>

// model is from http://www.ir-ltd.net/infinite-3d-head-scan-released

namespace SubSurfaceScatter {
	class DLLE InfiniteScan : public VirtualObject {
	public:
		HRESULT loadModel();
		void drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
		void drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
	};
};

#endif
