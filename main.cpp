#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
//#include <DirectXMath.h>
#include <vector>
#include <string>
#include <math.h>
#include <d3dcompiler.h>
#include <DirectXTex.h>
#include "Input.h"
#include <wrl.h> // 7-1
#include <d3dx12.h>//7-3
#include<xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include<fstream>

#include "Object3d.h"


#include "Win.h"
#include "DirectXCommon.h"
#include "CollisionPrimitive.h"
#include "Collision.h"


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "DirectXTex.lib")

using namespace Microsoft::WRL;

//#include <d2d1_1.h>
//#pragma comment(lib, "d2d1.lib")

//#include <dsound.h>
//#pragma comment(lib, "dsound.lib")
//#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "winmm.lib")

using namespace DirectX;

//スプライト用テクスチャ枚数
const int spriteSRVCount = 524;




struct ConstBufferData
{
	XMFLOAT4 color;//色
	XMMATRIX mat;//3D変換行列
};

#pragma region object
//3Dオブジェクト型
struct Object3d_1
{
	//定数バッファ
	ComPtr<ID3D12Resource> constBuff;// ID3D12Resource* constBuff;

	//定数バッファビューのハンドル(CPU)
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleCBV;

	//定数バッファビューのハンドル(GPU)
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleCBV;

	//アフィン変換情報
	XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
	XMFLOAT3 rotation = { 45.0f,0.0f,0.0f };
	XMFLOAT3 position = { 0.0f,0.0f,0.0f };

	//ワールド変換行列
	XMMATRIX matWorld;

	//現オブジェクトへのポインタ
	Object3d_1* parent = nullptr;


};
#pragma endregion



//#pragma region object void initialize
//void InitializeObject3d(Object3d_1* object, int index, ComPtr<ID3D12Device> dev, ComPtr < ID3D12DescriptorHeap> descHeap)
//{
//
//	HRESULT result;
//
//	CD3DX12_HEAP_PROPERTIES heapprop{};//D3D12_HEAP_PROPERTIES heapprop{};
//	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
//
//	CD3DX12_RESOURCE_DESC resdesc{};//D3D12_RESOURCE_DESC resdesc{};
//	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	resdesc.Width = (sizeof(ConstBufferData) + 0xff) & ~0xff;
//	resdesc.Height = 1;
//	resdesc.DepthOrArraySize = 1;
//	resdesc.MipLevels = 1;
//	resdesc.SampleDesc.Count = 1;
//	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//
//	result = dev->CreateCommittedResource(
//		&heapprop,
//		D3D12_HEAP_FLAG_NONE,
//		&resdesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&object->constBuff));
//
//	UINT descHandleIncrementSize =
//		dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	object->cpuDescHandleCBV = descHeap->GetCPUDescriptorHandleForHeapStart();
//	object->cpuDescHandleCBV.ptr += index * descHandleIncrementSize;
//
//	object->gpuDescHandleCBV = descHeap->GetGPUDescriptorHandleForHeapStart();
//	object->gpuDescHandleCBV.ptr += index * descHandleIncrementSize;
//
//	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
//	cbvDesc.BufferLocation = object->constBuff->GetGPUVirtualAddress();
//	cbvDesc.SizeInBytes = (UINT)object->constBuff->GetDesc().Width;
//	dev->CreateConstantBufferView(&cbvDesc, object->cpuDescHandleCBV);
//
//}
//#pragma endregion

#pragma region updateobject
void UpdateObject3d(Object3d_1* object, XMMATRIX& matView, XMMATRIX& matProjection)
{



	XMMATRIX matScale, matRot, matTrans;

	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();

	matRot *= XMMatrixRotationZ(XMConvertToRadians(object->rotation.z));//z軸回転
	matRot *= XMMatrixRotationX(XMConvertToRadians(object->rotation.x));//x軸回転
	matRot *= XMMatrixRotationY(XMConvertToRadians(object->rotation.y));//y軸回転

	matTrans = XMMatrixTranslation(
		object->position.x, object->position.y, object->position.z);

	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	if (object->parent != nullptr)
	{
		object->matWorld *= object->parent->matWorld;
	}

	ConstBufferData* constMap = nullptr;
	if (SUCCEEDED(object->constBuff->Map(0, nullptr, (void**)&constMap)))
	{
		constMap->color = XMFLOAT4(1, 1, 1, 1);
		constMap->mat = object->matWorld * matView * matProjection;
		object->constBuff->Unmap(0, nullptr);
	}

}
#pragma endregion

#pragma region object draw
void DrawObject3d(Object3d_1* object, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* descHeap,
	D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, D3D12_GPU_DESCRIPTOR_HANDLE
	gpuDescHandleSRV2, UINT numIndices)
{
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	cmdList->IASetIndexBuffer(&ibView);//インデックスバッファビューのセットコマンド

	ID3D12DescriptorHeap* ppHeaps[] = { descHeap };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//cmdList->SetGraphicsRootDescriptorTable(0, object->gpuDescHandleCBV);

	cmdList->SetGraphicsRootConstantBufferView(0, object->constBuff->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV2);

	cmdList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

}
#pragma endregion

struct VertexPosUv
{
	XMFLOAT3 pos;//x y z　座標
	XMFLOAT2 uv;//u v 座標
};

struct PipelineSet//パイプラインセット
{
	ComPtr<ID3D12PipelineState> pipelinestate;//パイプラインステートオブジェクト
	ComPtr<ID3D12RootSignature> rootsignature;//ルートシグネチャ
};

#pragma region 3dpipeline
PipelineSet Create3dpipe(ID3D12Device* dev)
{
	HRESULT result = S_OK;
	ComPtr<ID3DBlob> vsBlob;// ID3DBlob* vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;//ID3DBlob* psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob;//ID3DBlob* errorBlob = nullptr; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resource/shader/BasicVS.hlsl",  // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resource/shader/BasicPS.hlsl",   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}


	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}, // (1行で書いたほうが見やすい)

		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}, // (1行で書いたほうが見やすい)
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}, // (1行で書いたほうが見やすい)
	};
	CD3DX12_DESCRIPTOR_RANGE  descRangeSRV;
	//descRangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//b0 レジスタ
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);//t0 レジスタ


	CD3DX12_ROOT_PARAMETER rootparams[2];
	rootparams[0].InitAsConstantBufferView(0);//定数バッファビューとして初期化(b0　レジスタ)
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	//頂点シェーダ　ピクセルシェーダ
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	//標準的な設定(背面をカリング ポリゴン内塗りつぶし 深度クリッピングを有効に)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//ブレンド
//gpipeline.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;  // RBGA全てのチャンネルを描画
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = gpipeline.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//共通設定(BLEND)
	blenddesc.BlendEnable = true;					//ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;	//加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;		//ソースの値を100%使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;	//デストの値を0%使う

	//加算合成
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	 //加算
	//blenddesc.SrcBlend = D3D12_BLEND_ONE;	 //ソースの値を100%使う
	//blenddesc.DestBlend = D3D12_BLEND_ONE;	 //デストの値を100%使う

	//半透明合成
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipeline.NumRenderTargets = 1; // 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	//デプスステンシルステートの設定

	//標準的な設定(深度テストを行う 書き込み許可 小さければ合格)
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;//深度値フォーマット

	//デスクリプタテーブルの設定
	D3D12_DESCRIPTOR_RANGE descTblRange{};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータ
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;



	//ルートシグネチャ生成
	ComPtr<ID3D12RootSignature> rootsignature;



	//ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	PipelineSet pipelineSet;

	ComPtr<ID3DBlob> rootSigBlob;

	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);

	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&pipelineSet.rootsignature));

	gpipeline.pRootSignature = pipelineSet.rootsignature.Get();

	ComPtr<ID3D12PipelineState> pipelinestate;// ID3D12PipelineState* pipelinestate = nullptr;
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineSet.pipelinestate));

	return pipelineSet;
}
#pragma endregion

#pragma region 2dpipeline
PipelineSet CreateSprite2dpipe(ID3D12Device* dev)
{
	HRESULT result = S_OK;
	ComPtr<ID3DBlob> vsBlob;// ID3DBlob* vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;//ID3DBlob* psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob;//ID3DBlob* errorBlob = nullptr; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resource/shader/SpriteVS.hlsl",  // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resource/shader/SpritePS.hlsl",   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}


	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}, // (1行で書いたほうが見やすい)


		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}, // (1行で書いたほうが見やすい)
	};
	CD3DX12_DESCRIPTOR_RANGE  descRangeSRV;
	//descRangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);//b0 レジスタ
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);//t0 レジスタ


	CD3DX12_ROOT_PARAMETER rootparams[2];
	rootparams[0].InitAsConstantBufferView(0);//定数バッファビューとして初期化(b0　レジスタ)
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	//頂点シェーダ　ピクセルシェーダ
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());


	//ブレンド
//gpipeline.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;  // RBGA全てのチャンネルを描画
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = gpipeline.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//共通設定(BLEND)
	blenddesc.BlendEnable = true;					//ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;	//加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;		//ソースの値を100%使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;	//デストの値を0%使う

	//加算合成
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	 //加算
	//blenddesc.SrcBlend = D3D12_BLEND_ONE;	 //ソースの値を100%使う
	//blenddesc.DestBlend = D3D12_BLEND_ONE;	 //デストの値を100%使う

	//半透明合成
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipeline.NumRenderTargets = 1; // 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

		//標準的な設定(ポリゴン内塗りつぶし 深度クリッピングを有効に)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // 背面をカリングなし

	//デプスステンシルステートの設定

	//標準的な設定(書き込み許可)
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);//一旦標準値をセット
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;//常に上書き
	gpipeline.DepthStencilState.DepthEnable = false;//深度テストしない

	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;//深度値フォーマット

	//デスクリプタテーブルの設定
	D3D12_DESCRIPTOR_RANGE descTblRange{};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータ
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;



	//ルートシグネチャ生成
	ComPtr<ID3D12RootSignature> rootsignature;



	//ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	PipelineSet pipelineSet;

	ComPtr<ID3DBlob> rootSigBlob;

	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);

	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&pipelineSet.rootsignature));

	gpipeline.pRootSignature = pipelineSet.rootsignature.Get();

	ComPtr<ID3D12PipelineState> pipelinestate;// ID3D12PipelineState* pipelinestate = nullptr;
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineSet.pipelinestate));

	return pipelineSet;
}
#pragma endregion 

struct Sprite
{
	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;

	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView;

	//定数バッファ
	ComPtr<ID3D12Resource> constBuff;

	//XYZ軸周りの回転軸
	XMFLOAT3 rotation = { 0.0f,0.0f, 0.0f };

	//座標
	XMFLOAT3 position = { 0,0,0 };

	//ワールド行列
	XMMATRIX matWorld;

	//色
	XMFLOAT4 color = { 1,1,1,1 };

	//テクスチャ番号
	UINT texnumber = 0;

	//大きさ
	XMFLOAT2 size = { 100, 100 };

	//アンカーポイント
	XMFLOAT2 anchorpoint = { 0.5f,0.5f };

	//左右反転
	bool isFlagX = false;

