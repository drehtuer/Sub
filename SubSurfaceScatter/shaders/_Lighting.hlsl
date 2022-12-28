#ifndef _LIGHTING_HLSL
#define _LIGHTING_HLSL

#include "_Buffers.hlsl"
#include "_Utilities.hlsl"

float3 getLight(uint lightID) {
    return float3(g_light2World[lightID][3][0], g_light2World[lightID][3][1], g_light2World[lightID][3][2]);
}

float3 getCamera() {
    return float3(g_camera2World[3][0], g_camera2World[3][1], g_camera2World[3][2]);
}

float toLuminance(float3 RGB) {
    return 0.2126f * RGB.r + 0.7152f * RGB.g + 0.0722f * RGB.b;
}

// rho_s: specular brightness
// m:     roughness
void getSurfaceParamsTS(float2 texCoords, out float rho_s, out float m) {
    float2 Spec = g_SpecTex.Sample(g_SamplerPoint, texCoords).ra;
    rho_s = Spec.x * 0.16f + 0.18f; // magic parameters from human head shaders
    m = Spec.y * 0.09f + 0.23f; // magic parameters from human head shaders
}

void getSurfaceParamsSS(float2 texCoords, out float rho_s, out float m) {
    float2 Spec = g_SpecMap.Sample(g_SamplerPoint, texCoords).ra;
    rho_s = Spec.x * 0.16f + 0.18f; // magic parameters from human head shaders
    m = Spec.y * 0.09f + 0.23f; // magic parameters from human head shaders
}

float getRhoD(float rho_s, float cosTerm, float m) {
    return 1.0f - rho_s * g_Rho_dTex.Sample(g_SamplerPoint, float2(cosTerm, m)).r;
}

// attenuation.x: I
// attenuation.y: a0
// attenuation.z: a1
// attenuation.w: a2
float lightAttenuation(uniform float4 attenuation, float dist) {
    return attenuation.x / (attenuation.y + dist * attenuation.z + dist * dist * attenuation.w + 0.000001f);
}

float fresnelReflectance(float3 H, float3 V, uniform float F0) {
    float base = 1.0f - max(dot(V, H), 0.0f);
    float exponential = pow(base, 5);
    return exponential + F0 * (1.0f - exponential);
}

float PHBeckmann(float NdotH, float m) {
	float alpha = acos(NdotH);
	float ta = tan(alpha);
	float m2 = m * m;
	float ta2 = ta * ta;
	return exp(-ta2 / m2) / (m2 * pow(NdotH, 4.0f));
}
                                                                                                                             
float Specular_KelemenSzirmayKalos(float3 N, float3 L, float3 V, float m, float rho_s, uniform float F0) {
    float NdotL = saturate(dot(N, L));
    float3 h = L + V;
    float3 H = normalize(h);
    float NdotH = saturate(dot(N, H));
    float PH = pow(2.0f * g_Rho_dTex.Sample(g_SamplerPoint, float2(NdotH, 1-m)).g, 10.0f);
    //float PH = PHBeckmann(NdotH, m);
    float F = fresnelReflectance(H, V, F0);
    float frSpec = max(PH * F / dot(h, h), 0.0f);
    return NdotL * rho_s * frSpec;
}

float3 Diffuse_Lambertian(float3 N, float3 L) {
	return max(dot(N, L), 0.0f);
}

float3 Diffuse_OrenNayar(float3 N, float3 L, float3 V, float sigma) {
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);
	float theta_i = acos(NdotL);
	float theta_r = acos(NdotV);
	float alpha = max(theta_i, theta_r);
	float beta = min(theta_i, theta_r);
	float cos_phi_diff = dot(normalize(V - N * NdotV), normalize(L - N * NdotL));
	float sigma2 = sigma * sigma;
	float A = 1 - 0.5 * sigma2 / (sigma2 + 0.33f);
	float B = 0.45f * sigma2 / (sigma2 + 0.09f);
	
	return max(NdotL, 0.0f) * (A + B * max(cos_phi_diff, 0.0f) * sin(alpha) * tan(beta));
}

