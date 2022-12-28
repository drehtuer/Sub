#include "DXLib/Menu.h"
#include "DXLib/TextureManager.h"

using namespace SubSurfaceScatter;

#define STARTMODEL 0
#define STARTCAMERA 0
#define STARTALGO 0
#define STARTLIGHT 0
#define LIGHT0 true
#define LIGHT1 false
#define LIGHT2 false
#define LIGHT3 false
#define LIGHT4 false
#define GAMMA 2.2f

Menu *Menu::m_Singleton = NULL;

Menu::Menu() 
	: m_TwSceneBar(NULL),
	  m_TwShaderBar(NULL),
	  m_TwModelBar(NULL),
	  m_TwCameraBar(NULL),
	  m_TwLightBar(NULL),
	  m_FPS(0.0f),
      m_msecs(0.0f),
	  m_D3DVersion(0.0f),
	  m_lastTick(0),
	  m_countsPerSeconds(0)
{
	m_callbackID.push_back(MODEL_CALLBACK);
	m_callbackID.push_back(CAMERA_CALLBACK);
	m_callbackID.push_back(LIGHT_CALLBACK);
	m_callbackID.push_back(TEXTURE_CALLBACK);
	m_callbackID.push_back(ALGORITHM_CALLBACK);
	if(!m_Singleton)
		m_Singleton = this;
	else
		LOGG << "Menu class exists already";

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_countsPerSeconds);
}

