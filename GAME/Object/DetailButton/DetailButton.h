#pragma once
#include <vector>
#include <functional>
#include <memory>
#include "Object/Sprite.h"
#include "MathsIncluder.h"
#include "Data/Render/CPUData/materialconfig.h"
#include "kEngine.h"



class DetailButton
{
public:
	DetailButton(kEngine* system);
	~DetailButton();

	static void LoadTexture(kEngine* system);

	bool GetPress() { if(buttonState_ == PRESS)return true; }
	bool GetCanPress() { if (buttonState_ == LOCK)return true; }

	void SetPos(Vector2 pos) { pos_ = pos; }
	void SetWidth(float width) { buttonSize_.x = width; }
	void SetHeight(float height) { buttonSize_.y = height; }
	void SetIsLock(bool isLock) { buttonState_ = LOCK; }

	void SetNormalColor(Vector4 color) { normalColor_ = color; }
	void SetPressColor(Vector4 color) { pressColor_ = color; }
	void SetSelectColor(Vector4 color) { selectColor_ = color; }
	void SetLockColor(Vector4 color) { lockColor_ = color; }

	bool CheckIsSelect(Vector2 mouse, float width, float height);
	void ResetButton();

	void Init(float width, float height);

	void Update();
	void Render();

private:

	enum {
		NORMAL,
		PRESS,
		SELECT,
		LOCK,
	};
	int buttonState_ = NORMAL;

private:

	Vector2 pos_;

	/// ==================== モデルハンドル ===================== ///
	std::unique_ptr<SimpleSprite> sprite_[9];
	
	/// ================== テクスチャーハンドル ================== ///
	int texture1_;
	int texture2_;
	int texture3_;
	int texture4_;
	int texture5_;
	int texture6_;
	int texture7_;
	int texture8_;
	int texture9_;

	/// =================== デフォルトサイズ =================== ///
	Vector2 buttonDefaultSize_ = { 76.0f,86.0f };
	Vector2 spriteLTDefaultSize_ = { 33.0f,38.0f };
	Vector2 spriteCTDefaultSize_ = { 10.0f,38.0f };
	Vector2 spriteRTDefaultSize_ = { 33.0f,38.0f };
	Vector2 spriteLMDefaultSize_ = { 33.0f,10.0f };
	Vector2 spriteCMDefaultSize_ = { 10.0f,10.0f };
	Vector2 spriteRMDefaultSize_ = { 33.0f,10.0f };
	Vector2 spriteLBDefaultSize_ = { 33.0f,38.0f };
	Vector2 spriteCBDefaultSize_ = { 10.0f,38.0f };
	Vector2 spriteRBDefaultSize_ = { 33.0f,38.0f };

	/// =================== サイズ =================== ///
	Vector2 buttonSize_ = { 76.0f,86.0f };
	Vector2 spriteLTSize_ = { 33.0f,38.0f };
	Vector2 spriteCTSize_ = { 10.0f,38.0f };
	Vector2 spriteRTSize_ = { 33.0f,38.0f };
	Vector2 spriteLMSize_ = { 33.0f,10.0f };
	Vector2 spriteCMSize_ = { 10.0f,10.0f };
	Vector2 spriteRMSize_ = { 33.0f,10.0f };
	Vector2 spriteLBSize_ = { 33.0f,38.0f };
	Vector2 spriteCBSize_ = { 10.0f,38.0f };
	Vector2 spriteRBSize_ = { 33.0f,38.0f };

	/// ==================== 色設定 ===================== ///

	Vector4 normalColor_ = { 0.22f, 0.28f, 0.36f, 1.0f};
	Vector4 pressColor_ = { 0.16f, 0.20f, 0.26f, 1.0f };
	Vector4 selectColor_ = { 0.30f, 0.45f, 0.85f, 1.0f };
	Vector4 lockColor_ = { 0.45f, 0.48f, 0.52f, 1.0f };

	kEngine* system_ = nullptr;//借り
	MaterialConfig material_ = {};

	std::vector<std::function<void()>> drawButton_;

private:
	void DrawNormalButton();
	void DrawPressButton();
	void DrawSelectButton();
	void DrawLockButton();
};