float Specular_Phong(float N, float L, float V, float exponent) {
	float3 R = reflect(-V, N);
	float RdotL = dot(R, L);
	return max(dot(N, L), 0.0f) * pow(saturate(RdotL), exponent);
}

float LinearDepthUnscaled(float3 vertex3d, uint lightID) {
    float3 light3d = getLight(lightID);
    return distance(light3d, vertex3d);
}

float LinearDepth(float3 vertex3d, uint lightID) {
	return LinearDepthUnscaled(vertex3d, lightID) / g_lightZNearFar[lightID].y;
}

float LinearDepth2Distance(float depth, uint lightID) {
    float3 light3d = getLight(lightID);
    return depth * (g_lightZNearFar[lightID].y - g_lightZNearFar[lightID].x) + g_lightZNearFar[lightID].x;
}

float LinearDepthCam(float3 vertex3d) {
    float3 camera3d = getCamera();
    return (distance(vertex3d, camera3d) - g_cameraZNear) / (g_cameraZFar - g_cameraZNear);
}

// project vertex to light space
float4 _toShadowTex(float3 vertex3d, uint lightID) {
    float4 shadowTexCoords = mul(float4(vertex3d.xyz, 1.0f), g_world2LightProj[lightID]);
    shadowTexCoords /= shadowTexCoords.w;
    // screen space [-1, 1] to texture space [0, 1]
    shadowTexCoords.x = +0.5f * shadowTexCoords.x + 0.5f;
    shadowTexCoords.y = -0.5f * shadowTexCoords.y + 0.5f;
    return shadowTexCoords;
}

float _GetShadowVSM(float depth, float4 shadowTexCoords, uniform Texture2D ShadowMap) {
	// sample area around pixel
    float2 shadowMap_dxy = 1.0f / g_shadowMapSize;
    
    // VSM 3x3
    // r: depth, a: depth^2
    float2 ShadowSamples[9];
    ShadowSamples[0] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy                                             ).ra; // center
    ShadowSamples[1] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2(-shadowMap_dxy.x, -shadowMap_dxy.y)).ra; // left top
    ShadowSamples[2] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2(               0, -shadowMap_dxy.y)).ra; // mid top
    ShadowSamples[3] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2( shadowMap_dxy.x, -shadowMap_dxy.y)).ra; // right top
    ShadowSamples[4] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2( shadowMap_dxy.x,                0)).ra; // right mid
    ShadowSamples[5] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2( shadowMap_dxy.x,  shadowMap_dxy.y)).ra; // right bottom
    ShadowSamples[6] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2(               0,  shadowMap_dxy.y)).ra; // mid bottom
    ShadowSamples[7] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2(-shadowMap_dxy.x,  shadowMap_dxy.y)).ra; // left bottom
    ShadowSamples[8] = ShadowMap.Sample(g_SamplerLinear, shadowTexCoords.xy + float2(-shadowMap_dxy.x,                0)).ra; // left mid
    float2 moments = float2(0.0f, 0.0f);
    for(uint i=0; i<9; ++i) {
		moments += float2(ShadowSamples[i].r, ShadowSamples[i].g);
	}
	moments /= 9;
	float sigma2 = moments.g - moments.r * moments.r;
	float shadowResult = max(depth < (moments.r + g_shadowMapEpsilon), sigma2 / (sigma2 + pow(depth - moments.r, 2.0f)));
    
    return saturate(shadowResult);
}

