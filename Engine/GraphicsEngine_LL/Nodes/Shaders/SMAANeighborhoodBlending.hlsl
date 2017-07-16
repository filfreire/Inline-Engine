/*
* SMAA neighborhood blending shader
* Input0: LDR color texture
* Input1: Blending weights texture
* Output: Antialiased texture
*/

struct Uniforms
{
	float4 pixelSize;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //LDR texture
Texture2D blendingWeightsTex : register(t3); //Blending weights texture
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
	float4 offset : TEX_COORD1;
};

#define SMAA_HLSL_4_1
#define SMAA_PRESET_ULTRA 1
#define SMAA_RT_METRICS uniforms.pixelSize

#include "SMAA"

PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	SMAANeighborhoodBlendingVS(texcoord, result.offset);

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	return SMAANeighborhoodBlendingPS(input.texcoord, input.offset, inputTex, blendingWeightsTex);
}