	//上下反転
	bool isFlagY = false;

	//テクスチャ左上座標
	XMFLOAT2 texLeftTop = { 0,0 };

	//テクスチャ切り出しサイズ
	XMFLOAT2 texSize = { 100,100 };

	//非表示
	bool isInvisible = false;
};

struct SpriteCommon
{
	//パイプラインセット
	PipelineSet pipelineSet;

	//射影行列
	XMMATRIX matProjection{};

	//テクスチャ用デスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> descHeap;

	//テクスチャリソース(テクスチャバッファ)の配列
	ComPtr<ID3D12Resource> texBuff[spriteSRVCount];
};

void SpriteTransVertexBuffer(const Sprite& sprite, const SpriteCommon& spriteCommon)
{
	HRESULT result = S_FALSE;

	VertexPosUv vertices[] = {
		{{},{} },
		{{},{} },
		{{},{} },
		{{},{} },

	};

	//	 左下 左上 右下 右上
	enum { LB, LT, RB, RT };

	float left = (0.0f - sprite.anchorpoint.x) * sprite.size.x;

	float right = (1.0f - sprite.anchorpoint.x) * sprite.size.x;

	float top = (0.0f - sprite.anchorpoint.y) * sprite.size.y;

	float bottom = (1.0f - sprite.anchorpoint.y) * sprite.size.y;

	if (sprite.isFlagX == true)
	{
		left = -left;
		right = -right;
	}

	if (sprite.isFlagY == true)
	{
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = { left,	bottom, 0.0f };
	vertices[LT].pos = { left,	top,	0.0f };
	vertices[RB].pos = { right,	bottom, 0.0f };
	vertices[RT].pos = { right,	top,	0.0f };

	//UV
	//指定番号の画像が読み込み済みなら
	if (spriteCommon.texBuff[sprite.texnumber])
	{
		//テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = spriteCommon.texBuff[sprite.texnumber]->GetDesc();

		float tex_left = sprite.texLeftTop.x / resDesc.Width;

		float tex_right = (sprite.texLeftTop.x + sprite.texSize.x) / resDesc.Width;

		float tex_top = sprite.texLeftTop.y / resDesc.Height;

		float tex_bottom = (sprite.texLeftTop.y + sprite.texSize.y) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom };
		vertices[LT].uv = { tex_left,	tex_top };
		vertices[RB].uv = { tex_right,	tex_bottom };
		vertices[RT].uv = { tex_right,	tex_top };

	}

	//頂点バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = sprite.vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	sprite.vertBuff->Unmap(0, nullptr);
}

Sprite SpriteCreate(ID3D12Device* dev, int window_width, int window_height, UINT texnumber, const SpriteCommon& spriteCommon,
	XMFLOAT2 anchorpoint = { 0.5f,0.5f }, bool isFlagX = false, bool isFlagY = false)
{
	HRESULT result = S_FALSE;

	Sprite sprite{};

	//アンカーポイントをコピー
	sprite.anchorpoint = anchorpoint;

	//反転フラグをコピー
	sprite.isFlagX = isFlagX;

	sprite.isFlagY = isFlagY;

	VertexPosUv vertices[] = {
		{{0.0f	,100.0f	,0.0f},{0.0f,1.0f} },
		{{0.0f	,0.0f	,0.0f},{0.0f,0.0f} },
		{{100.0f,100.0f	,0.0f},{1.0f,1.0f} },
		{{100.0f,0.0f	,0.0f},{1.0f,0.0f} },

	};




	//頂点バッファの生成
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite.vertBuff)
	);

	sprite.texnumber = texnumber;

	if (spriteCommon.texBuff[sprite.texnumber])
	{
		//テクスチャ情報の画像が読み込み済みなら
		D3D12_RESOURCE_DESC resDesc = spriteCommon.texBuff[sprite.texnumber]->GetDesc();

		//スプライトの大きさを画像の解像度に合わせる
		sprite.size = { (float)resDesc.Width,(float)resDesc.Height };
	}

	//頂点バッファデータ転送
	SpriteTransVertexBuffer(sprite, spriteCommon);




	//頂点バッファビューへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = sprite.vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	sprite.vertBuff->Unmap(0, nullptr);

	//頂点バッファビューの生成
	sprite.vbView.BufferLocation = sprite.vertBuff->GetGPUVirtualAddress();
	sprite.vbView.SizeInBytes = sizeof(vertices);
	sprite.vbView.StrideInBytes = sizeof(vertices[0]);

	//定数バッファの生成
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite.constBuff));

	//定数バッファにデータを転送
	ConstBufferData* constMap = nullptr;
	result = sprite.constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = XMFLOAT4(1, 1, 1, 1);//色指定(R G B A)

	//平行投影法
	constMap->mat = XMMatrixOrthographicOffCenterLH(
		0.0f, window_width, window_height, 0.0f, 0.0f, 1.0f);
	sprite.constBuff->Unmap(0, nullptr);

	return sprite;
}




SpriteCommon SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height)
{
	HRESULT result = S_FALSE;




	//新たなスプライト共通データを生成
	SpriteCommon spriteCommon{};

	//設定構造
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダーから見える
	descHeapDesc.NumDescriptors = spriteSRVCount;//テクスチャ枚数

	//デスクリプタヒープの生成
	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&spriteCommon.descHeap));//生成

	//スプライト用パイプライン生成
	spriteCommon.pipelineSet = CreateSprite2dpipe(dev);

	//並行投影の射影行列変換
	spriteCommon.matProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f);


	return spriteCommon;
}

void SpriteCommonLoadTexture(SpriteCommon& spritecommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev)
{

	assert(texnumber <= spriteSRVCount - 1);

	HRESULT result;

	TexMetadata metadata{};
	ScratchImage scratchImg{};


	result = LoadFromWICFile(
		filename,
		WIC_FLAGS_NONE,
		&metadata, scratchImg);

	const Image* img = scratchImg.GetImage(0, 0, 0);

	const int texWidth = 256;//横方向ピクセル
	const int imageDataCount = texWidth * texWidth;//配列の要素数


	CD3DX12_HEAP_PROPERTIES texheapProp{};//テクスチャヒープ
	texheapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texheapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texheapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;


	CD3DX12_RESOURCE_DESC texresDesc{};
	texresDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	texresDesc.Format = metadata.format;
	texresDesc.Width = metadata.width;//幅
	texresDesc.Height = (UINT)metadata.height;//高さ
	texresDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	texresDesc.MipLevels = (UINT16)metadata.mipLevels;
	texresDesc.SampleDesc.Count = 1;


	result = dev->CreateCommittedResource(//GPUリソースの生成
		&texheapProp,
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&spritecommon.texBuff[texnumber]));

	//リソース設定
	texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);

	result = dev->CreateCommittedResource(//GPUリソースの生成
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&spritecommon.texBuff[texnumber]));


	//テクスチャバッファにデータ転送
	result = spritecommon.texBuff[texnumber]->WriteToSubresource(
		0,
		nullptr,							//全領域にコピー
		img->pixels,						//元データアドレス
		(UINT)img->rowPitch,				//1ラインサイズ
		(UINT)img->slicePitch);			//全サイズ


	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(spritecommon.descHeap->GetCPUDescriptorHandleForHeapStart(),
			texnumber,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV =
		CD3DX12_GPU_DESCRIPTOR_HANDLE(spritecommon.descHeap->GetGPUDescriptorHandleForHeapStart(),
			texnumber,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//シェーダーリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};//設定構造体
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//RGBA
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	//ヒープの?番目にシェーダーリソースビューを生成
	dev->CreateShaderResourceView(spritecommon.texBuff[texnumber].Get(),//ビューと関連付けるバッファ
		&srvDesc,//テクスチャ設定情報
		CD3DX12_CPU_DESCRIPTOR_HANDLE(spritecommon.descHeap->GetCPUDescriptorHandleForHeapStart(), texnumber,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));

}

void SpriteUpdate(Sprite& sprite, const SpriteCommon& spriteCommon)
{
	//ワールド行列の更新
	sprite.matWorld = XMMatrixIdentity();

	//Z軸回転
	sprite.matWorld *= XMMatrixRotationZ(XMConvertToRadians(sprite.rotation.z));

	//平行移動
	sprite.matWorld *= XMMatrixTranslation(sprite.position.x, sprite.position.y, sprite.position.z);

	//定数バッファの転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = sprite.constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = sprite.color;
	constMap->mat = sprite.matWorld * spriteCommon.matProjection;



	sprite.constBuff->Unmap(0, nullptr);
}

void SpriteCommonBeginDraw(ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon)
{
	//パイプラインステートの設定
	cmdList->SetPipelineState(spriteCommon.pipelineSet.pipelinestate.Get());

	//ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(spriteCommon.pipelineSet.rootsignature.Get());

	//プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャ用デスクリプタヒープ
	ID3D12DescriptorHeap* ppHeaps[] = { spriteCommon.descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}



void  SpriteDraw(const Sprite& sprite, ID3D12GraphicsCommandList* cmdList, SpriteCommon& spriteCommon, ID3D12Device* dev)
{

	if (sprite.isInvisible == true)
	{
		return;
	}
	//頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &sprite.vbView);

	//定数バッファをセット
	cmdList->SetGraphicsRootConstantBufferView(0, sprite.constBuff->GetGPUVirtualAddress());

	//
	cmdList->SetGraphicsRootDescriptorTable(1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(spriteCommon.descHeap->GetGPUDescriptorHandleForHeapStart(),
			sprite.texnumber,
			dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));

	//ポリゴンの描画
	cmdList->DrawInstanced(4, 1, 0, 0);
}

void Drawbye(ID3D12GraphicsCommandList* cmdList)
{
	cmdList = nullptr;
}

class DebugText
{
public:
	static const int maxCharCount = 256;

	static const int fontWidth = 80;

	static const int fontHeight = 85;

	static const int fontLineCount = 16;

	Sprite sprites[maxCharCount];

	int spriteIndex = 0;

	void debugTextInit(ID3D12Device* dev, int window_width, int window_height, UINT texnumber, const SpriteCommon& spriteCommon);

	void Print(const SpriteCommon& spritecommon, const std::string& text, float x, float y, float scale = 1.0f);

	void DrawAll(ID3D12GraphicsCommandList* cmdList, SpriteCommon& spriteCommon, ID3D12Device* dev);
};

void DebugText::debugTextInit(ID3D12Device* dev, int window_width, int window_height, UINT texnumber, const SpriteCommon& spritecommon)
{
	for (int i = 0; i < _countof(sprites); i++)
	{
		sprites[i] = SpriteCreate(dev, window_width, window_height, texnumber, spritecommon, { 0,0 });
	}
}

void DebugText::Print(const SpriteCommon& spritecommon, const std::string& text, float x, float y, float scale)
{
	for (int i = 0; i < text.size(); i++)
	{
		if (spriteIndex >= maxCharCount)
		{
			break;
		}

		const unsigned char& charctor = text[i];

		int fontIndex = charctor - 32;

		if (charctor >= 0x7f)
		{
			fontIndex = 0;
		}

		int fontIndexX = fontIndex % fontLineCount;

		int fontIndexY = fontIndex / fontLineCount;

		sprites[spriteIndex].position = { x + fontWidth * scale * i,y,0 };
		sprites[spriteIndex].texLeftTop = { (float)fontIndexX * fontWidth,(float)fontIndexY * fontHeight };
		sprites[spriteIndex].texSize = { fontWidth,fontHeight };
		sprites[spriteIndex].size = { fontWidth * scale,fontHeight * scale };

		SpriteTransVertexBuffer(sprites[spriteIndex], spritecommon);

		SpriteUpdate(sprites[spriteIndex], spritecommon);

		spriteIndex++;


	}
}

void DebugText::DrawAll(ID3D12GraphicsCommandList* cmdList, SpriteCommon& spriteCommon, ID3D12Device* dev)
{
	for (int i = 0; i < spriteIndex; i++)
	{
		SpriteDraw(sprites[i], cmdList, spriteCommon, dev);
	}

	spriteIndex = 0;
}


struct ChunkHeader
{
	char id[4];
	int32_t size;
};

struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];

};

