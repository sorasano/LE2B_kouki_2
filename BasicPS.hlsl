#include "Basic.hlsli"

Texture2D<float4>tex : register(t0);	//0番スロットに設定されたテクスチャ
SamplerState smp : register(s0);			//0番スロットに設定されたサンプラー

float4 main(VSOutput input) : SV_TARGET
{
	//右下奥	向きのライト
	float3 light = normalize(float3(1,-1,1));
	//diffuseを[0,1]の範囲にClampする
	float diffuse = saturate(dot(-light, input.normal));
	//アビエント項を0.3として出力
	float brightness = diffuse + 0.3f;

	//テクスチャマッピングの色を一時的に保存
	float4 texcolor = float4(tex.Sample(smp, input.uv));

	/*return float4(tex.Sample(smp,input.uv));*/
	/*return float4(1,0,1,1);*/
	/*return float4(input.normal,1);*/
	//輝度をRGBに代入して出力
	return float4(texcolor.rgb * brightness,texcolor.a);
}

//float WhiteNoise(float2 coord) {
//	return frac(sin(dot(coord, float2(8.7819, 3.255))) * 437.645);
//}
//
//float4 main(VSOutput input) : SV_TARGET
//{
//
//	float2 samplePoint = input.uv;
//	float4 Tex = tex.Sample(smp, samplePoint);
//	float noise = WhiteNoise(input.uv * Time) - 0.5;
//	Tex.rgb += float3(noise, noise, noise);
//	return Tex;
//}