void Menu::initMenuParams() {
	// models
	ModelMenuParams MMP;
	MMP.Name = "Lee Perry-Smith";
	MMP.RotationSelf = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	MMP.Translation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	MMP.RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	m_MenuParams.Models.push_back(MMP);

	MMP.Name = "MakeHuman";
	MMP.RotationSelf = D3DXQUATERNION(-0.71f, 0.0f, 0.0f, -0.7f);
	MMP.Translation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	MMP.RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	m_MenuParams.Models.push_back(MMP);

	MMP.Name = "Test case";
	MMP.RotationSelf = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	MMP.Translation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	MMP.RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	m_MenuParams.Models.push_back(MMP);

	m_MenuParams.modelSelected = STARTMODEL;
	

	// cameras
	CameraMenuParams CMP;
	CMP.Name = "Head frontal";
	CMP.RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
	CMP.Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
	CMP.RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	CMP.fovy = 42.0f;
	CMP.zNear = 200;
	CMP.zFar = 1200;
	m_MenuParams.Cameras.push_back(CMP);

	CMP.Name = "MakeHuman frontal";
	CMP.RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
	CMP.Translation = D3DXVECTOR3(0.0f, 0.0f, 50.0f);
	CMP.RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
	CMP.fovy = 30.0f;
	CMP.zNear = 1;
	CMP.zFar = 600;
	m_MenuParams.Cameras.push_back(CMP);

	CMP.Name = "MakeHuman right hand";
	CMP.RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
	CMP.Translation = D3DXVECTOR3(-7.74f, 0.45f, 10.15f);
	CMP.RotationGlobal = D3DXQUATERNION(0.72f, 0.0f, 0.0f, -0.7f);
	CMP.fovy = 30.0f;
	CMP.zNear = 1;
	CMP.zFar = 120;
	m_MenuParams.Cameras.push_back(CMP);

	m_MenuParams.cameraSelected = STARTLIGHT;


	// lights
	for(UINT i=0; i<NUMLIGHTSOURCES; ++i) {
		LightMenuParams LMP;
		LMP.Name = "PointLight"+num2str(i);
		LMP.RotationSelf = D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
		LMP.Translation = D3DXVECTOR3(0.0f, 0.0f, 500.0f);
		LMP.RotationGlobal = D3DXQUATERNION(1.0f, 0.0f, 0.0f, 0.0f);
		LMP.Color = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		LMP.showLight = LIGHT0;
		LMP.intensity = 120000.0f;
		LMP.a0 = 0.0f;
		LMP.a1 = 0.0f;
		LMP.a2 = 1.0f;
		LMP.fovy = 42.0f;
		LMP.zNear = 200;
		LMP.zFar = 1200;
		m_MenuParams.Lights.push_back(LMP);
	}
    m_MenuParams.Lights[0].Name = "Front";
	if(NUMLIGHTSOURCES > 1) {
		m_MenuParams.Lights[1].showLight = LIGHT1;
		m_MenuParams.Lights[1].RotationGlobal = D3DXQUATERNION(0.0f, 0.0f, -1.0f, 0.0f);
        m_MenuParams.Lights[1].Name = "Back";
	}
	if(NUMLIGHTSOURCES > 2) {
		m_MenuParams.Lights[2].showLight = LIGHT2;
		m_MenuParams.Lights[2].RotationGlobal = D3DXQUATERNION(0.70f, 0.0f, -0.71f, 0.0f);
        m_MenuParams.Lights[2].Name = "Left";
	}
	if(NUMLIGHTSOURCES > 3) {
		m_MenuParams.Lights[3].showLight = LIGHT3;
		m_MenuParams.Lights[3].RotationGlobal = D3DXQUATERNION(0.70f, 0.0f, 0.71f, 0.0f);
        m_MenuParams.Lights[3].Name = "Right";
	}
	if(NUMLIGHTSOURCES > 4) {
		m_MenuParams.Lights[4].showLight = LIGHT4;
		m_MenuParams.Lights[4].RotationGlobal = D3DXQUATERNION(0.70f, 0.0f, 0.0f, -0.72f);
        m_MenuParams.Lights[4].Name = "Top";
	}
	m_MenuParams.lightSelected = 0;

	// shader settings
	TextureManager *T = TextureManager::getInstance();
	m_MenuParams.ShaderSettings.TextureNames = T->getTextureNames();
	std::vector<std::string> UAVs = T->getUAVNames();
	for(UINT i=0; i<UAVs.size(); ++i)
		m_MenuParams.ShaderSettings.TextureNames.push_back(UAVs[i]);
	
	// start with diffuse map
	std::string startTexture = "PPCombineMap";
	m_MenuParams.ShaderSettings.textureSelected = (UINT)std::distance(m_MenuParams.ShaderSettings.TextureNames.begin(), std::find(m_MenuParams.ShaderSettings.TextureNames.begin(), m_MenuParams.ShaderSettings.TextureNames.end(), startTexture));
	m_MenuParams.ShaderSettings.stretchAlpha = 11;
	m_MenuParams.ShaderSettings.stretchBeta = 900;
	m_MenuParams.ShaderSettings.blurMix = 0.5f;
	m_MenuParams.ShaderSettings.bloomFilter = true;
	m_MenuParams.ShaderSettings.fresnelReflectance = 0.028f;
	m_MenuParams.ShaderSettings.ShadowMapSize = D3DXVECTOR2(SHADOWMAPWIDTH, SHADOWMAPHEIGHT);
	m_MenuParams.ShaderSettings.shadowMapEpsilon = 0.005f;
	m_MenuParams.ShaderSettings.transmittanceScale = 100.0f;
	m_MenuParams.ShaderSettings.gammaCorrection = GAMMA;
	m_MenuParams.ShaderSettings.minBloom = 0.8f;
	m_MenuParams.ShaderSettings.maxBloom = 1.0f;
    m_MenuParams.ShaderSettings.transmittanceBlur1 = 20.0f;
    m_MenuParams.ShaderSettings.transmittanceBlur2 = 30.0f;
	m_MenuParams.ShaderSettings.transmittanceFNL = 0.25f;
	m_MenuParams.ShaderSettings.depthPeelingEpsilon = 0.002f;

	// scene settings
	m_MenuParams.SceneSettings.BackgroundColor = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	m_MenuParams.SceneSettings.RenderSize = D3DXVECTOR2(TEXTUREWIDTH, TEXTUREHEIGHT);
	m_MenuParams.SceneSettings.drawSkin = true;
	m_MenuParams.SceneSettings.drawNonSkin = true;
	m_MenuParams.SceneSettings.lightSphereSize = 10.0f;
	m_MenuParams.SceneSettings.algorithm = STARTALGO;
	m_MenuParams.SceneSettings.showMenu = true;
}

Menu::~Menu() {
	TwTerminate();
}

Menu* Menu::getInstance() {
	if(!m_Singleton)
		m_Singleton = new Menu();
	return m_Singleton;
}

void TW_CALL Menu::handleError(const char *message) {
	if(message)
		LOGG << "AntTweakBar error: " + std::string(message);
}