struct FormatChunk
{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData
{
	WAVEFORMATEX wfex;

	BYTE* pBuffer;

	unsigned int bufferSize;


};

SoundData SoundLoadWave(const char* filename)
{
	HRESULT result;

	//File open
	std::ifstream file;

	file.open(filename, std::ios_base::binary);

	assert(file.is_open());

	//wavData Load
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}

	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}

	FormatChunk format = {};

	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0);
	}

	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);



	ChunkHeader data;
	file.read((char*)&data, sizeof(data));

	if (strncmp(data.id, "JUNK", 4) == 0)
	{
		file.seekg(data.size, std::ios_base::cur);

		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "LIST", 4) == 0)
	{
		file.seekg(data.size, std::ios_base::cur);

		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0)
	{
		assert(0);
	}

	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	file.close();
	//File close

	//return
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;



	return soundData;
}

void SoundUnload(SoundData* soundData)
{
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{
	HRESULT result;

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	// buf.AudioBytes = size;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

void SoundPlayWaveLoop(IXAudio2* xAudio2, const SoundData& soundData)
{
	HRESULT result;

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// buf.AudioBytes = size;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{


// WindowsAPI初期化処理
	

	Win* win = nullptr;
	Input* input = nullptr;
	DirectXCommon* dxcommon = nullptr;

	win = new Win();
	win->WinCreate();

	input = new Input();
	input->Initialize(win);

	// DirectX初期化処理　ここから
	HRESULT result;


	dxcommon = new DirectXCommon();
	dxcommon->Init(win);



	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	//マスターボイスを作成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	// DirectX初期化処理　ここまで

	//描画初期化処理　ここから
#pragma region 描画初期化処理
#pragma region 座標


	struct Vertex
	{
		XMFLOAT3 pos;	//xyz座標
		XMFLOAT3 normal;	//法線ベクトル
		XMFLOAT2 uv;	//uv座標

	};

	SpriteCommon spriteCommon;
	spriteCommon = SpriteCommonCreate(dxcommon->GetDev(), win->window_width, win->window_height);

	SpriteCommonLoadTexture(spriteCommon, 0, L"Resource/end_car2.png", dxcommon->GetDev());
	SpriteCommonLoadTexture(spriteCommon, 1, L"Resource/title_car2.png", dxcommon->GetDev());

	Sprite sprite;

	sprite = SpriteCreate(dxcommon->GetDev(), win->window_width, win->window_height, 0, spriteCommon);

	sprite.texnumber = 0;

	Sprite sprite1;

	sprite1 = SpriteCreate(dxcommon->GetDev(), win->window_width, win->window_height, 1, spriteCommon);

	sprite1.texnumber = 1;

	//デバッグテキスト

	DebugText debugtext;

	const int debugTextTexNumber = 2;

	SpriteCommonLoadTexture(spriteCommon, debugTextTexNumber, L"Resource/ASC_White.png", dxcommon->GetDev());

	debugtext.debugTextInit(dxcommon->GetDev(), win->window_width, win->window_height, debugTextTexNumber, spriteCommon);

	DebugText debugtext_eye;

	const int debugTextTexNumber2 = 3;

	SpriteCommonLoadTexture(spriteCommon, debugTextTexNumber2, L"Resource/ASC_White.png", dxcommon->GetDev());

	debugtext_eye.debugTextInit(dxcommon->GetDev(), win->window_width, win->window_height, debugTextTexNumber2, spriteCommon);

	DebugText debugtext_minute;

	const int debugTextTexNumber3 = 4;

	SpriteCommonLoadTexture(spriteCommon, debugTextTexNumber3, L"Resource/ASC_White.png", dxcommon->GetDev());

	debugtext_minute.debugTextInit(dxcommon->GetDev(), win->window_width, win->window_height, debugTextTexNumber3, spriteCommon);

	DebugText debugtext_minute2;

	const int debugTextTexNumber4 = 5;

	SpriteCommonLoadTexture(spriteCommon, debugTextTexNumber4, L"Resource/ASC_White.png", dxcommon->GetDev());

	debugtext_minute2.debugTextInit(dxcommon->GetDev(), win->window_width, win->window_height, debugTextTexNumber4, spriteCommon);


	//音声読み込み
	SoundData soundData1 = SoundLoadWave("Resource/powerup05.wav");//speedUp
	SoundData soundData2 = SoundLoadWave("Resource/go_on_your_way.wav");//bgm
	SoundData soundData3 = SoundLoadWave("Resource/powerdown07.wav");//speeddown
	SoundData soundData4 = SoundLoadWave("Resource/tm02.wav");//goal
	//SoundPlayWaveLoop(xAudio2.Get(), soundData2);


	const int n = 3;
	const float topHeight = 3.0f;
	Vertex vertices[n + 2] = {	//頂点

	};
	for (int i = 0; i < n; i++)
	{
		vertices[i] = { {(float)(10.0 * sinf(XM_2PI / n * i) * ((float)win->window_height / win->window_width)), (float)(10.0 * cosf(XM_2PI / n * i)), 0.0f} ,{}, { 0.0f,0.0f } };
	}
	vertices[n] = { {0,0,0 } ,{}, { 0.0f,0.0f } };

	vertices[n + 1] = { {0.0f, 0.0f, -topHeight}, {}, { 0.0f,0.0f } };


	unsigned short indices[] =	//インデックス
	{
		1,0,3,

		2,1,3,

		0,2,3,

		0,1,4,

		1,2,4,

		2,0,4,



	};


#pragma endregion

#pragma region 法線

	//法線の計算
	for (int i = 0; i < _countof(indices) / 3; i++)
	{
		//三角形一つごとに計算していく
		//三角形のインデックスを取り出して、一時的な変数を入れる
		unsigned short n0 = indices[i * 3 + 0];
		unsigned short n1 = indices[i * 3 + 1];
		unsigned short n2 = indices[i * 3 + 2];

		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[n0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[n1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[n2].pos);

		//p0 -> p1ベクトル p0 -> p2ベクトル を計算 (減算)
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);

		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);

		//正規化(長さを1にする)
		normal = XMVector3Normalize(normal);

		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[n0].normal, normal);
		XMStoreFloat3(&vertices[n1].normal, normal);
		XMStoreFloat3(&vertices[n2].normal, normal);

	}
#pragma endregion 

#pragma region 生成(頂点バッファ インデックスバッファ)

	//頂点バッファ

		// 頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
		/*UINT sizeVB = static_cast<UINT>(sizeof(XMFLOAT3) * _countof(vertices));*/
	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * _countof(vertices));

	//// 頂点バッファの生成
	ComPtr<ID3D12Resource> vertBuff; // ID3D12Resource* vertBuff = nullptr;
	

	result = dxcommon->GetDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//GPUと座標を共有させる

	//XMFLOAT3* vertMap = nullptr;
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	// 全頂点に対して
	for (int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];   // 座標をコピー
	}

	// マップを解除
	vertBuff->Unmap(0, nullptr);


	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	/*vbView.StrideInBytes = sizeof(XMFLOAT3);*/
	vbView.StrideInBytes = sizeof(Vertex);

	ComPtr<ID3DBlob> vsBlob;// ID3DBlob* vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;//ID3DBlob* psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob;//ID3DBlob* errorBlob = nullptr; // エラーオブジェクト



//インデックスバッファ
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * _countof(indices));//インデックスの全体サイズ

	ComPtr<ID3D12Resource> indexBuff; // ID3D12Resource* indexBuff = nullptr;//インデックスバッファの設定


	result = dxcommon->GetDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	//GPU上のバッファに対応した仮想メモリを取得
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);

	//全インデックスに対して
	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];//インデックスをコピー
	}

	indexBuff->Unmap(0, nullptr);//つながりを解除


	//インデックスバッファビューの作成
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

#pragma endregion

