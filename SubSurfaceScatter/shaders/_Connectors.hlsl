#ifndef _CONNECTORS_HLSL
#define _CONNECTORS_HLSL

struct IA2VS {
	float3 vertex    : POSITION0;
	float3 normal    : NORMAL0;
	float2 texCoords : TEXCOORD0;
};

struct VS2PS {
	float3 vertex3d        : POSITION0;
	float3 normal          : NORMAL0;
	float2 texCoords       : TEXCOORD0;
	float4 vertex2d        : SV_Position;
	float3 lightColor      : LIGHTCOLOR0;
};

struct VS2PS_light {
	float3 vertex3d        : POSITION0;
	float3 normal          : NORMAL0;
	float2 texCoords       : TEXCOORD0;
	float4 vertex2d        : SV_Position;
	float3 camera3d        : CAMERAPOS;
	float3 light3d         : LIGHT0POS;
	float3 lightColor      : LIGHT0COLOR;
	float4 lightAttenuation : LIGHT0ATTENUATION;
	float4 shadowTexCoords : TEXCOORD1;
};

struct PS2OM {
	float4 pixel : SV_Target;
};

#endif