void Menu::init(ID3D11Device *Device, int width, int height) {
	if(!TwInit(TW_DIRECT3D11, Device))
		show_error("Could not initialize AntTweakBar: " + std::string(TwGetLastError()));
	TwHandleErrors(handleError);

	LOGG << "\t\tUsing AntTweakBar version: "+num2str(TW_VERSION);

	m_TwSceneBar = TwNewBar("SceneMenu");
	m_TwShaderBar = TwNewBar("ShaderMenu");
	m_TwModelBar = TwNewBar("ModelMenu");
	m_TwCameraBar = TwNewBar("CameraMenu");
	m_TwLightBar = TwNewBar("LightMenu");
	initMenuParams();

	switch(Device->GetFeatureLevel()) {
	case D3D_FEATURE_LEVEL_10_0:
		m_D3DVersion = 10.0f;
		break;
	case D3D_FEATURE_LEVEL_10_1:
		m_D3DVersion = 10.1f;
		break;
	case D3D_FEATURE_LEVEL_11_0:
		m_D3DVersion = 11.0f;
		break;
	default:
		m_D3DVersion = 0.0f;
		break;
	}

	// infos
	TwAddVarRO(m_TwSceneBar, "InfoD3DVersion", TW_TYPE_FLOAT, &m_D3DVersion, "label='DirectX version' precision=1 group=Info");
	TwAddVarRO(m_TwSceneBar, "InfoFPS", TW_TYPE_FLOAT, &m_FPS, "label=FPS precision=1 group=Info");
    TwAddVarRO(m_TwSceneBar, "InfoMSec", TW_TYPE_FLOAT, &m_msecs, "label=ms/Frame precision=2 group=Info");
	
	// scene
	TwAddVarRW(m_TwSceneBar, "SceneClearColor", TW_TYPE_COLOR4F, &m_MenuParams.SceneSettings.BackgroundColor, "label='Clear color' group=Scene");
	TwAddVarRW(m_TwSceneBar, "SceneWidth", TW_TYPE_FLOAT, &m_MenuParams.SceneSettings.RenderSize.x, "min=256 step=1 label='Scene width' group=Scene");
	TwAddVarRW(m_TwSceneBar, "SceneHeight", TW_TYPE_FLOAT, &m_MenuParams.SceneSettings.RenderSize.y, "min=256 step=1 label='Scene height' group=Scene");
	TwAddVarRW(m_TwSceneBar, "SceneDrawSkin", TW_TYPE_BOOLCPP, &m_MenuParams.SceneSettings.drawSkin, "label='Draw skin' group=Scene");
	TwAddVarRW(m_TwSceneBar, "SceneDrawNonSkin", TW_TYPE_BOOLCPP, &m_MenuParams.SceneSettings.drawNonSkin, "label='Draw non skin' group=Scene");
	TwAddVarRW(m_TwSceneBar, "SceneLightSphereSize", TW_TYPE_FLOAT, &m_MenuParams.SceneSettings.lightSphereSize, "label='Light sphere size' group=Scene");

	UINT numAlgorithms = 5;
	TwEnumVal *AlgorithmEnumVal = new TwEnumVal[numAlgorithms];
	UINT i = 0;
	AlgorithmEnumVal[i].Label = "No SSS";
	AlgorithmEnumVal[i].Value = i++;
	AlgorithmEnumVal[i].Label = "d'Eon";
	AlgorithmEnumVal[i].Value = i++;
	AlgorithmEnumVal[i].Label = "Hable";
	AlgorithmEnumVal[i].Value = i++;
	AlgorithmEnumVal[i].Label = "Jimenez";
	AlgorithmEnumVal[i].Value = i++;
	AlgorithmEnumVal[i].Label = "Brand";
	AlgorithmEnumVal[i].Value = i++;
	
	TwType AlgorithmType = TwDefineEnum("AlgorithmType", AlgorithmEnumVal, numAlgorithms);

	TwAddVarCB(m_TwSceneBar, "AlgorithmName", AlgorithmType, Menu::setCB, Menu::getCB, (void*)&m_callbackID[ALGORITHM_CALLBACK], "label='Algorithm' group=Scene");


	// models
	TwEnumVal *ModelEnumVal = new TwEnumVal[m_MenuParams.Models.size()];
	for(UINT i=0; i<(UINT)m_MenuParams.Models.size(); ++i) {
		ModelEnumVal[i].Label = m_MenuParams.Models[i].Name.c_str();
		ModelEnumVal[i].Value = i;
	}
	TwType ModelType = TwDefineEnum("ModelType", ModelEnumVal, (UINT)m_MenuParams.Models.size());
	
	TwAddVarCB(m_TwModelBar, "ModelName", ModelType, Menu::setCB, Menu::getCB, (void*)&m_callbackID[MODEL_CALLBACK], "label=Model group=Model");
	TwAddVarRW(m_TwModelBar, "ModelRotationSelf", TW_TYPE_QUAT4F, m_MenuParams.Models[m_MenuParams.modelSelected].RotationSelf, "showval=true label='Rotation self' group=Model");
	TwAddVarRW(m_TwModelBar, "ModelTranslation", TW_TYPE_DIR3F, m_MenuParams.Models[m_MenuParams.modelSelected].Translation, "showval=true label='Translation' group=Model");
	TwAddVarRW(m_TwModelBar, "ModelRotationGlobal", TW_TYPE_QUAT4F, m_MenuParams.Models[m_MenuParams.modelSelected].RotationGlobal, "showval=true label='Rotation global' group=Model");
	safe_delete_array(ModelEnumVal);


	// cameras
	TwEnumVal *CameraEnumVal = new TwEnumVal[m_MenuParams.Cameras.size()];
	for(UINT i=0; i<(UINT)m_MenuParams.Cameras.size(); ++i) {
		CameraEnumVal[i].Label = m_MenuParams.Cameras[i].Name.c_str();
		CameraEnumVal[i].Value = i;
	}
	TwType CameraType = TwDefineEnum("CameraType", CameraEnumVal, (UINT)m_MenuParams.Cameras.size());
	TwAddVarCB(m_TwCameraBar, "CameraName", CameraType, Menu::setCB, Menu::getCB, (void*)&m_callbackID[CAMERA_CALLBACK], "group=Camera");
	TwAddVarRW(m_TwCameraBar, "CameraRotationSelf", TW_TYPE_QUAT4F, m_MenuParams.Cameras[m_MenuParams.cameraSelected].RotationSelf, "showval=true label='Rotation self' group=Camera");
	TwAddVarRW(m_TwCameraBar, "CameraTranslation", TW_TYPE_DIR3F, m_MenuParams.Cameras[m_MenuParams.cameraSelected].Translation, "showval=true label='Translation' group=Camera");
	TwAddVarRW(m_TwCameraBar, "CameraRotationGlobal", TW_TYPE_QUAT4F, m_MenuParams.Cameras[m_MenuParams.cameraSelected].RotationGlobal, "showval=true label='Rotation global' group=Camera");
	TwAddVarRW(m_TwCameraBar, "CameraFovy", TW_TYPE_FLOAT, &m_MenuParams.Cameras[m_MenuParams.cameraSelected].fovy, "min=0.1 max=180 step=0.1 label=Fovy group=Camera");
	TwAddVarRW(m_TwCameraBar, "CameraZNear", TW_TYPE_FLOAT, &m_MenuParams.Cameras[m_MenuParams.cameraSelected].zNear, "min=0 step=1 label=zNear group=Camera");
	TwAddVarRW(m_TwCameraBar, "CameraZFar", TW_TYPE_FLOAT, &m_MenuParams.Cameras[m_MenuParams.cameraSelected].zFar, "min=1 step=1 label=zFar group=Camera");
	safe_delete_array(CameraEnumVal);


	// lights
	TwEnumVal *LightEnumVal = new TwEnumVal[m_MenuParams.Lights.size()];
	for(UINT i=0; i<(UINT)m_MenuParams.Lights.size(); ++i) {
		LightEnumVal[i].Label = m_MenuParams.Lights[i].Name.c_str();
		LightEnumVal[i].Value = i;
	}
	TwType LightType = TwDefineEnum("LightType", LightEnumVal, (UINT)m_MenuParams.Lights.size());
	TwAddVarCB(m_TwLightBar, "LightName", LightType, Menu::setCB, Menu::getCB, (void*)&m_callbackID[LIGHT_CALLBACK], "group=Light");
	TwAddVarRW(m_TwLightBar, "LightEnable", TW_TYPE_BOOLCPP, &m_MenuParams.Lights[m_MenuParams.lightSelected].enableLight, "label='Enable light source' group=Light");
	TwAddVarRW(m_TwLightBar, "LightShow", TW_TYPE_BOOLCPP, &m_MenuParams.Lights[m_MenuParams.lightSelected].showLight, "label='Show light source' group=Light");
	TwAddVarRW(m_TwLightBar, "LightRotationSelf", TW_TYPE_QUAT4F, &m_MenuParams.Lights[m_MenuParams.lightSelected].RotationSelf, "showval=true label='Rotation self' group=Light");
	TwAddVarRW(m_TwLightBar, "LightTranslation", TW_TYPE_DIR3F, &m_MenuParams.Lights[m_MenuParams.lightSelected].Translation, "showval=true label='Translation' group=Light");
	TwAddVarRW(m_TwLightBar, "LightRotationGlobal", TW_TYPE_QUAT4F, &m_MenuParams.Lights[m_MenuParams.lightSelected].RotationGlobal, "showval=true label='Rotation global' group=Light");
	TwAddVarRW(m_TwLightBar, "LightColor", TW_TYPE_COLOR3F, &m_MenuParams.Lights[m_MenuParams.lightSelected].Color, "label=Color group=Light");
	TwAddVarRW(m_TwLightBar, "LightIntensity", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].intensity, "min=0 step=1 label=Intensity group=Attenuation");
	TwAddVarRW(m_TwLightBar, "LightA0", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].a0, "min=0 max=1 step=0.01 label=a0 group=Attenuation");
	TwAddVarRW(m_TwLightBar, "LightA1", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].a1, "min=0 max=1 step=0.01 label=a1 group=Attenuation");
	TwAddVarRW(m_TwLightBar, "LightA2", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].a2, "min=0 max=1 step=0.01 label=a2 group=Attenuation");
	TwAddVarRW(m_TwLightBar, "LightFovy", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].fovy, "min=0.1 max=359 step=0.1 label=Fovy group=Light");
	TwAddVarRW(m_TwLightBar, "LightZNear", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].zNear, "min=0 step=1 label=zNear group=Light");
	TwAddVarRW(m_TwLightBar, "LightZFar", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].zFar, "min=1 step=1 label=zFar group=Light");
	safe_delete_array(LightEnumVal);


	// shaders
	TwEnumVal *ShadersTexturesEnumVal = new TwEnumVal[m_MenuParams.ShaderSettings.TextureNames.size()];
	for(UINT i=0; i<(UINT)m_MenuParams.ShaderSettings.TextureNames.size(); ++i) {
		ShadersTexturesEnumVal[i].Label = m_MenuParams.ShaderSettings.TextureNames[i].c_str();
		ShadersTexturesEnumVal[i].Value = i;
	}
	TwType ShadersTextureType = TwDefineEnum("TextureNamesType", ShadersTexturesEnumVal, (UINT)m_MenuParams.ShaderSettings.TextureNames.size());
	TwAddVarCB(m_TwShaderBar, "ShowTexture", ShadersTextureType, Menu::setCB, Menu::getCB, (void*)&m_callbackID[TEXTURE_CALLBACK], "label='Show texture' group=Shaders");
	safe_delete_array(ShadersTexturesEnumVal);
	TwAddVarRW(m_TwShaderBar, "FresnelReflectance", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.fresnelReflectance, "min=0.001 max=1 step=0.001 label='Fresnel reflectance (F0)' group=Shaders");
	TwAddVarRW(m_TwShaderBar, "GammaCorrection", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.gammaCorrection, "label='Gamma' step=0.01 min=0 max=5 group=Shaders");
	TwAddVarRW(m_TwShaderBar, "StretchAlpha", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.stretchAlpha, "min=0 step=0.1 label='Stretch alpha' group=Blur");
	TwAddVarRW(m_TwShaderBar, "StretchBeta", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.stretchBeta, "min=0 step=1 label='Stretch beta' group=Blur");
	TwAddVarRW(m_TwShaderBar, "BlurMix", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.blurMix, "min=0 max=1 step=0.01 label='Blur mix' group=Blur");
	TwAddVarRW(m_TwShaderBar, "TransmittanceScale", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.transmittanceScale, "label='Scale' min=0.01 step=0.1 group=Transmittance");
    TwAddVarRW(m_TwShaderBar, "TransmittanceBlurWidth1", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.transmittanceBlur1, "label='Blur width1' step=0.1 min=0 group=Transmittance");
    TwAddVarRW(m_TwShaderBar, "TransmittanceBlurWidth2", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.transmittanceBlur2, "label='Blur width2' step=0.1 min=0 group=Transmittance");
	TwAddVarRW(m_TwShaderBar, "TransmittanceFrontsideNdotL", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.transmittanceFNL, "label='Frontside NdotL impact' step=0.01 min=0 max=1 group=Transmittance");
	TwAddVarRW(m_TwShaderBar, "ShadowMapWidth", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.ShadowMapSize.x, "label='Shadow map width' min=64 step=1 group=Shadowmap");
	TwAddVarRW(m_TwShaderBar, "ShadowMapHeight", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.ShadowMapSize.y, "label='Shadow map height' min=64 step=1 group=Shadowmap");
	TwAddVarRW(m_TwShaderBar, "ShadowMapEpsilon", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.shadowMapEpsilon, "label='Shadow map epsilon' step=0.0001 group=Shadowmap");
	TwAddVarRW(m_TwShaderBar, "DepthPeelingEpsilon", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.depthPeelingEpsilon, "label='Depth peeling epsilon' step=0.0001 min=0 max=1 group=Shadowmap");
	TwAddVarRW(m_TwShaderBar, "EnableBloom", TW_TYPE_BOOLCPP, &m_MenuParams.ShaderSettings.bloomFilter, "label='Enable bloom' group=Bloom");
	TwAddVarRW(m_TwShaderBar, "MinBloom", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.minBloom, "label='Min bloom' step=0.01 min=0 max=1 group=Bloom");
	TwAddVarRW(m_TwShaderBar, "MaxBloom", TW_TYPE_FLOAT, &m_MenuParams.ShaderSettings.maxBloom, "label='Max bloom' step=0.01 min=0 max=1 group=Bloom");

	TwDefine("LightMenu/Attenuation group=Light\n");
	TwDefine("LightMenu/Light group=Controls\n");
	TwDefine("ModelMenu/Model group=Controls\n");
	TwDefine("CameraMenu/Camera group=Controls\n");
	TwDefine("ShaderMenu/Blur group=Shaders\n");
	TwDefine("ShaderMenu/Bloom group=Shaders opened=false\n");
	TwDefine("ShaderMenu/Shadowmap group=Shaders\n");
	TwDefine("ShaderMenu/Transmittance group=Shaders\n");

	// resize called by context on initialization
}

