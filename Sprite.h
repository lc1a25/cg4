#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include "spriteCommon.h"

using namespace DirectX;

/// <summary>
/// スプライト1枚を表す
/// </summary>
class Sprite
{
private: // エイリアス
// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略


	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
private:

	SpriteCommon* spriteCommon_ = nullptr;

	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff_;

	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	//定数バッファ
	ComPtr<ID3D12Resource> constBuff_;

	//XYZ軸周りの回転軸
	XMFLOAT3 rotation_ = { 0.0f,0.0f, 0.0f };

		//座標
	XMFLOAT3 position_ = { 0,0,0 };

	//ワールド行列
	XMMATRIX matWorld_;

	//色
	XMFLOAT4 color_ = { 1,1,1,1 };

	//テクスチャ番号
	UINT texnumber_ = 0;

	//大きさ
	XMFLOAT2 size_ = { 100, 100 };

	//アンカーポイント
	XMFLOAT2 anchorpoint_ = { 0.5f,0.5f };

	//左右反転
	bool isFlagX_ = false;

	//上下反転
	bool isFlagY_ = false;

	//テクスチャ左上座標
	XMFLOAT2 texLeftTop_ = { 0,0 };

	//テクスチャ切り出しサイズ
	XMFLOAT2 texSize_ = { 100,100 };

	//非表示
	bool isInvisible_ = false;

public:

	struct VertexPosUv
	{
		XMFLOAT3 pos;//x y z　座標
		XMFLOAT2 uv;//u v 座標
	};

	struct ConstBufferData
	{
		XMFLOAT4 color;//色
		XMMATRIX mat;//3D変換行列
	};

	/// <summary>
	/// スプライト生成処理
	/// </summary>
	/// <param name="spriteCommon">スプライト共通</param>
	/// <param name="texnumber">テクスチャ番号</param>
	/// <param name="anchorpoint">アンカーポイント</param>
	/// <param name="isFlagX">x反転するか</param>
	/// <param name="isFlagY">y反転するか</param>
	static Sprite* Create(SpriteCommon* spriteCommon, UINT texnumber, XMFLOAT2 anchorpoint = { 0.5f,0.5f },
		bool isFlagX = false, bool isFlagY = false);


	/// <summary>
	/// 初期化
	/// </summary>
	void Init(SpriteCommon* spriteCommon, UINT texnumber,XMFLOAT2 anchorpoint , 
				bool isFlagX , bool isFlagY );

	void TransVertexBuffer();


	void Update();

	void Draw();

	void SetPosition(const XMFLOAT3& position) { position_ = position;}

	void SetSize(const XMFLOAT2& size) { size_ = size; }

	void SetTexsize(const XMFLOAT2& texsize) { texSize_ = texsize; }

	void SetTexLeftTop(const XMFLOAT2& texLeftTop) { texLeftTop_ = texLeftTop; }


};
