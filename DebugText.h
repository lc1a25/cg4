#pragma once
#include "Sprite.h"

#include <string>

class DebugText
{
public:
	static const int maxCharCount = 256;

	static const int fontWidth = 80;

	static const int fontHeight = 85;

	static const int fontLineCount = 16;

private:
	//スプライト共通(借りてくる)
	SpriteCommon* spriteCommon_ = nullptr;

	Sprite* sprites_[maxCharCount];

	int spriteIndex_ = 0;

public:

	void debugTextInit(SpriteCommon* spritecommon, UINT texnumber);

	void Print(const std::string& text, float x, float y, float scale = 1.0f);

	void DrawAll();

	void Finalize();
};
