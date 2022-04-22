#include "BasicShaderHeader.hlsli"

//cbuffer cbuff0 : register(b0)
//{
//	float4 color;
//};
//
//float4 main() : SV_TARGET
//{
//	return color;
//}

Texture2D<float4> tex : register(t0);//0番目に設定されたテクスチャ
SamplerState smp : register(s0);//0番目に設定されたサンプラー



float4 main(VSOutput input) : SV_TARGET
{
	float4 texcolor = float4(tex.Sample(smp, input.uv)) * color;


	float3 light = normalize(float3(1, -1, 1));

	float diffuse = saturate(dot(-light, input.normal));

	float brightness = diffuse + 0.3f;

	return float4(texcolor.rgb * brightness, texcolor.a) * color;
	//return float4(brightness, brightness, brightness, 1);

	//return float4(input.normal,1);//RGBをそれぞれの法線xyz Aを1で出力
}