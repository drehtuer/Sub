#ifndef SUBSURFACESCATTER_DXLIB_MENU_H
#define SUBSURFACESCATTER_DXLIB_MENU_H

#include "DXLib/Utilities.h"
#include <Windows.h>
#include <D3D11.h>
#include "AntTweakBar.h"
#include <D3DX10math.h>
#include <vector>
#include <string>
#include "DXLib/Structs.h"

enum {
	MODEL_CALLBACK,
	CAMERA_CALLBACK,
	LIGHT_CALLBACK,
	TEXTURE_CALLBACK,
	ALGORITHM_CALLBACK,
	SCREENSHOT_CALLBACK,
};

namespace SubSurfaceScatter {
	
	class DLLE Menu {
	public:
		Menu();
		~Menu();
		void init(ID3D11Device *Device, int width, int height);
		void draw();
		void setSize(int width, int height);
		static void TW_CALL handleError(const char *message);
		static Menu *getInstance();

		MenuParams *getParams();

	private:
		void initMenuParams();
		void updateMenuParamsCamera(const UINT i);
		void updateMenuParamsLight(const UINT i);
		void updateMenuParamsModel(const UINT i);

		static void TW_CALL setCB(const void *value, void *clientData);
		static void TW_CALL getCB(void *value, void *clientData);

		std::vector<UINT> m_callbackID;
		MenuParams m_MenuParams;
		TwBar *m_TwSceneBar, *m_TwShaderBar, *m_TwModelBar, *m_TwCameraBar, *m_TwLightBar;
		float m_FPS, m_D3DVersion, m_msecs;
		__int64 m_lastTick, m_countsPerSeconds;
		static Menu *m_Singleton;
	};
}

#endif
