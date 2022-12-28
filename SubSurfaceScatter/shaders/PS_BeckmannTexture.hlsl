#ifndef PS_BECKMANNTEXTURE_HLSL
#define PS_BECKMANNTEXTURE_HLSL

#include "_Connectors.hlsl"
#include "_Utilities.hlsl"

#define SIZE 512

float PHBeckmann(float NdotH, float m) {
	float alpha = acos(NdotH);
	float ta = tan(alpha);
	float m2 = m * m;
	float ta2 = ta * ta;
	float val = exp(-ta2 / m2) / (m2 * pow(NdotH, 4.0f));
	return val;
}

void Main(VS2PS psIn, out PS2OM psOut) {
	float beckmann = 0.5f * pow(PHBeckmann(psIn.texCoords.x, psIn.texCoords.y), 0.1f); // for NdotH from [0, 1] and m from [0, 1]
	psOut.pixel = make4(beckmann);
}

#endif
