#pragma once

#include "PipelineSet.h"
#include <DirectXTex.h>

/// <summary>
/// スプライト共通
/// </summary>
class SpriteCommon
{
private: // エイリアス
// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略


	using XMMATRIX = DirectX::XMMATRIX;
	

public:
	//テクスチャの最大枚数
	static const int spriteSRVCount = 512;

private://メンバ変数には _ がつく
	//パイプラインセット
	PipelineSet pipelineSet_;

	//射影行列
	XMMATRIX matProjection_{};

	//テクスチャ用デスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> descHeap_;

	//テクスチャリソース(テクスチャバッファ)の配列
	ComPtr<ID3D12Resource> texBuff_[spriteSRVCount];

	//デバイス(借りてくる)
	ID3D12Device* dev_ = nullptr;

	//コマンドリスト(借りてくる)
	ID3D12GraphicsCommandList* cmdList_;

private:
	void CreatePipeline2d();

public://メンバ関数


	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dev">デバイス</param>
	/// <param name="cmdList">コマンドリスト</param>
	/// <param name="window_width">ウィンドウよこ</param>
	/// <param name="window_height">ウィンドウたて</param>
	void Init(ID3D12Device* dev, ID3D12GraphicsCommandList* cmdList,int window_width, int window_height);
	
	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 画像読み込み
	/// </summary>
	void LoadTexture(UINT texnumber, const wchar_t* filename);

	/// <summary>
	/// テクスチャ取得
	/// </summary>
	/// <param name="texNumber">テクスチャ番号</param>
	/// <returns>テクスチャリソース</returns>
	ID3D12Resource* GetTexBuff(int texNumber);

	ID3D12Device* Device_() { return dev_; }

	const XMMATRIX& GetMatProjection() { return matProjection_; }

	ID3D12GraphicsCommandList* GetcmdList() { return cmdList_; }

	/// <summary>
	/// ルートデスクリプタテーブルの設定
	/// </summary>
	/// <param name="rootParametorIndex">ルートパラメータ番号</param>
	/// <param name="texNumber">テクスチャ番号</param>
	void SetGraphicsRootDescriptorTable(UINT rootParametorIndex,UINT texNumber);

};


