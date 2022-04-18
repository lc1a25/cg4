#include "Sprite.h"

Sprite* Sprite::Create(SpriteCommon* spriteCommon, UINT texnumber, XMFLOAT2 anchorpoint, bool isFlagX, bool isFlagY)
{
	Sprite* instance = new Sprite();

	instance->Init(spriteCommon, texnumber, anchorpoint, isFlagX, isFlagY);

	return instance;
}

void Sprite::Init(SpriteCommon* spriteCommon, UINT texnumber, XMFLOAT2 anchorpoint,
					bool isFlagX, bool isFlagY)
{
	HRESULT result = S_FALSE;


	spriteCommon_ = spriteCommon;

	texnumber_ = texnumber;

	//アンカーポイントをコピー
	anchorpoint_ = anchorpoint;

	//反転フラグをコピー
	isFlagX_ = isFlagX;

	isFlagY_ = isFlagY;

	VertexPosUv vertices[] = {
		{{0.0f	,100.0f	,0.0f},{0.0f,1.0f} },
		{{0.0f	,0.0f	,0.0f},{0.0f,0.0f} },
		{{100.0f,100.0f	,0.0f},{1.0f,1.0f} },
		{{100.0f,0.0f	,0.0f},{1.0f,0.0f} },

	};



	if (spriteCommon_->GetTexBuff(texnumber))
	{
		//テクスチャ情報の画像が読み込み済みなら
		D3D12_RESOURCE_DESC resDesc = spriteCommon_->GetTexBuff(texnumber)->GetDesc();

		//スプライトの大きさを画像の解像度に合わせる
		size_ = { (float)resDesc.Width,(float)resDesc.Height };
		texSize_ = { (float)resDesc.Width,(float)resDesc.Height };
	}



	//頂点バッファの生成
	result = spriteCommon_->Device_()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff_)
	);

	//頂点バッファデータ転送
	TransVertexBuffer();

	//頂点バッファビューへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	vertBuff_->Unmap(0, nullptr);

	//頂点バッファビューの生成
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = sizeof(vertices);
	vbView_.StrideInBytes = sizeof(vertices[0]);

	//定数バッファの生成
	result = spriteCommon->Device_()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff_));

	//定数バッファにデータを転送
	ConstBufferData* constMap = nullptr;
	result = constBuff_->Map(0, nullptr, (void**)&constMap);
	constMap->color = XMFLOAT4(1, 1, 1, 1);//色指定(R G B A)

	//平行投影法
	constMap->mat = spriteCommon_->GetMatProjection();	
	constBuff_->Unmap(0, nullptr);

}

void Sprite::TransVertexBuffer()
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

	float left = (0.0f - anchorpoint_.x) * size_.x;

	float right = (1.0f - anchorpoint_.x) * size_.x;

	float top = (0.0f -	anchorpoint_.y) * size_.y;

	float bottom = (1.0f - anchorpoint_.y) * size_.y;

	if (isFlagX_ == true)
	{
		left = -left;
		right = -right;
	}

	if (isFlagY_ == true)
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
	if (spriteCommon_->GetTexBuff(texnumber_))
	{
		//テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = spriteCommon_->GetTexBuff(texnumber_)->GetDesc();

		float tex_left = texLeftTop_.x / resDesc.Width;

		float tex_right = (texLeftTop_.x + texSize_.x) / resDesc.Width;

		float tex_top = texLeftTop_.y / resDesc.Height;

		float tex_bottom = (texLeftTop_.y + texSize_.y) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom };
		vertices[LT].uv = { tex_left,	tex_top };
		vertices[RB].uv = { tex_right,	tex_bottom };
		vertices[RT].uv = { tex_right,	tex_top };

	}

	//頂点バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	vertBuff_->Unmap(0, nullptr);
}

void Sprite::Update()
{
	//ワールド行列の更新
	matWorld_ = XMMatrixIdentity();

	//Z軸回転
	matWorld_ *= XMMatrixRotationZ(XMConvertToRadians(rotation_.z));

	//平行移動
	matWorld_ *= XMMatrixTranslation(position_.x, position_.y, position_.z);

	//定数バッファの転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = constBuff_->Map(0, nullptr, (void**)&constMap);
	constMap->color = color_;
	constMap->mat = matWorld_ * spriteCommon_->GetMatProjection();



	constBuff_->Unmap(0, nullptr);
}

void Sprite::Draw()
{

	if (isInvisible_ == true)
	{
		return;
	}

	ID3D12GraphicsCommandList* cmdList = spriteCommon_->GetcmdList();

	//頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &vbView_);

	//ルートパラメータ0番に定数バッファをセット
	cmdList->SetGraphicsRootConstantBufferView(0,constBuff_->GetGPUVirtualAddress());

	//ルートパラメータ1番にシェーダーリソースビューをセット
	spriteCommon_->SetGraphicsRootDescriptorTable(1, texnumber_);
	//
	/*cmdList->SetGraphicsRootDescriptorTable(1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(spriteCommon.descHeap->GetGPUDescriptorHandleForHeapStart(),
			texnumber_,
			spriteCommon_->Device_()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));*/

	//ポリゴンの描画
	cmdList->DrawInstanced(4, 1, 0, 0);
}