#pragma region テクスチャ

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(
		L"Resource/mimikkyu.jpg",
		WIC_FLAGS_NONE,
		&metadata, scratchImg);

	TexMetadata metadata2{};
	ScratchImage scratchImg2{};

	result = LoadFromWICFile(
		L"Resource/red.png",
		WIC_FLAGS_NONE,
		&metadata2, scratchImg2);




	const Image* img = scratchImg.GetImage(0, 0, 0);
	const Image* img2 = scratchImg2.GetImage(0, 0, 0);

	const int texWidth = 256;//横方向ピクセル
	const int imageDataCount = texWidth * texWidth;//配列の要素数

	const int imageDataCount2 = texWidth * texWidth;//配列の要素数

	//画像イメージデータ配列
	XMFLOAT4* imagedata = new XMFLOAT4[imageDataCount];//*必ず後で解放する！！
	XMFLOAT4* imagedata2 = new XMFLOAT4[imageDataCount2];//*必ず後で解放する！！

	//全ピクセルの色を初期化する
	for (int i = 0; i < imageDataCount; i++)
	{
		if (i % 10 == 0)
		{
			imagedata[i].x = 1.0f;//R
			imagedata[i].y = 0.0f;//G
			imagedata[i].z = 0.0f;//B
			imagedata[i].w = 0.0f;//A
		}
		else
		{
			imagedata[i].x = 1.0f;//R
			imagedata[i].y = 0.0f;//G
			imagedata[i].z = 0.0f;//B
			imagedata[i].w = 1.0f;//A

		}

	}
	for (int i = 0; i < imageDataCount2; i++)
	{
		if (i % 10 == 0)
		{


			imagedata2[i].x = 1.0f;//R
			imagedata2[i].y = 0.0f;//G
			imagedata2[i].z = 0.0f;//B
			imagedata2[i].w = 0.0f;//A
		}
		else
		{

			imagedata2[i].x = 1.0f;//R
			imagedata2[i].y = 0.0f;//G
			imagedata2[i].z = 0.0f;//B
			imagedata2[i].w = 1.0f;//A
		}

	}



	//リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	CD3DX12_RESOURCE_DESC texresDesc2 = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata2.format,
		metadata2.width,
		(UINT)metadata2.height,
		(UINT16)metadata2.arraySize,
		(UINT16)metadata2.mipLevels
	);

	//テクスチャ用バッファの生成
	ComPtr<ID3D12Resource> texbuff = nullptr;
	result = dxcommon->GetDev()->CreateCommittedResource(//GPUリソースの生成
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff));

	ComPtr<ID3D12Resource> texbuff2 = nullptr;
	result = dxcommon->GetDev()->CreateCommittedResource(//GPUリソースの生成
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc2,
		D3D12_RESOURCE_STATE_GENERIC_READ,//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff2));


	//テクスチャバッファにデータ転送
	result = texbuff->WriteToSubresource(
		0,
		nullptr,							//全領域にコピー
		img->pixels,						//元データアドレス
		(UINT)img->rowPitch,				//1ラインサイズ
		(UINT)img->slicePitch);				//全サイズ
		//imagedata,
		//sizeof(XMFLOAT4) * texWidth,
		//sizeof(XMFLOAT4) * imageDataCount);

	result = texbuff2->WriteToSubresource(
		0,
		nullptr,							//全領域にコピー
		img2->pixels,						//元データアドレス
		(UINT)img2->rowPitch,				//1ラインサイズ
		(UINT)img2->slicePitch);				//全サイズ

	delete[] imagedata;					//元データ解放
	delete[] imagedata2;					//元データ解放


#pragma endregion

#pragma region 定数バッファ
	//定数バッファ
	struct ConstBufferData
	{
		XMFLOAT4 color;//色
		XMMATRIX mat;//3D変換行列
	};

	//
	const int OBJECT_NUM = 50;

	Object3d_1 object3ds[OBJECT_NUM];

	const int OBJECT_NUM2 = 50;

	Object3d_1 object3ds2[OBJECT_NUM2];

	//定数バッファビュー用のスペース
	const int constantBufferNum = 128;

	//ヒープ設定
	CD3DX12_HEAP_PROPERTIES cbheapprop{};//D3D12_HEAP_PROPERTIES cbheapprop{};
	cbheapprop.Type = D3D12_HEAP_TYPE_UPLOAD;




	//定数バッファ用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> basicDescHeap;// ID3D12DescriptorHeap* basicDescHeap = nullptr;

	//設定構造
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダーから見える
	descHeapDesc.NumDescriptors = constantBufferNum + 2;//定数バッファの数 + 1はsrv分テクスチャふやすなら+値をかえる

	//デスクリプタヒープの生成
	result = dxcommon->GetDev()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));//生成

	Object3d::StaticInitialize(dxcommon->GetDev(), win->window_width, win->window_height);
	Object3d* object3d_camera = nullptr;

	Model* model = Model::LoadFromOBJ("box");
	Model* model1 = Model::LoadFromOBJ("car1");
	//Model* model2 = Model::LoadFromOBJ("box2");
	Model* model3 = Model::LoadFromOBJ("home3-0");
	Model* model4 = Model::LoadFromOBJ("ene-0");
	Model* model5 = Model::LoadFromOBJ("road");
	Model* model6 = Model::LoadFromOBJ("dash");
	Model* model7 = Model::LoadFromOBJ("home_white");
	Model* model8 = Model::LoadFromOBJ("home_long");
	Model* model9 = Model::LoadFromOBJ("glass");
	Model* model10 = Model::LoadFromOBJ("box_player");
	Model* model11 = Model::LoadFromOBJ("box_enemy");
	Model* model12 = Model::LoadFromOBJ("box2");
	Model* model13 = Model::LoadFromOBJ("box_select");

	Object3d* box_select = box_select->Create();
	box_select->SetModel(model12);
		box_select->scale.x = 3;
		box_select->scale.y = 3;
		box_select->scale.z = 3;
		box_select->SetPosition({ -1,0,-1 });

Object3d* box_player = box_player->Create();
		box_player->SetModel(model10);
		box_player->scale.x = 3;
		box_player->scale.y = 3;
		box_player->scale.z = 3;
		box_player->SetPosition({ -1,100,-1 });

		Object3d* box_enemy = box_enemy->Create();
		box_enemy->SetModel(model11);
		box_enemy->scale.x = 3;
		box_enemy->scale.y = 3;
		box_enemy->scale.z = 3;
		box_enemy->SetPosition({ 11,12,23 });

		Object3d* box_enemy2 = box_enemy2->Create();
		box_enemy2->SetModel(model11);
		box_enemy2->scale.x = 3;
		box_enemy2->scale.y = 3;
		box_enemy2->scale.z = 3;
		box_enemy2->SetPosition({ -25,12,23 });

//1
	Object3d* box = box->Create();
			  box->SetModel(model);
			  box->scale.x = 3;
			  box->scale.y = 3;
			  box->scale.z = 3;
			  box->SetPosition({ -25,0,-25 });

	Object3d* box2 = box2->Create();
			  box2->SetModel(model);
			  box2->scale.x = 3;
			  box2->scale.y = 3;
			  box2->scale.z = 3;
			  box2->SetPosition({ -13,0,-25 });
	Object3d* box3 = box3->Create();
			  box3->SetModel(model);
			  box3->scale.x = 3;
			  box3->scale.y = 3;
			  box3->scale.z = 3;
			  box3->SetPosition({ -1,0,-25 });
	Object3d* box4 = box4->Create();
			  box4->SetModel(model);
			  box4->scale.x = 3;
			  box4->scale.y = 3;
			  box4->scale.z = 3;
			  box4->SetPosition({ 11,0,-25 });
	Object3d* box5 = 
			  box5->Create();
			  box5->SetModel(model);
			  box5->scale.x = 3;
			  box5->scale.y = 3;
			  box5->scale.z = 3;
			  box5->SetPosition({ 23,0,-25 });
//2列目
	Object3d* box6 =
			  box6->Create();
			  box6->SetModel(model);
			  box6->scale.x = 3;
			  box6->scale.y = 3;
			  box6->scale.z = 3;
			  box6->SetPosition({ -25,0,-13 });

	Object3d* box7 =
		      box7->Create();
			  box7->SetModel(model);
			  box7->scale.x = 3;
			  box7->scale.y = 3;
			  box7->scale.z = 3;
			  box7->SetPosition({ -13,0,-13 });

    Object3d* box8 =
			  box8->Create();
			  box8->SetModel(model);
			  box8->scale.x = 3;
			  box8->scale.y = 3;
			  box8->scale.z = 3;
			  box8->SetPosition({ -1,0,-13 });

   Object3d*  box9 =
			  box9->Create();
			  box9->SetModel(model);
			  box9->scale.x = 3;
			  box9->scale.y = 3;
			  box9->scale.z = 3;
			  box9->SetPosition({ 11,0,-13 });

    Object3d* box10 =
			  box10->Create();
			  box10->SetModel(model);
			  box10->scale.x = 3;
			  box10->scale.y = 3;
			  box10->scale.z = 3;
			  box10->SetPosition({ 23,0,-13 });
//3
	Object3d* box11 =
			  box11->Create();
			  box11->SetModel(model);
			  box11->scale.x = 3;
			  box11->scale.y = 3;
			  box11->scale.z = 3;
			  box11->SetPosition({ -25,0,-1 });

	Object3d* box12 =
			  box12->Create();
			  box12->SetModel(model);
			  box12->scale.x = 3;
			  box12->scale.y = 3;
			  box12->scale.z = 3;
			  box12->SetPosition({ -13,0,-1 });

			  Object3d* box13 =
			  box13->Create();
			  box13->SetModel(model);
			  box13->scale.x = 3;
			  box13->scale.y = 3;
			  box13->scale.z = 3;
			  box13->SetPosition({ -1,0,-1 });

			  Object3d* box14 =
			  box14->Create();
			  box14->SetModel(model);
			  box14->scale.x = 3;
			  box14->scale.y = 3;
			  box14->scale.z = 3;
			  box14->SetPosition({ 11,0,-1 });

			  Object3d* box15 =
				  box15->Create();
			  box15->SetModel(model);
			  box15->scale.x = 3;
			  box15->scale.y = 3;
			  box15->scale.z = 3;
			  box15->SetPosition({ 23,0,-1 });
//4
			  Object3d* box16 =
				  box16->Create();
			  box16->SetModel(model);
			  box16->scale.x = 3;
			  box16->scale.y = 3;
			  box16->scale.z = 3;
			  box16->SetPosition({ -25,0,11 });

			  Object3d* box17 =
				  box17->Create();
			  box17->SetModel(model);
			  box17->scale.x = 3;
			  box17->scale.y = 3;
			  box17->scale.z = 3;
			  box17->SetPosition({ -13,0,11 });

			  Object3d* box18 =
				  box18->Create();
			  box18->SetModel(model);
			  box18->scale.x = 3;
			  box18->scale.y = 3;
			  box18->scale.z = 3;
			  box18->SetPosition({ -1,0,11 });

			  Object3d* box19 =
				  box19->Create();
			  box19->SetModel(model);
			  box19->scale.x = 3;
			  box19->scale.y = 3;
			  box19->scale.z = 3;
			  box19->SetPosition({ 11,0,11 });

			  Object3d* box20 =
				  box20->Create();
			  box20->SetModel(model);
			  box20->scale.x = 3;
			  box20->scale.y = 3;
			  box20->scale.z = 3;
			  box20->SetPosition({ 23,0,11 });
