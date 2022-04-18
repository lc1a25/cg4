#pragma once

#include "fbxsdk.h"

#include <d3d12.h>
#include <d3dx12.h>

#include <string>

class FbxLoader
{
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static FbxLoader* GetInstance();

private:
	// privateなコンストラクタ（シングルトンパターン）
	FbxLoader() = default;
	// privateなデストラクタ（シングルトンパターン）
	~FbxLoader() = default;
	// コピーコンストラクタを禁止（シングルトンパターン）
	FbxLoader(const FbxLoader& obj) = delete;
	// コピー代入演算子を禁止（シングルトンパターン）
	void operator=(const FbxLoader& obj) = delete;

public:
	/// <summary>
	///	初期化
	/// </summary>
	/// <param name = "device">D3D12device</param>
	void Init(ID3D12Device * device);

	/// <summary>
	/// 後始末
	/// </summary>
	void Finalize();

private:
	//D3D12device
	ID3D12Device* device = nullptr;
	//FBXマネージャー
	FbxManager* fbxManager = nullptr;
	//FBXインポーター
	FbxImporter* fbxImporter = nullptr;


//fbxよみこみ
private://エイリアス
	//std:: を省略
	using string = std::string;

public://定数
	//モデル格納ルートパス
	static const string baseDirectory;

	/// <summary>
	///	ファイルからFBXモデルよみこみ
	/// </summary>
	/// <param name = "model name">モデル名</param>
	void LoadModelFile(const string& modelName);

	
};