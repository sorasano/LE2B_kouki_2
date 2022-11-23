#include "peraHeader.hlsli"

//float BlackChange(float2 coord) {
//	return frac(sin(dot(coord, float2(8.7819, 3.255))) * 437.645);
//}


float4 main(Output input) : SV_Target
{


	//return float4(input.uv,1,1);
	float4 col = tex.Sample(smp, input.uv);
	return float4(1.0f - col.rgb,col.a);
	//
	//	
	//float4 col = tex.Sample(smp, input.uv);
	//col.rgb = col.rgb - time;
	//return float4(col.rgb,col.a);

}