void Menu::draw() {
	__int64 tick;
	QueryPerformanceCounter((LARGE_INTEGER*)&tick);
	
	float timeElapsed = (tick - m_lastTick) / (float)m_countsPerSeconds;
	m_lastTick = tick;
    m_msecs = 0.5f * (timeElapsed*1000 + m_msecs);
	m_FPS = 0.5f * (1.0f / ((float)timeElapsed + 0.0000001f) + m_FPS);

	TwDraw();
}

void Menu::setSize(int width, int height) {
	TwWindowSize(width, height);

	int positions[2], size[2], margin = 10;
	std::string BarParam;

	// top left
	BarParam = "SceneMenu label='Scene menu' position='"+num2str(margin)+" "+num2str(margin)+"'";
	TwDefine(BarParam.c_str());
	
	// left
	TwGetParam(m_TwSceneBar, NULL, "position", TW_PARAM_INT32, 2, positions);
	TwGetParam(m_TwSceneBar, NULL, "size", TW_PARAM_INT32, 2, size);
	BarParam = "ShaderMenu label='Shader menu' position='"+num2str(margin)+" "+num2str(positions[1] + size[1] + margin)+"'";
	TwDefine(BarParam.c_str());

	// left
	TwGetParam(m_TwShaderBar, NULL, "position", TW_PARAM_INT32, 2, positions);
	TwGetParam(m_TwShaderBar, NULL, "size", TW_PARAM_INT32, 2, size);
	BarParam = "ModelMenu label='Model menu' position='"+num2str(margin)+" "+num2str(positions[1] + size[1] + margin)+"'";
	TwDefine(BarParam.c_str());

	// top right
	TwGetParam(m_TwCameraBar, NULL, "size", TW_PARAM_INT32, 2, size);
	BarParam = "CameraMenu label='Camera menu' position='"+num2str(width - 1 - size[0] - 3*margin)+" "+num2str(margin)+"'";
	TwDefine(BarParam.c_str());

	// right
	TwGetParam(m_TwCameraBar, NULL, "position", TW_PARAM_INT32, 2, positions);
	TwGetParam(m_TwCameraBar, NULL, "size", TW_PARAM_INT32, 2, size);
	BarParam = "LightMenu label='Light menu' position='"+num2str(positions[0])+" "+num2str(positions[1] + size[1] + margin)+"'";
	TwDefine(BarParam.c_str());
}