//5
			  Object3d* box21 =
				  box21->Create();
			  box21->SetModel(model);
			  box21->scale.x = 3;
			  box21->scale.y = 3;
			  box21->scale.z = 3;
			  box21->SetPosition({ -25,0,23 });

			  Object3d* box22 =
				  box22->Create();
			  box22->SetModel(model);
			  box22->scale.x = 3;
			  box22->scale.y = 3;
			  box22->scale.z = 3;
			  box22->SetPosition({ -13,0,23 });

			  Object3d* box23 =
				  box23->Create();
			  box23->SetModel(model);
			  box23->scale.x = 3;
			  box23->scale.y = 3;
			  box23->scale.z = 3;
			  box23->SetPosition({ -1,0,23 });

			  Object3d* box24 =
				  box24->Create();
			  box24->SetModel(model);
			  box24->scale.x = 3;
			  box24->scale.y = 3;
			  box24->scale.z = 3;
			  box24->SetPosition({ 11,0,23 });

			  Object3d* box25 =
				  box25->Create();
			  box25->SetModel(model);
			  box25->scale.x = 3;
			  box25->scale.y = 3;
			  box25->scale.z = 3;
			  box25->SetPosition({ 23,0,23 });
	//自機
	Object3d* object3d_player = object3d_player->Create();

	object3d_player->SetModel(model1);

	object3d_player->SetPosition({ 0,-10,-20 });
	object3d_player->rotation.y = -90;
	//道
	Object3d* object3d_roadCenter = object3d_roadCenter->Create();
	Object3d* object3d_roadCenter2 = object3d_roadCenter2->Create();
	Object3d* object3d_roadCenter3 = object3d_roadCenter3->Create();

	object3d_roadCenter->SetModel(model5);
	object3d_roadCenter2->SetModel(model5);
	object3d_roadCenter3->SetModel(model5);

	object3d_roadCenter->SetPosition({ 0,-30,0 });
	object3d_roadCenter->scale.z = 200;
	object3d_roadCenter->scale.x = 20;

	object3d_roadCenter2->SetPosition({ 0,-30,+800 });//z 400
	object3d_roadCenter2->scale.z = 200;
	object3d_roadCenter2->scale.x = 20;

	object3d_roadCenter3->SetPosition({ 0,-30,+1600 });//z 400
	object3d_roadCenter3->scale.z = 200;
	object3d_roadCenter3->scale.x = 20;
	//家
	Object3d* object3d_home = object3d_home->Create();
	Object3d* object3d_homeWhite = object3d_homeWhite->Create();
	Object3d* object3d_homeLong = object3d_homeLong->Create();

	object3d_home->SetModel(model3);
	object3d_homeWhite->SetModel(model7);
	object3d_homeLong->SetModel(model8);

	object3d_home->SetPosition({ -70,-10,0 });
	object3d_home->rotation.y = 270;
	object3d_homeWhite->SetPosition({ 90,-30,30 });
	object3d_homeWhite->rotation.y = 90;
	object3d_homeLong->SetPosition({ -90,-30,60 });
	object3d_homeLong->rotation.y = 270;
	//敵
	Object3d* object3d_eneCenter = object3d_eneCenter->Create();

	object3d_eneCenter->SetModel(model4);

	object3d_eneCenter->SetPosition({ 0,-15,+200 });

	Object3d* object3d_eneLeft = object3d_eneLeft->Create();

	object3d_eneLeft->SetModel(model4);

	object3d_eneLeft->SetPosition({ -35,-15,+400 });

	Object3d* object3d_eneRight = object3d_eneRight->Create();

	object3d_eneRight->SetModel(model4);

	object3d_eneRight->SetPosition({ 35,-15,+400 });

	//背景
	Object3d* object3d_glass = object3d_glass->Create();
	Object3d* object3d_glass2 = object3d_glass2->Create();

	object3d_glass->SetModel(model9);
	object3d_glass2->SetModel(model9);

	object3d_glass->SetPosition({ -50,-30,0 });
	object3d_glass->scale.x = 200;
	object3d_glass->scale.z = 200;

	object3d_glass2->SetPosition({ -50,-30,780 });
	object3d_glass2->scale.x = 200;
	object3d_glass2->scale.z = 200;


	//ダッシュ
	Object3d* object3d_dash = object3d_dash->Create();

	object3d_dash->SetModel(model6);

	object3d_dash->rotation.y = 180;

	object3d_dash->SetPosition({ 0,-15,400 });

	Object3d* object3d_dash2 = object3d_dash->Create();

	object3d_dash2->SetModel(model6);

	object3d_dash2->rotation.y = 180;

	object3d_dash2->SetPosition({ -35,-15,1600 });

	Object3d* object3d_goal = object3d_goal->Create();

	object3d_goal->SetPosition({ 0,-60,13000 });
	object3d_goal->SetModel(model10);
	object3d_goal->scale.x = 30;
	object3d_goal->scale.y = 25;
	//object3d_goal->rotation.y = 45;


	//当たり判定　球
	Sphere sphere;
	//当たり判定　平面
	Plane plane;
	//当たり判定　三角形
	Triangle triangle_ene;
	Triangle triangle_ene2;
	Triangle triangle_ene3;
	Triangle triangle_ene4;
	

	Triangle triangle_dash;
	Triangle triangle_dash2;
	//当たり判定　レイ
	Ray ray;

	//球の初期値を設定
	sphere.center = XMVectorSet(box_player->position.x + 6, box_player->position.y - 6, box_player->position.z + 6, 1);//中心点座標
	sphere.radius = 4.0f;//半径

	//平面の初期値を設定
	plane.normal = XMVectorSet(0, 1, 0, 0);//法線ベクトル
	plane.distance = 0.0f;//原点(0,0,0)からの距離






	//レイの初期値を設定
	ray.start = XMVectorSet(0, 1, 0, 1);//原点やや上
	ray.dir = XMVectorSet(0, -1, 0, 0);//下向き


	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
			constantBufferNum,
			dxcommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV =
		CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
			constantBufferNum,
			dxcommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV2 =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(),
			constantBufferNum + 1,
			dxcommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV2 =
		CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(),
			constantBufferNum + 1,
			dxcommon->GetDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));





	//シェーダーリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};//設定構造体
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//RGBA
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};//設定構造体
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//RGBA
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = 1;



	//ヒープのbuffernum番目にシェーダーリソースビューを生成
	dxcommon->GetDev()->CreateShaderResourceView(texbuff.Get(),//ビューと関連付けるバッファ
		&srvDesc,//テクスチャ設定情報
		cpuDescHandleSRV);

	//ヒープのbuffernum+1番目にシェーダーリソースビューを生成
	dxcommon->GetDev()->CreateShaderResourceView(texbuff2.Get(),//ビューと関連付けるバッファ
		&srvDesc2,//テクスチャ設定情報
		cpuDescHandleSRV2);



	D3D12_STATIC_SAMPLER_DESC samplerDesc{};

	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し(タイリング)

	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し(タイリング)

	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し(タイリング)

	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒

	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)

	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミニマップ最大値

	samplerDesc.MinLOD = 0.0f;//ミニマップ最小値

	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ見える




#pragma endregion


	PipelineSet object3dPipelineSet = Create3dpipe(dxcommon->GetDev());
	PipelineSet spritePipelineSet = CreateSprite2dpipe(dxcommon->GetDev());
	//描画初期化処理　ここまで



#pragma region シェーダーの色,初期化


	float xmfloat4_r = 1.0f;
	float xmfloat4_b = 1.0f;
	float xmfloat4_g = 1.0f;
	float xmfloat4_a = 1.0f;

	float speed = 3.0f;

	float ene_x = box_enemy->position.x;
	float ene_y = box_enemy->position.y;
	float ene_z = box_enemy->position.z;

	float ene_x2 = box_enemy->position.x;
	float ene_y2 = box_enemy->position.y;
	float ene_z2 = box_enemy->position.z;

	float ene_x3 = box_enemy->position.x;
	float ene_y3 = box_enemy->position.y;
	float ene_z3 = box_enemy->position.z;

	float ene_x4 = box_enemy->position.x;
	float ene_y4 = box_enemy->position.y;
	float ene_z4 = box_enemy->position.z;

	float dash_x = object3d_dash->position.x;
	float dash_z = object3d_dash->position.z;
	float dash2_x = object3d_dash2->position.x;
	float dash2_z = object3d_dash2->position.z;

	char moji[64];
	char moji2[64];
	char moji3[64];

	int minute_x = 0;
	int minute_y = 0;
	int secound_x = 150;
	int secound_y = 0;
	int secound2_x = 100;
	int secound2_y = 0;



	int atattaflag = 0;
	int timer_dash = 60;

	int timer = 60;
	int secound = 0;
	int secound2 = 0;
	int minute = 0;

	int centerCount = 0;
	int leftCount = 0;
	int rightCount = 0;

	int gameScene = 1;

	int setEyeflag = 0;

	int moveflag = 0;

	float player_place_x = 0;
	float player_place_z = 0;

	int enemyflag = 0;
	

	int timer2 = 30;
	int timer3 = 30;



	sprite.rotation = { 0,0,0 };
	sprite.position = { 640, 360, 0 };
	//sprite.size = { 150.0f,150.0f };
	sprite.texSize = { 1280.0f,720.0f };


	sprite1.rotation = { 0,0,0 };
	sprite1.position = { 640, 360, 0 };
	//sprite1.size = { 1280.0f,720.0f };
	sprite1.texSize = { 1280.0f,720.0f };



	//sprite1.isInvisible = true;


	//sprite.isFlagX = true;


	SpriteTransVertexBuffer(sprite, spriteCommon);
	SpriteTransVertexBuffer(sprite1, spriteCommon);
	
