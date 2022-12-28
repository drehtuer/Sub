#ifndef SUBSURFACESCATTER_STRUCTS_H
#define SUBSURFACESCATTER_STRUCTS_H

#include "DXLib/Utilities.h"
#include <D3DX10math.h>

namespace SubSurfaceScatter {

    // input layouts
    struct Point3dVNT {
        D3DXVECTOR3 position;
        D3DXVECTOR3 normal;
        D3DXVECTOR2 texCoord;
    };

	// RWBufferStructs
	struct BufferColorRGBA {
		float RGBA[4];
	};

    // constant buffers
    struct cbPerCamera {
        // world space -> camera space
        D3DXMATRIX world2Camera;      // c0
        // world space -> camera space (inverse transposed)
        D3DXMATRIX world2Camera_IT;   // c4
        // camera space -> world space
        D3DXMATRIX camera2World;      // c8
        // world space -> camera space -> camera proj
        D3DXMATRIX world2CameraProj;       // c12
        // camera proj -> camera space -> world space
        D3DXMATRIX cameraProj2World;       // c16
        float cameraZNear;            // c20.x
        float cameraZFar;             // c20.y
        float padding[2];             // c20.z
    };

    struct cbPerLight {
        // world space -> light space
        D3DXMATRIX  world2Light[NUMLIGHTSOURCES];      // c0
        // world space -> light space -> light proj
        D3DXMATRIX  world2LightProj[NUMLIGHTSOURCES];  // c4
        D3DXMATRIX  light2World[NUMLIGHTSOURCES];      // c8
        D3DXVECTOR4 attenuation[NUMLIGHTSOURCES];      // c12
        D3DXVECTOR4 color[NUMLIGHTSOURCES];            // c13  // only xyz used
        D3DXVECTOR4 zNearFar[NUMLIGHTSOURCES];         // c14 // only xy used
    };

    struct cbPerModel {
        // object space -> world space
        D3DXMATRIX object2World;       // c0
        // object space -> world space (inverse transposed)
        D3DXMATRIX object2World_IT;    // c4
        // object space -> world space -> camera space -> camera proj
        D3DXMATRIX object2CameraProj;  // c8
    };

    struct cbShaderSettings {
        UINT textureSelected;  // c0.x
        float stretchAlpha;    // c0.y
        float stretchBeta;     // c0.z
        float blurMix;         // c0.w
        bool bloomFilter;      // c1.x
        float fresnelReflectance; // c1.y
        D3DXVECTOR2 shadowMapSize; // c1.z
        float shadowMapEpsilon; // c2.x
        float transmittanceScale; // c2.y
        D3DXVECTOR2 sceneSize; // c2.z
        float gammaCorrection; // c3.x
		float minBloom; // c3.y
		float maxBloom; // c3.z
        float transmittanceBlur1; // c3.w
        float transmittanceBlur2; // c4.x
		float transmittanceFNL; // c4.y
		float depthEpplingEpsilon; // c4.z
        float shaderPadding; // c4.w
    };

    // misc
    struct ShaderMacro {
        std::string Name;
        std::string Definition;
    };


    // menu structs
    struct DLLE ModelMenuParams {
        std::string Name;
        D3DXQUATERNION RotationSelf;
        D3DXVECTOR3 Translation;
        D3DXQUATERNION RotationGlobal;
    };

    struct DLLE CameraMenuParams {
        std::string Name;
        D3DXQUATERNION RotationSelf;
        D3DXVECTOR3 Translation;
        D3DXQUATERNION RotationGlobal;
        float fovy;
        float zNear;
        float zFar;
    };

    struct DLLE LightMenuParams {
        std::string Name;
        D3DXQUATERNION RotationSelf;
        D3DXVECTOR3 Translation;
        D3DXQUATERNION RotationGlobal;
        D3DXVECTOR3 Color;
        bool enableLight, showLight;
        float intensity, a0, a1, a2;
        float fovy;
        float zNear;
        float zFar;
    };

    struct DLLE ShaderMenuParams {
        std::vector<std::string> TextureNames;
        UINT textureSelected;
        float stretchAlpha;
        float stretchBeta;
        float blurMix;
        bool bloomFilter;
        float fresnelReflectance;
        D3DXVECTOR2 ShadowMapSize;
        float shadowMapEpsilon;
        float transmittanceScale;
        float gammaCorrection;
		float minBloom;
		float maxBloom;
        float transmittanceBlur1;
        float transmittanceBlur2;
		float transmittanceFNL;
		float depthPeelingEpsilon;
    };

    struct DLLE SceneMenuParams {
        D3DXVECTOR4 BackgroundColor;
        D3DXVECTOR2 RenderSize;
        bool drawSkin;
        bool drawNonSkin;
        float lightSphereSize;
		UINT algorithm;
		bool showMenu;
    };

    struct DLLE MenuParams {
        std::vector<ModelMenuParams> Models;
        UINT modelSelected;
        
        std::vector<CameraMenuParams> Cameras;
        UINT cameraSelected;

        std::vector<LightMenuParams> Lights;
        UINT lightSelected;

        ShaderMenuParams ShaderSettings;

        SceneMenuParams SceneSettings;
    };

	struct DLLE ShaderStruct {
		ShaderStruct()
		  : Filename(L""),
			EntryPoint(""),
	        Profile(""),
	       lastModified(0),
	       lastHR(S_OK) {};
		virtual ~ShaderStruct() {};
		std::wstring Filename;
		std::string EntryPoint;
		std::string Profile;
		std::time_t lastModified;
		std::vector<ShaderMacro> Macros;
		HRESULT lastHR;
	};

	struct DLLE ShaderVS : ShaderStruct {
		ShaderVS() : ShaderStruct(), VS(NULL) {};
		virtual ~ShaderVS() { safe_delete(VS); };
		ID3D11VertexShader *VS;
	};

	struct DLLE ShaderGS : ShaderStruct {
		ShaderGS() : ShaderStruct(), GS(NULL) {};
		virtual ~ShaderGS() { safe_delete(GS); };
		ID3D11GeometryShader *GS;
	};

	struct DLLE ShaderPS : ShaderStruct {
		ShaderPS() : ShaderStruct(), PS(NULL) {};
		virtual ~ShaderPS() { safe_delete(PS); };
		ID3D11PixelShader *PS;
	};

	struct DLLE ShaderCS : ShaderStruct {
		ShaderCS() : ShaderStruct(), CS(NULL) {};
		virtual ~ShaderCS() { safe_delete(CS); };
		ID3D11ComputeShader *CS;
	};

};

#endif
