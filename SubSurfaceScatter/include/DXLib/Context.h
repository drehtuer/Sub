#ifndef SUBSURFACESCATTER_CONTEXT_H
#define SUBSURFACESCATTER_CONTEXT_H

#include "DXLib/Utilities.h"

#include <Windows.h>

#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10math.h>

#include <vector>
#include <map>
#include <string>

#include "DXLib/BasicWindow.h"
#include "DXLib/Structs.h"
#include "DXLib/Camera.h"
#include "DXLib/ShaderManager.h"
#include "DXLib/LightSource.h"
#include "DXLib/RenderToTexture.h"
#include "DXLib/Menu.h"
#include "DXLib/Quad.h"
#include "DXLib/Sphere.h"
#include "DXLib/TextureManager.h"
#include "DXLib/VirtualObject.h"
#include "DXLib/QuadManager.h"

namespace SubSurfaceScatter {
    class DLLE Context : public BasicWindow {
    public:
        Context(HINSTANCE hInstance);
        virtual ~Context();
        virtual HRESULT init();
		HRESULT saveCurrentImage(const std::string &Filename);

    protected:
        virtual HRESULT initD3D();
        virtual HRESULT resize();

        // called by BasicWindow.run()
        virtual void doWork();
        virtual void clear();
        virtual void render();
        virtual void swapBuffers();

        virtual void release();

        virtual void updateCameras();
        virtual void updateLights();
        virtual void updateModels();
        virtual void updateShaderSettings();

        virtual void updateModelMesh();

        void resetOMTargetsAndViewport();

    private:
        void resizeQuads();
        void unbindQuads();
        void initShaders();
        void initTextures();
        void initStencils();
        void initQuads();
        void prerenderTextures();
        bool shaderMenuUpdated(const ShaderMenuParams &SHMP, const SceneMenuParams &SCMP);

		// NoSSS
		void TS_geometryPass();
		void NoSSS_renderPass();

		// Brand
        void Brand_shadowPeelingPass();
		void Brand_dilateShadowNormalsPass();
		void Brand_lightPass();
        void Brand_transmittanceBlurPass();
		void Brand_multiblurPass();
        void Brand_combinePass();
        
		// Jimenez
		void Jimenez_lightPass();
		void Jimenez_multiblurPass();
		void Jimenez_combinePass();

		// d'Eon
		void dEon_irradiancePass();
		void dEon_dilateIrradiancePass();
		void dEon_shadowPass();
		void dEon_textureStretchPass();
		void dEon_multiblurPass();
		void dEon_combinePass();

		// Hable
		void Hable_blurPass();
		void Hable_combinePass();

		// generic
		void shadowPass();
		void geometryPass();
		void bloomPass();
		void displayQuadPass();


		std::string updateSceneSettings(const UINT index, const UINT counter, const UINT algo);
		

        ID3D11Device *m_Device;
        ID3D11DeviceContext *m_Context;
        IDXGISwapChain *m_d3dSwapChain;
        ID3D11RenderTargetView *m_d3dRenderTargetView;
        ID3D11DepthStencilView *m_d3dDepthStencilView;
        D3D_FEATURE_LEVEL m_d3dFeatureLevel;

        std::map<std::string, ID3D11Buffer*> m_d3dBuffers;
        std::map<std::string, ID3D11DepthStencilState*> m_DSSs;
        std::map<std::string, ID3D11RasterizerState*> m_Rasterizers;
        std::map<UINT, float> m_TransmittanceScales;
        Quad m_Quad;
		QuadManager *m_Quads;
        std::vector<LightSource*> m_LightSources;
        std::vector<Sphere*> m_LightSpheres;
        VirtualObject *m_ModelObject;
        Camera m_Camera;
        Menu *m_Menu;
        ShaderManager *m_ShaderManager;
        TextureManager *m_TextureManager;

        bool m_isEnabled, m_lightsUpdated, m_modelUpdate, m_cameraUpdate, m_windowSizeUpdate, m_shaderMenuUpdate, m_algorithmUpdate;
        ShaderMenuParams m_SHMP_old;
        SceneMenuParams m_SCMP_old;
		UINT m_algorithm_old;
        int m_SaveImageIndex, m_ImageCounter, m_algo;
    };
};

#endif
