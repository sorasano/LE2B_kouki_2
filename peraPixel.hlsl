#include "peraHeader.hlsli"

float4 main(Output input) : SV_Target
{
	//float4 col = tex.Sample(smp, input.uv);
	return tex.Sample(smp,input.uv);
	/*return float4(input.uv,1,1);*/
	//return float4(1.0f - col.rgb,col.a);
}