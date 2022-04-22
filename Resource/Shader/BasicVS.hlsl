#include "BasicShaderHeader.hlsli"


VSOutput main(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	VSOutput output;
	/*output.svpos = pos;*/
	output.svpos = mul(mat, pos);//座標に行列を乗算
	output.normal = normal;//受け取った法線をそのまま出力
	output.uv = uv;
	return output;
}