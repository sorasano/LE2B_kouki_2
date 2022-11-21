#include "Sprite.hlsli"

Texture2D<float4> tex : register(t0);  	// 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      	// 0番スロットに設定されたサンプラー

float WhiteNoise(float2 coord) {
	return frac(sin(dot(coord, float2(8.7819, 3.255))) * 437.645);
}

float4 main(VSOutput input) : SV_TARGET
{
	float2 samplePoint = input.uv;
	float4 Tex = tex.Sample(smp, samplePoint);
	float noise = WhiteNoise(input.uv * time) - 0.5;
	Tex.rgb += float3(noise, noise, noise);
	return Tex;
}