MenuParams *Menu::getParams() {
	return &m_MenuParams;
}


void TW_CALL Menu::setCB(const void *value, void *clientData) {
	UINT caller = *static_cast<UINT*>(clientData);
	UINT msg = *static_cast<const UINT*>(value);
	Menu *M = Menu::getInstance();
	switch(caller) {
	case MODEL_CALLBACK:
		M->getParams()->modelSelected = msg;
		M->updateMenuParamsModel(M->getParams()->modelSelected);
		break;

	case CAMERA_CALLBACK:
		M->getParams()->cameraSelected = msg;
		M->updateMenuParamsCamera(M->getParams()->cameraSelected);
		break;

	case LIGHT_CALLBACK:
		M->getParams()->lightSelected = msg;
		M->updateMenuParamsLight(M->getParams()->lightSelected);
		break;

	case TEXTURE_CALLBACK:
		M->getParams()->ShaderSettings.textureSelected = msg;
		break;

	case ALGORITHM_CALLBACK:
		M->getParams()->SceneSettings.algorithm = msg;
		break;

	default:
		break;
	}
}

void TW_CALL Menu::getCB(void *value, void *clientData) {
	UINT caller = *static_cast<UINT*>(clientData);
	Menu *M = Menu::getInstance();
	switch(caller) {
	case MODEL_CALLBACK:
		*(UINT*)value = M->getParams()->modelSelected;
		break;

	case CAMERA_CALLBACK:
		*(UINT*)value = M->getParams()->cameraSelected;
		break;

	case LIGHT_CALLBACK:
		*(UINT*)value = M->getParams()->lightSelected;
		break;

	case TEXTURE_CALLBACK:
		*(UINT*)value = M->getParams()->ShaderSettings.textureSelected;
		break;

	case ALGORITHM_CALLBACK:
		*(UINT*)value = M->getParams()->SceneSettings.algorithm;
		break;

	default:
		value = 0;
		break;
	}
}

