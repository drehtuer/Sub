#ifndef SUBSURFACESCATTER_UTILITIES_H
#define SUBSURFACESCATTER_UTILITIES_H

#define UNICODE
#define DLLE __declspec(dllexport)

#define NOMINMAX
#undef min
#undef max

#define HR( x ) hr = x;\
	if(FAILED(hr))\
	return hr

#define chkHr( x ) hr = x;\
	if(FAILED(hr)) {\
	DXTrace(__FILE__, __LINE__, hr, DXGetErrorDescription(hr), true);\
	}

#define ZEROMEM(x) ZeroMemory(&x, sizeof(x))

enum ModelIDs {
	MODEL_LEEPERRYSMITH,
	MODEL_MAKEHUMAN,
	MODEL_TESTCASE,
};

// settings
#define USEHDR                    0
#define TRANSLUCENCYLIGHT         0
#define NUMLIGHTSOURCES           5
#define USEVARIANCESHADOWMAP      1
// more than 3 doesn't make sense since we are looking for occluders
// passing 3 layer -> there must be something in the way
#define NUMDEPTHPEELING           3
#define NUMBLURPASSES             6
#define TRANSMITTANCETEXSIZE      1024
#define TEXTUREWIDTH              1024
#define TEXTUREHEIGHT             1024
#define SHADOWMAPWIDTH            512
#define SHADOWMAPHEIGHT           512
#if USEHDR
	#define COLORBUFFERFORMAT     DXGI_FORMAT_R32G32B32A32_FLOAT
	#define IMAGEBUFFERFORMAT     DXGI_FORMAT_R32G32B32A32_FLOAT
#else
	#define COLORBUFFERFORMAT     DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	#define IMAGEBUFFERFORMAT     DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
#endif
#define HIGHRESBUFFERFORMAT   DXGI_FORMAT_R32G32B32A32_FLOAT
#define LINEARBUFFERFORMAT    DXGI_FORMAT_R8G8B8A8_UNORM
#if USEVARIANCESHADOWMAP
	#define DEPTHONLYBUFFERFORMAT DXGI_FORMAT_R32G32B32A32_FLOAT
#else
	#define DEPTHONLYBUFFERFORMAT DXGI_FORMAT_R32_FLOAT
#endif
#define MULTISAMPLINGLEVEL        1
#define MULTISAMPLINGQUALITY      0

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <map>

#include <boost/math/constants/constants.hpp>

#include <Windows.h>
#include <DxErr.h>
#include <vector>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10math.h>

namespace SubSurfaceScatter {

	namespace bmc = boost::math::constants;

	template <typename OBJ> void safe_delete(OBJ &Obj) {
		if(Obj) {
			// is Obj a windows com object?
			IUnknown *Com = dynamic_cast<IUnknown*>(Obj);
			if(Com) {
				Com->Release();
			}
			else {
				// normal c++ object
				delete Obj;
				Obj = NULL;
			}
		}
	};

	template <typename OBJ> void safe_delete_array(OBJ &Obj) {
		if(Obj) {
			delete [] Obj;
			Obj = NULL;
		}
	};

	template<class Type> inline void d3dLiveName(Type t, std::string name) {
#if defined(_DEBUG) || defined(DEBUG)
		if(t)
			t->SetPrivateData(WKPDID_D3DDebugObjectName, (int)name.size(), name.c_str());
#endif
	};

	template <typename OBJ> void safe_release_array(OBJ &Obj, const UINT size) {
		if(Obj) {
			for(UINT i=0; i<size; ++i)
				safe_delete(Obj[i]);
			// Obj is a normal c++ array
			safe_delete_array(Obj);
			Obj = NULL;
		}
	};

	template <typename OBJ> void safe_delete_vector(std::vector<OBJ> &V) {
		std::vector<OBJ>::iterator itr;
		for(itr=V.begin(); itr != V.end(); ++itr)
			safe_delete(*itr);
		V.clear();
	};

	template <typename OBJ> void safe_delete_map(std::map<std::string, OBJ> &M) {
		std::map<std::string, OBJ>::iterator itr;
		for(itr=M.begin(); itr != M.end(); ++itr)
			safe_delete(itr->second);
		M.clear();
	};

	template <class Type> std::string num2str(const Type t, const int precision = 12) {
		std::stringstream ws;
		ws << std::setprecision(precision) << t;
		return ws.str();
	};

	template <typename T> T str2num(const std::string &string) {
		return boost::lexical_cast<T>(string);
	};

	template <class Type> std::wstring num2wstr(Type t, const int precision = 12) {
		std::wstringstream ws;
		ws << std::setprecision(precision) << t;
		return ws.str();
	};

	template <class Type> Type deg2rad(Type deg) {
		return deg * bmc::pi<Type>() / 180;
	};

	template <class Type> Type rad2deg(Type rad) {
		return rad * 180 / bmc::pi<Type>();
	};

	inline void show_error(std::wstring message) {
		MessageBox(NULL, message.c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);
	};

	inline std::wstring str2wstr(const std::string &str) {
		std::wstring wstr(str.begin(), str.end()); // not safe, but it works as long as you use only ascii characters
		return wstr;
	};

	inline std::string wstr2str(const std::wstring &wstr) {
		std::string str(wstr.begin(), wstr.end()); // not safe, but it works as long as you use only ascii characters
		return str;
	};

	inline std::string bool2str(const bool b) {
		if(b)
			return "true";
		else
			return "false";
	};

	inline void show_error(const std::string &message) {
		show_error(str2wstr(message));
	};

	inline HRESULT chkError(HRESULT hr) {
		LPTSTR errorText = NULL;
		if(FAILED(hr)) {
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&errorText,
				0,
				NULL);
			if(errorText) {
				show_error(errorText);
				LocalFree(errorText);
				errorText = NULL;
			}
		}
		return hr;
	};

	class _Logger {
	public:
		template <typename T> _Logger &operator<<(const T &right) {
			std::cout << right << std::endl;
			return *this;
		};
	};
	static _Logger LOGG;

	inline std::wstring getErrorMessage() {
		DWORD lastError = GetLastError();
		LPWSTR errorMsg = NULL;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&errorMsg,
			0,
			NULL
			);
		return std::wstring(errorMsg);
	};
}

#endif
