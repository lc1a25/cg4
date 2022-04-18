#include "DebugText.h"

void DebugText::debugTextInit(SpriteCommon* spritecommon, UINT texnumber)
{
	assert(spritecommon);

	spriteCommon_ = spritecommon;

	for (int i = 0; i < _countof(sprites_); i++)
	{
		sprites_[i] = Sprite::Create(spritecommon, texnumber, { 0,0 });
	}
}

void DebugText::Print(const std::string& text, float x, float y, float scale)
{
	for (int i = 0; i < text.size(); i++)
	{
		if (spriteIndex_ >= maxCharCount)
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

		sprites_[spriteIndex_]->SetPosition({ x + fontWidth * scale * i,y,0 });
		sprites_[spriteIndex_]->SetTexLeftTop({ (float)fontIndexX * fontWidth,(float)fontIndexY * fontHeight });
		sprites_[spriteIndex_]->SetTexsize({ fontWidth,fontHeight });
		sprites_[spriteIndex_]->SetSize({ fontWidth * scale,fontHeight * scale });

		//頂点バッファ転送
		sprites_[spriteIndex_]->TransVertexBuffer();

		//更新
		sprites_[spriteIndex_]->Update();

		spriteIndex_++;
	}
}

void DebugText::DrawAll()
{
	for (int i = 0; i < spriteIndex_; i++)
	{
		sprites_[i]->Draw();
	}

	spriteIndex_ = 0;
}

void DebugText::Finalize()
{
	for (auto& sprite : sprites_)
	{
		delete sprite;
	}
}