float _GetShadowPCF(float depth, float4 shadowTexCoords, uniform Texture2D ShadowMap) {
    // sample area around pixel
    float2 shadowMap_dxy = 1.0f / g_shadowMapSize;
    
    // pcf shadow map (3x3)
    float ShadowSamples[9];
    ShadowSamples[0] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy                                             ).r; // center
    ShadowSamples[1] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2(-shadowMap_dxy.x, -shadowMap_dxy.y)).r; // left top
    ShadowSamples[2] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2(               0, -shadowMap_dxy.y)).r; // mid top
    ShadowSamples[3] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2( shadowMap_dxy.x, -shadowMap_dxy.y)).r; // right top
    ShadowSamples[4] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2( shadowMap_dxy.x,                0)).r; // right mid
    ShadowSamples[5] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2( shadowMap_dxy.x,  shadowMap_dxy.y)).r; // right bottom
    ShadowSamples[6] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2(               0,  shadowMap_dxy.y)).r; // mid bottom
    ShadowSamples[7] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2(-shadowMap_dxy.x,  shadowMap_dxy.y)).r; // left bottom
    ShadowSamples[8] = ShadowMap.Sample(g_SamplerPoint, shadowTexCoords.xy + float2(-shadowMap_dxy.x,                0)).r; // left mid
    
    float result = 0.0f;
    for(uint i=0; i<9; ++i)
        result += depth < (ShadowSamples[i] + g_shadowMapEpsilon);

    return result / 9;
}

// pcf shadow map
float _GetShadow(float depth, float4 shadowTexCoords, uniform Texture2D ShadowMap) {
	// point outside of the shadow map -> no light
    if(shadowTexCoords.x < 0.0f || shadowTexCoords.x > 1.0f ||
       shadowTexCoords.y < 0.0f || shadowTexCoords.y > 1.0f ||
       shadowTexCoords.z < 0.0f) {
        return 0.0f;
    } else {
		#if VARIANCESHADOWMAP
			return _GetShadowVSM(depth, shadowTexCoords, ShadowMap);
		#else
			return _GetShadowPCF(depth, shadowTexCoords, ShadowMap);
		#endif
	}
}

float GetShadow(float3 vertex3d, uint lightID) {
    float depth = LinearDepth(vertex3d, lightID);
    float4 shadowTexCoords = _toShadowTex(vertex3d, lightID);
    
    return _GetShadow(depth, shadowTexCoords, g_ShadowMap[lightID * NUMDEPTHPEELING]);
}

float GetShadow(float3 vertex3d, uint lightID, uniform Texture2D ShadowMap) {
    float depth = LinearDepth(vertex3d, lightID);
    float4 shadowTexCoords = _toShadowTex(vertex3d, lightID);
    
    return _GetShadow(depth, shadowTexCoords, ShadowMap);
}

// shadow map for texture space
void GetShadowTS(float3 vertex3d, uint lightID, out float depth, out float shadow, out float3 normal, out float3 albedo, out float4 shadowTexCoords) {
    depth = LinearDepth(vertex3d, lightID);
    shadowTexCoords = _toShadowTex(vertex3d, lightID);

    normal = uncompressVector(g_NormalTex.Sample(g_SamplerPoint, shadowTexCoords.xy).xyz);
    albedo = g_AlbedoTex.Sample(g_SamplerLinear, shadowTexCoords.xy).xyz;
    
    shadow = _GetShadow(depth, shadowTexCoords, g_ShadowMap[lightID * NUMDEPTHPEELING]);
}

// shadow map for modified translucent shadow map (d'Eon)
void GetShadowTSM(float3 vertex3d, uint lightID, float3 N, out float depth, out float shadow, out float backFacingEst, out float thicknessToLight) {
    depth = LinearDepth(vertex3d, lightID);
    float4 shadowTexCoords = _toShadowTex(vertex3d, lightID);

    // shadow
    shadow = _GetShadow(depth, shadowTexCoords, g_TSM[lightID]);

    // backface normal
    float3 TSM_DUV = g_TSM[lightID].Sample(g_SamplerPoint, shadowTexCoords.xy).xyz; // depth, u, v
    float3 Nback = normalize(uncompressVector(g_NormalTex.Sample(g_SamplerPoint, TSM_DUV.yz).xyz)); // problem with vertex normals
    Nback = normalize(mul(float4(Nback, 0.0f), g_object2World_IT).xyz);
    backFacingEst = saturate(-dot(N, Nback)); // how 'opposed' are the normals
    thicknessToLight = /*max(*/depth - TSM_DUV.x/* - g_shadowMapEpsilon*//*, 0.0f)*/; // both scaled to [0,1]
}