void Menu::updateMenuParamsLight(const UINT i) {
	int opened;
	TwRemoveVar(m_TwLightBar, "LightShow");
	TwAddVarRW(m_TwLightBar,  "LightShow", TW_TYPE_BOOLCPP, &m_MenuParams.Lights[m_MenuParams.lightSelected].showLight, "label='Show light source' group=Light");
	TwGetParam(m_TwLightBar, "LightRotationSelf", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwLightBar, "LightRotationSelf");
	TwAddVarRW(m_TwLightBar,  "LightRotationSelf", TW_TYPE_QUAT4F, &m_MenuParams.Lights[m_MenuParams.lightSelected].RotationSelf, "showval=true label='Rotation self' group=Light");
	TwSetParam(m_TwLightBar, "LightRotationSelf", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwLightBar, "LightTranslation", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwLightBar, "LightTranslation");
	TwAddVarRW(m_TwLightBar,  "LightTranslation", TW_TYPE_DIR3F, &m_MenuParams.Lights[m_MenuParams.lightSelected].Translation, "showval=true label='Translation' group=Light");
	TwSetParam(m_TwLightBar, "LightTranslation", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwLightBar, "LightRotationGlobal", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwLightBar, "LightRotationGlobal");
	TwAddVarRW(m_TwLightBar,  "LightRotationGlobal", TW_TYPE_QUAT4F, &m_MenuParams.Lights[m_MenuParams.lightSelected].RotationGlobal, "showval=true label='Rotation global' group=Light");
	TwSetParam(m_TwLightBar, "LightRotationGlobal", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwLightBar, "LightColor", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwLightBar, "LightColor");
	TwAddVarRW(m_TwLightBar,  "LightColor", TW_TYPE_COLOR3F, &m_MenuParams.Lights[m_MenuParams.lightSelected].Color, "label=Color group=Light");
	TwSetParam(m_TwLightBar, "LightColor", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwLightBar, "LightIntensity");
	TwAddVarRW(m_TwLightBar,  "LightIntensity", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].intensity, "min=0 step=1 label=Intensity group=Attenuation");
	TwRemoveVar(m_TwLightBar, "LightA0");
	TwAddVarRW(m_TwLightBar,  "LightA0", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].a0, "min=0 max=1 step=0.01 label=a0 group=Attenuation");
	TwRemoveVar(m_TwLightBar, "LightA1");
	TwAddVarRW(m_TwLightBar,  "LightA1", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].a1, "min=0 max=1 step=0.01 label=a1 group=Attenuation");
	TwRemoveVar(m_TwLightBar, "LightA2");
	TwAddVarRW(m_TwLightBar,  "LightA2", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].a2, "min=0 max=1 step=0.01 label=a2 group=Attenuation");
	TwRemoveVar(m_TwLightBar, "LightFovy");
	TwAddVarRW(m_TwLightBar,  "LightFovy", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].fovy, "min=0.1 max=180 step=0.1 label=Fovy group=Light");
	TwRemoveVar(m_TwLightBar, "LightZNear");
	TwAddVarRW(m_TwLightBar,  "LightZNear", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].zNear, "min=1 step=1 label=zNear group=Light");
	TwRemoveVar(m_TwLightBar, "LightZFar");
	TwAddVarRW(m_TwLightBar,  "LightZFar", TW_TYPE_FLOAT, &m_MenuParams.Lights[m_MenuParams.lightSelected].zFar, "min=1 step=1 label=zFar group=Light");
}

