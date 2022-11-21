#include "Sprite.hlsli"

Texture2D<float4> tex : register(t0);  	// 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0);      	// 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

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