// shadow map for Jimenez screen space
void GetShadowJimenez(float3 vertex3d, uint lightID, out float depth, out float shadow, out float4 shadowTexCoords) {
    depth = LinearDepth(vertex3d, lightID);
    shadowTexCoords = _toShadowTex(vertex3d, lightID);

    shadow = _GetShadow(depth, shadowTexCoords, g_ShadowMap[lightID * NUMDEPTHPEELING]);
}

// shadow map for Brand screen space
void GetShadowBrand(float3 vertex3d, uint lightID, out float depth, out float shadow, out float3 backsideAlbedo, out float3 backsideNormal, out float4 shadowTexCoords) {
    depth = LinearDepth(vertex3d, lightID);
    shadowTexCoords = _toShadowTex(vertex3d, lightID);

    shadow = _GetShadow(depth, shadowTexCoords, g_ShadowMap[lightID * NUMDEPTHPEELING]);
    backsideAlbedo = g_ShadowAlbedo[lightID].Sample(g_SamplerLinear, shadowTexCoords).rgb;
    backsideNormal = normalize(uncompressVector(g_ShadowNormal2[lightID].Sample(g_SamplerLinear, shadowTexCoords)));
}


void GetLightAttenuation(uint lightID, float3 vertex3d, out float3 L, out float attenuation) {
    float3 l = getLight(lightID) - vertex3d;
    float lightDistance = length(l);
    L = normalize(l);
    attenuation = lightAttenuation(g_lightAttenuation[lightID], lightDistance);
}
    

// transmittance function (Jimenez)
// T = \sum^k_{i=1}w_i * exp(-s^2 / v_i)
float3 T(float s) {
    return float3(0.233f, 0.455f, 0.649f) * exp(-s*s/0.0064f) + 
           float3(0.100f, 0.336f, 0.344f) * exp(-s*s/0.0484f) + 
           float3(0.118f, 0.198f, 0.000f) * exp(-s*s/0.187f) + 
           float3(0.113f, 0.007f, 0.007f) * exp(-s*s/0.567f) + 
           float3(0.358f, 0.004f, 0.000f) * exp(-s*s/1.99f) + 
           float3(0.078f, 0.000f, 0.000f) * exp(-s*s/7.41f);
}

// calculate distance through an object
// Jimenez
float distance_through_object(float3 vertex3d, float3 N, float depth, uint lightID) {
    float3 shrinkedVertex3d = vertex3d - 1.0f * N;
    float4 shadowTexCoords = _toShadowTex(shrinkedVertex3d, lightID);
    float d1 = g_ShadowMap[lightID * NUMDEPTHPEELING].Sample(g_SamplerLinear, shadowTexCoords.xy).r;
    float d2 = depth;
    float dist = d2 - d1; // unorm
    if(dist < 0.0f)
        return 9001.0f; // huge -> no transmittance
    else
        return dist;
}

// calculate distance through an object
// Brand with shadow peeling
float2 distance_through_object2(float3 vertex3d, float3 N, float depthPrev, uint lightID) {
    float3 shrinkedVertex3d = vertex3d - 1.0f * N;
    float4 shadowTexCoords = _toShadowTex(shrinkedVertex3d, lightID);
    float dist = 0.0f, mask;
    uint counter = 0;
    for(uint peel=0; peel < NUMDEPTHPEELING; ++peel) {
        // from last peel to first
        float depthNext = g_ShadowMap[lightID * NUMDEPTHPEELING + (NUMDEPTHPEELING-1)-peel].Sample(g_SamplerLinear, shadowTexCoords.xy).r;
        if(depthNext < 0.99f) { // 1.0f == nothing
            dist += depthPrev - depthNext; // depthPrev (farer) >= depthNext (nearer), unorm
            depthPrev = depthNext;
            counter++;
        }
    }
    if(counter > NUMDEPTHPEELING-1) { // 3 layers -> no transmittance
        mask = 0.0f; // no transmittance
    } else {
        if(dist < 0.0f) { // just to be sure
            mask = 0.0f; // huge -> no transmittance
        } else {
            mask = 1.0f; // yay, transmittance
        }
    }
	return float2(dist, mask);
}

#endif
