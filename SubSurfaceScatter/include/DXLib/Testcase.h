#ifndef SUBSURFACESCATTER_TESTCASE_H
#define SUBSURFACESCATTER_TESTCASE_H

#include "DXLib/Utilities.h"
#include "DXLib/VirtualObject.h"
#include <string>
#include "DXLib/ShaderManager.h"
#include "DXLib/TextureManager.h"

namespace SubSurfaceScatter {
	class DLLE Testcase : public VirtualObject {
	public:
		HRESULT loadModel();
		void drawSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
		void drawNonSkin(const std::string &VSName, const std::string &GSName, const std::string &PSName, const std::string &CSName);
	};
};

#endif
