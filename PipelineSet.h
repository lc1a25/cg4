#pragma once
#include <d3d12.h>
#include <wrl.h>
class PipelineSet
{
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略

public:
		ComPtr<ID3D12PipelineState> pipelinestate;//パイプラインステートオブジェクト
		ComPtr<ID3D12RootSignature> rootsignature;//ルートシグネチャ
	
};