#pragma endregion


	while (true)  // ゲームループ
	{
		//// メッセージがある？
		

		//// ?ボタンで終了メッセージが来たらゲームループを抜ける

		if (win->ProcessMessage()) {
			break;
		}

		// DirectX毎フレーム処理　ここから



		//キーボード情報の取得開始
		//全キーの入力状態を取得する
		input->Update();

		//キーが入力されているときの処理例
		/*if (input->isKeyTrigger(DIK_0)) {
			OutputDebugStringA("Hit 0\n");
		}*/

		SpriteUpdate(sprite, spriteCommon);

		SpriteUpdate(sprite1, spriteCommon);
		
		 if (gameScene == 1)
		{
			 sphere.center = XMVectorSet(box_player->position.x + 6.0f, box_player->position.y - 6.0f, box_player->position.z - 6.0f, 1);//中心点座標
			 sphere.radius = 5.0f;//半径

			 if (input->isKeyTrigger(DIK_Q))
			 {
				 moveflag = 15;
			 }
			 if (moveflag == 0)
			 {
				 if (input->isKeyTrigger(DIK_W))
				 {
					 box_select->position.z += 12.0f;
					 if (box_select->position.z >= 24.0f)
					 {
						 box_select->position.z = 23.0f;
					 }
					 //sphere.center += moveZ;

				 }
				 if (input->isKeyTrigger(DIK_S))
				 {
					 box_select->position.z -= 12.0f;
					 if (box_select->position.z <= -26.0f)
					 {
						 box_select->position.z = -25.0f;
					 }
					 // sphere.center -= moveZ;
				 }
				 if (input->isKeyTrigger(DIK_A))
				 {
					 box_select->position.x -= 12.0f;
					 if (box_select->position.x <= -26.0f)
					 {
						 box_select->position.x = -25.0f;
					 }
					 //sphere.center -= moveX2;
				 }
				 if (input->isKeyTrigger(DIK_D))
				 {
					 box_select->position.x += 12.0f;
					 if (box_select->position.x >= 24.0f)
					 {
						 box_select->position.x = 23.0f;
					 }
					 //sphere.center += moveX2;
				 }

				 if (input->isKeyTrigger(DIK_SPACE))
				 {
					 if (box_select->position.x == box_enemy->position.x &&
						 box_select->position.z == box_enemy->position.z &&
						 box_select->position.x == box_enemy2->position.x &&
						 box_select->position.z == box_enemy2->position.z)
					 {

					 }
					 else
					 {
						 box_player->position.x = box_select->position.x;
						 box_player->position.y = box_select->position.y + 12;
						 box_player->position.z = box_select->position.z;
						 player_place_x = box_select->position.x;
						 player_place_z = box_select->position.z;
						 moveflag = 1;
					 }
					 //sphere.center += moveX2;
				 }
			 }
			 else if (moveflag == 1)
			 {
				 if (input->isKeyTrigger(DIK_W))
				 {
					 if (box_player->position.x <= box_select->position.x &&
						 box_player->position.x + 11 >= box_select->position.x)
					 {
						 box_select->position.z += 12.0f;
						 if (box_select->position.z >= 24.0f)
						 {
							 box_select->position.z = 23.0f;
						 }
					 }
					 
					

				 }
				 if (input->isKeyTrigger(DIK_S))
				 {
					 if (box_player->position.x <= box_select->position.x &&
						 box_player->position.x + 11 >= box_select->position.x)
					 {
						 box_select->position.z -= 12.0f;
						 if (box_select->position.z <= -26.0f)
						 {
							 box_select->position.z = -25.0f;
						 }
					 }
					
				 }
				 if (input->isKeyTrigger(DIK_A))
				 {
					 if (box_player->position.z <= box_select->position.z &&
						 box_player->position.z + 11 >= box_select->position.z)
					 {
						 box_select->position.x -= 12.0f;
						 if (box_select->position.x <= -26.0f)
						 {
							 box_select->position.x = -25.0f;
						 }
					 }
					 
				 }
				 if (input->isKeyTrigger(DIK_D))
				 {
					 if (box_player->position.z <= box_select->position.z &&
						 box_player->position.z + 11 >= box_select->position.z)
					 {
						 box_select->position.x += 12.0f;
						 if (box_select->position.x >= 24.0f)
						 {
							 box_select->position.x = 23.0f;
						 }
					 }
					
				 }

				 if (input->isKeyTrigger(DIK_SPACE))
				 {
					 //今いる地点を記録
					 player_place_x = box_player->position.x;
					 player_place_z = box_player->position.z;
					 
					 //タイマー設置(段階的に動かすため)
					 timer2 = 60;
					 timer3 = 120;
					 enemyflag++;

					 //zが一緒ならx方向 xが一緒ならz方向に動く
					 if (box_player->position.z == box_select->position.z)
					 {
						 moveflag = 3;//x方向に動かす
					 }
					 if (box_player->position.x == box_select->position.x)
					 {
						 moveflag = 2;//z方向に動かす
					 }
					
						 
						
					
					
				 }
				 if (input->isKeyTrigger(DIK_RETURN))
				 {
					 timer2 = 60;
					 moveflag = 5;
				 }

			 }
			 else if (moveflag == 2)
			 {
			 //上に動く
			 if (box_select->position.z >= box_player->position.z &&
				 player_place_x == box_player->position.x)
			 {
				 timer2--;
				 timer3--;
				 if (timer2 <= 0)
				 {
					 box_player->position.z = box_player->position.z + 12;


					 if (box_select->position.z <= box_player->position.z)
					 {

						 box_player->position.z = box_select->position.z;
						 if (timer3 <= 0)
						 {
							 moveflag = 1;
							 timer3 = 120;
						 }
					 }
					 timer2 = 60;
				 }

			 }
			 //下に動く
			 else if (box_select->position.z <= box_player->position.z)
			 {
				 timer2--;
				 timer3--;
				 if (timer2 <= 0)
				 {
					 box_player->position.z = box_player->position.z - 12;
					 if (box_select->position.z >= box_player->position.z)
					 {
						 box_player->position.z = box_select->position.z;
						 if (timer3 <= 0)
						 {
							 moveflag = 1;
							 timer3 = 120;
						 }
					 }
					 timer2 = 60;
				 }

			 }


			 }
			 else if (moveflag == 3)
			 {
			 //右に動く
				 if (box_select->position.x >= box_player->position.x &&
					 player_place_z == box_player->position.z)
				 {
					 timer2--;
					 timer3--;
					 if (timer2 <= 0)
					 {
						 box_player->position.x = box_player->position.x + 12;
						 if (box_select->position.x <= box_player->position.x)
						 {
							 box_player->position.x = box_select->position.x;
							 

							 if (timer3 <= 0)
							 {
								 moveflag = 1;
								 timer3 = 200;
							 }
							 
						 }
						 timer2 = 60;
					 }
					 else if (timer2 <= 0 && box_select->position.x <= box_player->position.x)
					 {
						 box_player->position.x = box_select->position.x;
						 moveflag = 5;
					 }
					 

				 }

				 //左に動く
				 else if (box_select->position.x <= box_player->position.x &&
					 player_place_z == box_player->position.z)
				 {
					 timer2--;
					 timer3--;
					 if (timer2 <= 0)
					 {
						 box_player->position.x = box_player->position.x - 12;
						 
						 if (box_select->position.x >= box_player->position.x)
						 {
							 timer3--;
							 box_player->position.x = box_select->position.x;
							 if (timer3 <= 0)
							 {
								 moveflag = 1;
								 timer3 = 120;
							 }
						 }
						 timer2 = 60;
					 }
					 
				 }
				
			}
			 else if (moveflag == 4)
			 {
			 //敵の動く回数を決める
			 float box_player_x = box_player->position.x;
			 float box_player_z = box_player->position.z;
			 float box_oldplayer_x = player_place_x;
			 float box_oldplayer_z = player_place_z;
			 if (box_player_x > 0)
			 {
				 box_player_x *= -1.0f;
			 }
			 if (box_player_z > 0)
			 {
				 box_player_z *= -1.0f;
			 }
			 if (box_oldplayer_x > 0)
			 {
				 box_oldplayer_x *= -1.0f;
			 }
			 if (box_oldplayer_z > 0)
			 {
				 box_oldplayer_z *= -1.0f;
			 }

			 enemyflag = ((box_player_x + box_oldplayer_x) / 12) + ((box_player_z + box_oldplayer_z) / 12);
			 
			 if (enemyflag < 0)
			 {
				 enemyflag *= -1;
				 moveflag = 5;
				 timer2 = 60;
				 timer3 = 60;
			 }
			 else
			 {
				 moveflag = 5;
			 }
			 
			}
			
			 if (moveflag == 5)
			 {
				 //敵の移動
				 if (enemyflag == 4)
				 {
					 timer2--;
					 //敵の目の前に自機があるなら
					 if (box_enemy->position.z - 12 == box_player->position.z &&
						 box_enemy->position.x  == box_player->position.x)
					 {
						 if(timer2 <= 0)
						 { 
						 //xを引くと画面外なら足す
						 if (box_enemy->position.x - 12 <= box->position.x)
						 {
							 box_enemy->position.x = box_enemy->position.x + 12;
							 // enemyflag = 1;
							 timer2 = 60;
							 enemyflag = 3;
						 }
						 else//xを引いて回避
						 {
							 box_enemy->position.x = box_enemy->position.x - 12;
							 timer2 = 60;
							 enemyflag = 3;
						 }
						 }

					 }
					 else//前に進む
					 {
						 
						 if (timer2 <= 0)
						 {

							 box_enemy->position.z = box_enemy->position.z - 12;
							 timer2 = 60;
							 enemyflag = 3;
						 }
					 }

					 

				 }
				 if (enemyflag == 3)//上のと一緒
				 {
					 timer2--;
					 if (box_enemy->position.z - 12 == box_player->position.z &&
						 box_enemy->position.x  == box_player->position.x)
					 {
						 if (timer2 <= 0)
						 {
							 if (box_enemy->position.x - 12 <= box->position.x)
							 {
								 box_enemy->position.x = box_enemy->position.x + 12;
								 // enemyflag = 1;
								 timer2 = 60;
								 enemyflag = 2;
							 }
							 else
							 {
								 box_enemy->position.x = box_enemy->position.x - 12;
								 timer2 = 60;
								 enemyflag = 2;
							 }
						 }
					 }
					 else
					 {
						 if (timer2 <= 0)
						 {
							 box_enemy->position.z = box_enemy->position.z - 12;
							 timer2 = 60;
							 enemyflag = 2;
						 }
					 }
				 
				}
			 if (enemyflag == 2)//上のと一緒
			 {
				 timer2--;
				if (box_enemy->position.z - 12 == box_player->position.z &&
					box_enemy->position.x  == box_player->position.x)
				 {
					if (timer2 <= 0)
					{
						if (box_enemy->position.x - 12 <= box->position.x)
						{
							box_enemy->position.x = box_enemy->position.x + 12;
							// enemyflag = 1;
							timer2 = 60;
							enemyflag = 1;
						}
						else
						{
							box_enemy->position.x = box_enemy->position.x - 12;
							timer2 = 60;
							enemyflag = 1;
						}
					}

					 //enemyflag = 1;
				 }
				else
				{
					if (timer2 <= 0)
					{
						box_enemy->position.z = box_enemy->position.z - 12;
						timer2 = 60;
						enemyflag = 1;
					}
				}
				 
				 
			 }
			 if (enemyflag == 1)//上のと一緒
			 {
				 timer2--;
				 if (box_enemy->position.z - 12 == box_player->position.z &&
					 box_enemy->position.x  == box_player->position.x)
				 {
					 if (timer2 <= 0)
					 {
						 if (box_enemy->position.x - 12 <= box->position.x)
						 {
							 box_enemy->position.x = box_enemy->position.x + 12;
							 timer2 = 60;
							 // enemyflag = 1;
							 enemyflag = 0;
						 }
						 else
						 {
							 box_enemy->position.x = box_enemy->position.x - 12;
							 timer2 = 60;
							 enemyflag = 0;
						 }
					 }
					
				 }
				 else
				 {
					 box_enemy->position.z = box_enemy->position.z - 12;
					 timer2 = 60;
					 enemyflag = 0;
				 }
			 }
			 if (enemyflag == 0)//行動が終わったのでセレクトに戻す
			 {
				 moveflag = 1;
			 }
			 
			 }
			 else if (moveflag == 15)//デバッグ用
			 {

				 if (input->isKeyTrigger(DIK_R))
				 {
					 box_player->position.x = box_select->position.x;
					 box_player->position.y = box_select->position.y + 12;
					 box_player->position.z = box_select->position.z;
				 }

				 if (input->isKeyTrigger(DIK_W))
				 {

					 box_player->position.z += speed;
				 }

				 if (input->isKey(DIK_A))
				 {

					 box_player->position.x -= speed;
				 }
				 if (input->isKey(DIK_S))
				 {
					 box_player->position.z -= speed;

				 }
				 if (input->isKey(DIK_D))
				 {
					 box_player->position.x += speed;

				 }
			 }

			 //勝敗
			 if (box_enemy->position.x <= box->position.x-6.0f)
			 {
				 debugtext.Print(spriteCommon, "KATI", 200, 200);
			 }
			 else if (box_enemy->position.x >= box5->position.x + 6.0f)
			 {
				 debugtext.Print(spriteCommon, "KATI", 200, 200);
			 }
			 else if (box_enemy->position.z <= box->position.z - 6)
			 {
				 debugtext.Print(spriteCommon, "MAKE", 200, 200);
			 }



			sprite.position.x = 2000;
			sprite1.position.x = 2000;

			//eye 13000 
			//object3d_camera->CameraMoveVector({ 0,0,speed });
			/*eye.z++;
			target.z++;
			matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));*/

			XMFLOAT3 eye = Object3d::GetEye();

			XMVECTOR moveZ = XMVectorSet(0, 0, speed, 0);

			XMVECTOR moveX = XMVectorSet(30.0f, 0, 0, 0);

			XMVECTOR moveX2 = XMVectorSet(1.0f, 0, 0, 0);//debug

			//三角形の初期値を設定
			triangle_ene.p0 = XMVectorSet(box_enemy->position.x, box_enemy->position.y - 5.0f, box_enemy->position.z - 10.0f, 1);//左手前
			triangle_ene.p1 = XMVectorSet(box_enemy->position.x + 5.0f , box_enemy->position.y - 5.0f, box_enemy->position.z, 1);//左奥
			triangle_ene.p2 = XMVectorSet(box_enemy->position.x + 10.0f, box_enemy->position.y - 5.0f, box_enemy->position.z -10.0f, 1);//右手前
			triangle_ene.normal = XMVectorSet(0.0f, 4.0f, 0.0f, 0);//上向き

			triangle_ene2.p0 = XMVectorSet(box_enemy2->position.x, box_enemy2->position.y - 5.0f, box_enemy2->position.z - 10.0f, 1);//左手前
			triangle_ene2.p1 = XMVectorSet(box_enemy2->position.x + 5.0f, box_enemy2->position.y - 5.0f, box_enemy2->position.z, 1);//左奥
			triangle_ene2.p2 = XMVectorSet(box_enemy2->position.x + 10.0f, box_enemy2->position.y - 5.0f, box_enemy2->position.z - 10.0f, 1);//右手前
			triangle_ene2.normal = XMVectorSet(0.0f, 4.0f, 0.0f, 0);//上向き

			triangle_ene3.p0 = XMVectorSet(ene_x3 - 1.0f, ene_y3, ene_z3, 1);//左手前
			triangle_ene3.p1 = XMVectorSet(ene_x3, ene_y3, ene_z3 + 5.0f, 1);//左奥
			triangle_ene3.p2 = XMVectorSet(ene_x3 + 1.0f, ene_y3, ene_z3, 1);//右手前
			triangle_ene3.normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0);//上向き


			triangle_dash.p0 = XMVectorSet(dash_x - 5.5f, object3d_dash->position.y, dash_z - 10.5f, 1);//左手前
			triangle_dash.p1 = XMVectorSet(dash_x - 5.5f, object3d_dash->position.y, dash_z + 10.5f, 1);//左奥
			triangle_dash.p2 = XMVectorSet(dash_x + 10.5f, object3d_dash->position.y, dash_z - 10.5f, 1);//右手前
			triangle_dash.normal = XMVectorSet(0.0f, 0.1f, 0.0f, 0);//上向き

			triangle_dash2.p0 = XMVectorSet(dash2_x - 5.5f, object3d_dash2->position.y, dash2_z - 10.5f, 1);//左手前
			triangle_dash2.p1 = XMVectorSet(dash2_x - 5.5f, object3d_dash2->position.y, dash2_z + 10.5f, 1);//左奥
			triangle_dash2.p2 = XMVectorSet(dash2_x + 10.5f, object3d_dash2->position.y, dash2_z - 10.5f, 1);//右手前
			triangle_dash2.normal = XMVectorSet(0.0f, 0.1f, 0.0f, 0);//上向き





			bool hit_dash = Collision::CheckSphere2Triangle(sphere, triangle_dash);
			bool hit_dash2 = Collision::CheckSphere2Triangle(sphere, triangle_dash2);
			bool hit = Collision::CheckSphere2Triangle(sphere, triangle_ene);
			bool hit_left = Collision::CheckSphere2Triangle(sphere, triangle_ene2);
			bool hit_right = Collision::CheckSphere2Triangle(sphere, triangle_ene3);


			sprintf_s(moji, "%d", timer2);
			sprintf_s(moji2, "%d", timer3);
			sprintf_s(moji3, "%d", enemyflag);
		

	//当たり判定


	

			if (hit )
			{
				if (moveflag == 3 || moveflag == 1)
				{
					//debugtext.Print(spriteCommon, "HELLO", 0, 0, 5.0f);
					if (player_place_x >= box_enemy->position.x)
					{
						if (box_enemy->position.x -12 == box_enemy2->position.x)
						{
							box_enemy2->position.x = box_enemy2->position.x - 12;
						}
						box_enemy->position.x = box_enemy->position.x - 12;

						if (box_enemy->position.x <= -27)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
					if (player_place_x <= box_enemy->position.x)
					{
						if (box_enemy->position.x + 12 == box_enemy2->position.x)
						{
							box_enemy2->position.x = box_enemy2->position.x + 12;
						}
						box_enemy->position.x = box_enemy->position.x + 12;
						if (box_enemy->position.x >= 23)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
				}
				else if (moveflag == 2 || moveflag == 1)
				{
					if (player_place_z >= box_enemy->position.z)
					{
						if (box_enemy->position.z - 12 == box_enemy2->position.z)
						{
							box_enemy2->position.z = box_enemy2->position.z - 12;
						}
						box_enemy->position.z = box_enemy->position.z - 12;
						if (box_enemy->position.z <= -25)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
					if (player_place_z <= box_enemy->position.z)
					{
						if (box_enemy->position.z + 12 == box_enemy2->position.z)
						{
							box_enemy2->position.z = box_enemy2->position.z + 12;
						}
						box_enemy->position.z = box_enemy->position.z + 12;
						if (box_enemy->position.z >= 23)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
				}
				//SoundPlayWave(xAudio2.Get(), soundData3);
				//object3d_eneCenter->scale.y = -2;
				

			}

			if (hit_left)
			{
				if (moveflag == 3 || moveflag == 1)
				{
					//debugtext.Print(spriteCommon, "HELLO", 0, 0, 5.0f);
					if (player_place_x >= box_enemy2->position.x)
					{
						if (box_enemy2->position.x - 12 == box_enemy->position.x)
						{
							box_enemy->position.x = box_enemy->position.x - 12;
						}
						box_enemy2->position.x = box_enemy2->position.x - 12;

						if (box_enemy2->position.x <= -27)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
					if (player_place_x <= box_enemy2->position.x)
					{
						if (box_enemy2->position.x + 12 == box_enemy->position.x)
						{
							box_enemy->position.x = box_enemy->position.x + 12;
						}
						box_enemy2->position.x = box_enemy2->position.x + 12;
						if (box_enemy2->position.x >= 23)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
				}
				else if (moveflag == 2 || moveflag == 1)
				{
					if (player_place_z >= box_enemy2->position.z)
					{
						if (box_enemy2->position.z - 12 == box_enemy->position.z)
						{
							box_enemy->position.z = box_enemy->position.z - 12;
						}
						box_enemy2->position.z = box_enemy2->position.z - 12;
						if (box_enemy2->position.z <= -25)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
					if (player_place_z <= box_enemy2->position.z)
					{
						if (box_enemy2->position.z + 12 == box_enemy->position.z)
						{
							box_enemy->position.z = box_enemy->position.z + 12;
						}
						box_enemy2->position.z = box_enemy2->position.z + 12;
						if (box_enemy2->position.z >= 23)
						{
							debugtext.Print(spriteCommon, "AAA", 0, 0, 5.0f);
						}
					}
				}
				//SoundPlayWave(xAudio2.Get(), soundData3);
				//object3d_eneCenter->scale.y = -2;


			}

			//if (hit_left)
			//{

			//	//debugtext.Print(spriteCommon, "HELLO", 100, 0, 5.0f);
			//	SoundPlayWave(xAudio2.Get(), soundData3);
			//	//object3d_eneCenter->scale.y = -2;
			//	speed = 2.0f;

			//}

			//if (hit_right)
			//{

			//	//debugtext.Print(spriteCommon, "HELLO", 0, 0, 5.0f);
			//	SoundPlayWave(xAudio2.Get(), soundData3);
			//	//object3d_eneCenter->scale.y = -2;
			//	speed = 2.0f;

			//}

			//if (atattaflag == 0)
			//{
			//	if (hit_dash == true)
			//	{
			//		//	debugtext.Print(spriteCommon, "HELO", 0, 0, 5.0f);
			//		atattaflag = 1;
			//		speed += 2.0f;
			//		SoundPlayWave(xAudio2.Get(), soundData1);
			//		if (speed >= 8.0f)
			//		{
			//			speed = 7.0f;
			//		}
			//	}
			//	if (hit_dash2 == true)
			//	{
			//		//debugtext.Print(spriteCommon, "HELO", 0, 0, 5.0f);
			//		atattaflag = 1;
			//		speed += 2.0f;
			//		SoundPlayWave(xAudio2.Get(), soundData1);
			//		if (speed >= 8.0f)
			//		{
			//			speed = 7.0f;
			//		}
			//	}
			//}

			//if (atattaflag == 1)
			//{
			//	timer_dash--;
			//	if (timer_dash <= 0)
			//	{
			//		timer_dash = 60;
			//		atattaflag = 0;

			//	}
			//}

			//object3d_player->position.z += speed;
			//sphere.center += moveZ;
			/*	if (input->isKey(DIK_W))
				{
					object3d_player->position.z += 1.0f;
					sphere.center += moveZ;

				}
				if (input->isKey(DIK_S))
				{
					object3d_player->position.z -= 1.0f;
					sphere.center -= moveZ;
				}*/
				/*if (input->isKey(DIK_A))
				{
					object3d_player->position.x -= 1.0f;
					sphere.center -= moveX2;
				 }
				if (input->isKey(DIK_D))
				{
					object3d_player->position.x += 1.0f;
					sphere.center += moveX2;
				}*/
//			if (input->isKeyTrigger(DIK_A))
//			{
//				if (object3d_player->position.x <= -30.0f)
//				{
//					object3d_player->position.x = -30.0f;
//					//sphere.center = -moveX;
//				}
//				else
//				{
//					object3d_player->position.x += -30.0f;
//					sphere.center -= moveX;
//				}
//
//			}
//			if (input->isKeyTrigger(DIK_D))
//			{
//				if (object3d_player->position.x >= +30.0f)
//				{
//					object3d_player->position.x = +30.0f;
//					//sphere.center = -moveX;
//				}
//				else
//				{
//					object3d_player->position.x += 30.0f;
//					sphere.center += moveX;
//				}
//			}
//
////秒数カウント
//			timer--;
//			if (timer <= 0)
//			{
//				secound++;
//				if (secound >= 10)
//				{
//					secound2++;
//					secound = 0;
//
//				}
//				if (secound2 >= 6)
//				{
//					minute++;
//					secound2 = 0;
//				}
//				timer = 60;
//
//			}
			debugtext_minute.Print(spriteCommon, moji, secound_x, secound_y, 1.0f);
			debugtext_minute2.Print(spriteCommon, moji2, secound2_x, secound2_y, 1.0f);
			debugtext_eye.Print(spriteCommon, moji3, minute_x, minute_y, 1.0f);

			/*if (object3d_player->position.z >= 13000 )
			{
				object3d_camera->CameraMoveVector({ 0,0,-speed });
				SoundPlayWave(xAudio2.Get(), soundData4);
	
				if (object3d_player->position.z >= 13250)
				{
					gameScene = 2;
				}

				
			}*/

			/*if (input->isKey(DIK_R))
			{
				gameScene = 2;
			}*/
		}
		else if (gameScene == 2)
		{
			sprite.position.x = 640;
			minute_x = 500;	
			minute_y = 250;
			secound_x = 650;
			secound_y = 250;
			secound2_x = 600;
			secound2_y = 250;
			debugtext_minute.Print(spriteCommon, moji, secound_x, secound_y, 1.0f);
			debugtext_minute2.Print(spriteCommon, moji2, secound2_x, secound2_y, 1.0f);
			debugtext_eye.Print(spriteCommon, moji3, minute_x, minute_y, 1.0f);
			if (input->isKeyTrigger(DIK_SPACE))
			{
				//カメラ
				object3d_camera->SetEye({ 0,50,-50 });
				object3d_camera->SetTarget({ 0,0,0 });
				//プレイヤー
				object3d_player->position.x = 0; //({ 0,-10,-20 });
				object3d_player->position.y = -10;
				object3d_player->position.z = -20;
				object3d_player->rotation.y = -90;
				//道
				object3d_roadCenter->SetPosition({ 0,-30,0 });
				object3d_roadCenter->scale.z = 200;
				object3d_roadCenter->scale.x = 20;

				object3d_roadCenter2->SetPosition({ 0,-30,+800 });//z 400
				object3d_roadCenter2->scale.z = 200;
				object3d_roadCenter2->scale.x = 20;

				object3d_roadCenter3->SetPosition({ 0,-30,+1600 });//z 400
				object3d_roadCenter3->scale.z = 200;
				object3d_roadCenter3->scale.x = 20;

				object3d_eneCenter->SetPosition({ 0,-15,+200 });
				object3d_eneLeft->SetPosition({ -35,-15,+400 });
				object3d_eneRight->SetPosition({ 35,-15,+400 });

				object3d_glass->SetPosition({ -50,-30,0 });
				object3d_glass->scale.x = 200;
				object3d_glass->scale.z = 200;

				object3d_glass2->SetPosition({ -50,-30,780 });
				object3d_glass2->scale.x = 200;
				object3d_glass2->scale.z = 200;

				object3d_dash->rotation.y = 180;

				object3d_dash->SetPosition({ 0,-15,400 });



				object3d_dash2->rotation.y = 180;

				object3d_dash2->SetPosition({ -35,-15,1600 });

				ene_x = object3d_eneCenter->position.x;
				ene_y = object3d_eneCenter->position.y;
				ene_z = object3d_eneCenter->position.z;

				ene_x2 = object3d_eneLeft->position.x;
				ene_y2 = object3d_eneLeft->position.y;
				ene_z2 = object3d_eneLeft->position.z;

				ene_x3 = object3d_eneRight->position.x;
				ene_y3 = object3d_eneRight->position.y;
				ene_z3 = object3d_eneRight->position.z;

				dash_x = object3d_dash->position.x;
				dash_z = object3d_dash->position.z;
				dash2_x = object3d_dash2->position.x;
				dash2_z = object3d_dash2->position.z;

				sphere.center = XMVectorSet(0, -10, 0, 1);//中心点座標

				minute_x = 0;
				minute_y = 0;
				secound_x = 150;
				secound_y = 0;
				secound2_x = 100;
				secound2_y = 0;

				atattaflag = 0;
				timer_dash = 60;

				timer = 60;
				secound = 0;
				secound2 = 0;
				minute = 0;

				centerCount = 0;
				leftCount = 0;
				rightCount = 0;
				sprite1.position = { 640, 360, 0 };

				setEyeflag = 0;

				//三角形の初期値を設定
				triangle_ene.p0 = XMVectorSet(ene_x - 1.0f, ene_y, ene_z, 1);//左手前
				triangle_ene.p1 = XMVectorSet(ene_x, ene_y, ene_z + 5.0f, 1);//左奥
				triangle_ene.p2 = XMVectorSet(ene_x + 1.0f, ene_y, ene_z, 1);//右手前
				triangle_ene.normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0);//上向き

				triangle_ene2.p0 = XMVectorSet(ene_x2 - 1.0f, ene_y2, ene_z2, 1);//左手前
				triangle_ene2.p1 = XMVectorSet(ene_x2, ene_y2, ene_z2 + 5.0f, 1);//左奥
				triangle_ene2.p2 = XMVectorSet(ene_x2 + 1.0f, ene_y2, ene_z2, 1);//右手前
				triangle_ene2.normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0);//上向き

				triangle_ene3.p0 = XMVectorSet(ene_x3 - 1.0f, ene_y3, ene_z3, 1);//左手前
				triangle_ene3.p1 = XMVectorSet(ene_x3, ene_y3, ene_z3 + 5.0f, 1);//左奥
				triangle_ene3.p2 = XMVectorSet(ene_x3 + 1.0f, ene_y3, ene_z3, 1);//右手前
				triangle_ene3.normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0);//上向き


				triangle_dash.p0 = XMVectorSet(dash_x - 5.5f, object3d_dash->position.y, dash_z - 10.5f, 1);//左手前
				triangle_dash.p1 = XMVectorSet(dash_x - 5.5f, object3d_dash->position.y, dash_z + 10.5f, 1);//左奥
				triangle_dash.p2 = XMVectorSet(dash_x + 10.5f, object3d_dash->position.y, dash_z - 10.5f, 1);//右手前
				triangle_dash.normal = XMVectorSet(0.0f, 0.1f, 0.0f, 0);//上向き

				triangle_dash2.p0 = XMVectorSet(dash2_x - 5.5f, object3d_dash2->position.y, dash2_z - 10.5f, 1);//左手前
				triangle_dash2.p1 = XMVectorSet(dash2_x - 5.5f, object3d_dash2->position.y, dash2_z + 10.5f, 1);//左奥
				triangle_dash2.p2 = XMVectorSet(dash2_x + 10.5f, object3d_dash2->position.y, dash2_z - 10.5f, 1);//右手前
				triangle_dash2.normal = XMVectorSet(0.0f, 0.1f, 0.0f, 0);//上向き

				gameScene = 0;
			}
		}


		object3d_player->Update();
		object3d_roadCenter->Update();
		object3d_roadCenter2->Update();
		object3d_roadCenter3->Update();

		object3d_eneCenter->Update();
		object3d_eneLeft->Update();
		object3d_eneRight->Update();

		object3d_glass->Update();
		object3d_glass2->Update();

		object3d_home->Update();
		object3d_homeWhite->Update();
		object3d_homeLong->Update();

		object3d_dash->Update();
		object3d_dash2->Update();

		object3d_goal->Update();

		box->Update();
		box2->Update();
		box3->Update();
		box4->Update();
		box5->Update();
		box6->Update();
		box7->Update();
		box8->Update();
		box9->Update();
		box10->Update();
		box11->Update();
		box12->Update();
		box13->Update();
		box14->Update();
		box15->Update();
		box16->Update();
		box17->Update();
		box18->Update();
		box19->Update();
		box20->Update();
		box21->Update();
		box22->Update();
		box23->Update();
		box24->Update();
		box25->Update();

		box_select->Update();
		box_player->Update();
		box_enemy->Update();
		box_enemy2->Update();
		//box4->Update();
		//GPUと座標を共有させる
		// GPU上のバッファに対応した仮想メモリを取得
		Vertex* vertMap = nullptr;
		result = vertBuff->Map(0, nullptr, (void**)&vertMap);

		// 全頂点に対して
		for (int i = 0; i < _countof(vertices); i++)
		{
			vertMap[i] = vertices[i];   // 座標をコピー
		}

		// マップを解除
		vertBuff->Unmap(0, nullptr);

		dxcommon->BeginDraw();

		// ４．描画コマンドここから
		dxcommon->GetCmdlist()->SetPipelineState(object3dPipelineSet.pipelinestate.Get());

		dxcommon->GetCmdlist()->SetGraphicsRootSignature(object3dPipelineSet.rootsignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { basicDescHeap.Get() };
		dxcommon->GetCmdlist()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		dxcommon->GetCmdlist()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		Object3d::PreDraw(dxcommon->GetCmdlist());
		box->Draw();
		box2->Draw();
		box3->Draw();
		box4->Draw();
		box5->Draw();
		box6->Draw();
		box7->Draw();
		box8->Draw();
		box9->Draw();
		box10->Draw();
		box11->Draw();
		box12->Draw();
		box13->Draw();
		box14->Draw();
		box15->Draw();
		box16->Draw();
		box17->Draw();
		box18->Draw();
		box19->Draw();
		box20->Draw();
		box21->Draw();
		box22->Draw();
		box23->Draw();
		box24->Draw();
		box25->Draw();

		box_select->Draw();
		box_player->Draw();
		box_enemy->Draw();
		box_enemy2->Draw();
			/*object3d_Draw();player->Draw();
		object3d_roadCenter->Draw();
		object3d_roadCenter2->Draw();
		object3d_roadCenter3->Draw();

		object3d_eneCenter->Draw();
		object3d_eneLeft->Draw();
		object3d_eneRight->Draw();

		object3d_glass->Draw();
		object3d_glass2->Draw();

		object3d_home->Draw();
		object3d_homeWhite->Draw();
		object3d_homeLong->Draw();

		object3d_dash->Draw();
		object3d_dash2->Draw();*/

		//object3d_goal->Draw();
		Object3d::PostDraw();

		//スプライト共通コマンド
		SpriteCommonBeginDraw(dxcommon->GetCmdlist(), spriteCommon);
		//スプライト描画
		SpriteDraw(sprite, dxcommon->GetCmdlist(),spriteCommon, dxcommon->GetDev());

		SpriteDraw(sprite1, dxcommon->GetCmdlist(), spriteCommon, dxcommon->GetDev());

		debugtext.DrawAll(dxcommon->GetCmdlist(), spriteCommon, dxcommon->GetDev());

		debugtext_eye.DrawAll(dxcommon->GetCmdlist(), spriteCommon, dxcommon->GetDev());
		debugtext_minute.DrawAll(dxcommon->GetCmdlist(), spriteCommon, dxcommon->GetDev());
		debugtext_minute2.DrawAll(dxcommon->GetCmdlist(), spriteCommon, dxcommon->GetDev());
		// ４．描画コマンドここまで
		// ５．リソースバリアを戻す

		dxcommon->EndDraw();
		
	}
	// ウィンドウクラスを登録解除
	win->WinFinalize();

	delete input;
	delete dxcommon;
	delete object3d_player;
	delete object3d_roadCenter;
	delete object3d_roadCenter2;
	delete object3d_roadCenter3;
	delete object3d_home;
	delete object3d_homeWhite;
	delete object3d_homeLong;
	delete object3d_eneCenter;
	delete object3d_eneLeft;
	delete object3d_eneRight;
	delete object3d_dash ;
	delete object3d_dash2;
	delete object3d_glass;
	delete object3d_glass2;

	delete box;
	delete box2;
	delete box3;
	delete box4;
	delete box5;
	delete box6;
	delete box7;
	delete box8;
	delete box9;
	delete box10;
	delete box11;
	delete box12;
	delete box13;
	delete box14;
	delete box15;
	delete box16;
	delete box17;
	delete box18;
	delete box19;
	delete box20;
	delete box21;
	delete box22;
	delete box23;
	delete box24;
	delete box25;
	
		xAudio2.Reset();
	SoundUnload(&soundData1);
	SoundUnload(&soundData3);
	return 0;
}		   