void Menu::updateMenuParamsModel(const UINT i) {
	int opened;
	TwGetParam(m_TwModelBar, "ModelRotationSelf", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwModelBar, "ModelRotationSelf");
	TwAddVarRW(m_TwModelBar, "ModelRotationSelf", TW_TYPE_QUAT4F, m_MenuParams.Models[m_MenuParams.modelSelected].RotationSelf, "showval=true label='Rotation self' group=Model");
	TwSetParam(m_TwModelBar, "ModelRotationSelf", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwModelBar, "ModelTranslation", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwModelBar, "ModelTranslation");
	TwAddVarRW(m_TwModelBar, "ModelTranslation", TW_TYPE_DIR3F, m_MenuParams.Models[m_MenuParams.modelSelected].Translation, "showval=true label='Translation' group=Model");
	TwSetParam(m_TwModelBar, "ModelTranslation", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwModelBar, "ModelRotationGlobal", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwModelBar, "ModelRotationGlobal");
	TwAddVarRW(m_TwModelBar, "ModelRotationGlobal", TW_TYPE_QUAT4F, m_MenuParams.Models[m_MenuParams.modelSelected].RotationGlobal, "showval=true label='Rotation global' group=Model");
	TwSetParam(m_TwModelBar, "ModelRotationGlobal", "opened", TW_PARAM_INT32, 1, &opened);
}

