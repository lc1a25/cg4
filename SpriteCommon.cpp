#include "SpriteCommon.h"

#include <d3dcompiler.h>
#include <string>
#include <d3dx12.h>
#include <cassert>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

void SpriteCommon::CreatePipeline2d()
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
		//ComPtr<ID3D12RootSignature> rootsignature;
	
	
	
		//ルートシグネチャの設定
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);
	
		rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	
		//PipelineSet pipelineSet_;
	
		ComPtr<ID3DBlob> rootSigBlob;
	
		result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
			&rootSigBlob, &errorBlob);
	
		result = dev_->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
			IID_PPV_ARGS(&pipelineSet_.rootsignature));
		gpipeline.pRootSignature = pipelineSet_.rootsignature.Get();
	
		ComPtr<ID3D12PipelineState> pipelinestate;// ID3D12PipelineState* pipelinestate = nullptr;
		result = dev_->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineSet_.pipelinestate));
}

void SpriteCommon::Init(ID3D12Device* dev, ID3D12GraphicsCommandList* cmdList,int window_width, int window_height)
{
	//nullポインタチェック
	assert(dev);

	//メンバ変数に記録
	dev_ = dev;

	cmdList_ = cmdList;

	HRESULT result = S_FALSE;



	//スプライト用パイプライン生成
	CreatePipeline2d();

	//並行投影の射影行列変換
	matProjection_ = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f);

	//設定構造
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダーから見える
	descHeapDesc.NumDescriptors = spriteSRVCount;//テクスチャ枚数

	//デスクリプタヒープの生成
	result = dev_->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap_));//生成
}

void SpriteCommon::PreDraw()
{
	//パイプラインステートの設定
	cmdList_->SetPipelineState(pipelineSet_.pipelinestate.Get());

	//ルートシグネチャの設定
	cmdList_->SetGraphicsRootSignature(pipelineSet_.rootsignature.Get());

	//プリミティブ形状を設定
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャ用デスクリプタヒープ
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap_.Get() };
	cmdList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void SpriteCommon::LoadTexture(UINT texnumber, const wchar_t* filename)
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


	result = dev_->CreateCommittedResource(//GPUリソースの生成
		&texheapProp,
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texBuff_[texnumber]));

	//リソース設定
	texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);

	result = dev_->CreateCommittedResource(//GPUリソースの生成
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texBuff_[texnumber]));


	//テクスチャバッファにデータ転送
	result = texBuff_[texnumber]->WriteToSubresource(
		0,
		nullptr,							//全領域にコピー
		img->pixels,						//元データアドレス
		(UINT)img->rowPitch,				//1ラインサイズ
		(UINT)img->slicePitch);			//全サイズ


	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap_->GetCPUDescriptorHandleForHeapStart(),
			texnumber,
			dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV =
		CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap_->GetGPUDescriptorHandleForHeapStart(),
			texnumber,
			dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	//シェーダーリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};//設定構造体
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//RGBA
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	//ヒープの?番目にシェーダーリソースビューを生成
	dev_->CreateShaderResourceView(texBuff_[texnumber].Get(),//ビューと関連付けるバッファ
		&srvDesc,//テクスチャ設定情報
		CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap_->GetCPUDescriptorHandleForHeapStart(), texnumber,
			dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));

}

ID3D12Resource* SpriteCommon::GetTexBuff(int texNumber)
{
	assert(0 <= texNumber && texNumber < spriteSRVCount);

	return texBuff_[texNumber].Get();
}

void SpriteCommon::SetGraphicsRootDescriptorTable(UINT rootParametorIndex,UINT texNumber)
{
	cmdList_->SetGraphicsRootDescriptorTable(rootParametorIndex,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap_->GetGPUDescriptorHandleForHeapStart(),
			texNumber,
			dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));
}
