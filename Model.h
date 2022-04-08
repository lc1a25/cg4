#pragma once

#include <string>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3dx12.h>
class Model
{
private:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	//using XMFLOAT4 = DirectX::XMFLOAT4;
	//using XMMATRIX = DirectX::XMMATRIX;

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuffB1;
public:


	static Model* LoadFromOBJ(const std::string& modelname);

	static void SetDevice(ID3D12Device* device)
	{
		Model::device = device;
	}

	struct VertexPosNormalUv
	{
		XMFLOAT3 pos; // xyz座標
		XMFLOAT3 normal; // 法線ベクトル
		XMFLOAT2 uv;  // uv座標
	};

	struct Material
	{
		std::string name;//マテリアル名
		XMFLOAT3 ambient;//アンビエント影響度
		XMFLOAT3 diffuse;//ディフューズ影響度
		XMFLOAT3 specular;//スペキュラー影響度
		float alpha;//アルファ
		std::string textureFilename;//テクスチャファイル名
		//コンストラクタ
		Material()
		{
			ambient = { 0.3f, 0.3f,0.3f };
			diffuse = { 0.0f,0.0f,0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};

	Material material;

	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient;//アンビエント係数
		float pad1;//パディング
		XMFLOAT3 diffuse;//ディフューズ係数
		float pad2;//パディング
		XMFLOAT3 specular;//スペキュラー係数
		float alpha;//アルファ
	};

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	/// <returns>成否</returns>
	bool LoadTexture(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	///	マテリアル読み込み
	///	</summary>
	void LoadMaterial(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// デスクリプタヒープの初期化
	/// </summary>
	/// <returns></returns>
	bool InitializeDescriptorHeap();

	/// <summary>
	/// 各種バッファ生成
	/// </summary>
	void CreateBuffer();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name = "cmdList">描画コマンドリスト</param>
	/// <param name = "rootParamIndexMaterial">マテリアル用ルートパラメータ番号</param>
	void Draw(ID3D12GraphicsCommandList* cmdList, UINT rootParamIndexMaterial);


private:
	//// デバイス
	static ID3D12Device* device;
	//// デスクリプタサイズ
	UINT descriptorHandleIncrementSize;
	//// デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descHeap;
	//// テクスチャバッファ
	ComPtr<ID3D12Resource> texbuff;
	//// シェーダリソースビューのハンドル(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	//// シェーダリソースビューのハンドル(CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	// 頂点データ配列
	std::vector<VertexPosNormalUv> vertices;
	// 頂点インデックス配列
	std::vector<unsigned short> indices;

	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView;
	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView;

private:

	void LoadFromOBJInternal(const std::string& modelname);
};