void Menu::updateMenuParamsCamera(const UINT i) {
	int opened;
	TwGetParam(m_TwCameraBar, "CameraRotationSelf", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwCameraBar, "CameraRotationSelf");
	TwAddVarRW(m_TwCameraBar, "CameraRotationSelf", TW_TYPE_QUAT4F, m_MenuParams.Cameras[m_MenuParams.cameraSelected].RotationSelf, "showval=true label='Rotation self' group=Camera");
	TwSetParam(m_TwCameraBar, "CameraRotationSelf", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwCameraBar, "CameraTranslation", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwCameraBar, "CameraTranslation");
	TwAddVarRW(m_TwCameraBar, "CameraTranslation", TW_TYPE_DIR3F, m_MenuParams.Cameras[m_MenuParams.cameraSelected].Translation, "showval=true label='Translation' group=Camera");
	TwSetParam(m_TwCameraBar, "CameraTranslation", "opened", TW_PARAM_INT32, 1, &opened);
	TwGetParam(m_TwCameraBar, "CameraRotationGlobal", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwCameraBar, "CameraRotationGlobal");
	TwAddVarRW(m_TwCameraBar, "CameraRotationGlobal", TW_TYPE_QUAT4F, m_MenuParams.Cameras[m_MenuParams.cameraSelected].RotationGlobal, "showval=true label='Rotation global' group=Camera");
	TwSetParam(m_TwCameraBar, "CameraRotationGlobal", "opened", TW_PARAM_INT32, 1, &opened);
	TwRemoveVar(m_TwCameraBar, "CameraFovy");
	TwAddVarRW(m_TwCameraBar, "CameraFovy", TW_TYPE_FLOAT, &m_MenuParams.Cameras[m_MenuParams.cameraSelected].fovy, "min=0.1 max=180 step=0.1 label=Fovy group=Camera");
	TwRemoveVar(m_TwCameraBar, "CameraZNear");
	TwAddVarRW(m_TwCameraBar, "CameraZNear", TW_TYPE_FLOAT, &m_MenuParams.Cameras[m_MenuParams.cameraSelected].zNear, "min=1 step=1 label=zNear group=Camera");
	TwRemoveVar(m_TwCameraBar, "CameraZFar");
	TwAddVarRW(m_TwCameraBar, "CameraZFar", TW_TYPE_FLOAT, &m_MenuParams.Cameras[m_MenuParams.cameraSelected].zFar, "min=1 step=1 label=zFar group=Camera");
}