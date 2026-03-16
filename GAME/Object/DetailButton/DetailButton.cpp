#include "DetailButton.h"

DetailButton::DetailButton(kEngine* system) {
	system_ = system;

	pos_ = {};

	LoadTexture(system);

	texture1_ = system_->LoadTexture("./GAME/Assets/texture/button/1.png");
	texture2_ = system_->LoadTexture("./GAME/Assets/texture/button/2.png");
	texture3_ = system_->LoadTexture("./GAME/Assets/texture/button/3.png");
	texture4_ = system_->LoadTexture("./GAME/Assets/texture/button/4.png");
	texture5_ = system_->LoadTexture("./GAME/Assets/texture/button/5.png");
	texture6_ = system_->LoadTexture("./GAME/Assets/texture/button/6.png");
	texture7_ = system_->LoadTexture("./GAME/Assets/texture/button/7.png");
	texture8_ = system_->LoadTexture("./GAME/Assets/texture/button/8.png");
	texture9_ = system_->LoadTexture("./GAME/Assets/texture/button/9.png");

	buttonState_ = NORMAL;

	drawButton_[NORMAL] = [this]() { DrawNormalButton(); };
	drawButton_[PRESS] = [this]() { DrawPressButton(); };
	drawButton_[SELECT] = [this]() { DrawSelectButton(); };
	drawButton_[LOCK] = [this]() { DrawLockButton(); };
}

DetailButton::~DetailButton() {
	for (auto& ptr : sprite_) {
		ptr.reset();
	}
}

void DetailButton::LoadTexture(kEngine* system) {
	system->LoadTexture("./GAME/Assets/texture/button/1.png");
	system->LoadTexture("./GAME/Assets/texture/button/2.png");
	system->LoadTexture("./GAME/Assets/texture/button/3.png");
	system->LoadTexture("./GAME/Assets/texture/button/4.png");
	system->LoadTexture("./GAME/Assets/texture/button/5.png");
	system->LoadTexture("./GAME/Assets/texture/button/6.png");
	system->LoadTexture("./GAME/Assets/texture/button/7.png");
	system->LoadTexture("./GAME/Assets/texture/button/8.png");
	system->LoadTexture("./GAME/Assets/texture/button/9.png");
}


bool DetailButton::CheckIsSelect(Vector2 TPos, float width, float height) {
	if (buttonState_ == LOCK) return false;

	Vector2 A[2] = { { pos_.x - buttonSize_.x / 2, pos_.y + buttonSize_.y / 2},{ pos_.x + buttonSize_.x / 2, pos_.y - buttonSize_.y / 2 } };
	Vector2 B[2] = { { TPos.x - width / 2, TPos.y + height / 2},{ TPos.x + width / 2, TPos.y - height / 2 } };
	int DFlat[2] = {};

	if (B[0].x < A[1].x) { DFlat[0]++; }
	if (B[1].x > A[0].x) { DFlat[0]++; }

	if (B[0].y > A[1].y) { DFlat[1]++; }
	if (B[1].y < A[0].y) { DFlat[1]++; }

	if (DFlat[0] == 2 && DFlat[1] == 2) {
		return true;
	} else {
		return false;
	}
}

void DetailButton::ResetButton() {
	buttonState_ = NORMAL;
}


void DetailButton::Init(float width, float height) {
	width = buttonDefaultSize_.x;
	if (width > buttonDefaultSize_.x) buttonSize_.x = width;
	height = buttonDefaultSize_.y;
	if (height > buttonDefaultSize_.y) buttonSize_.y = height;

	/// Buttonの数値を計算する



	for (auto& ptr : sprite_) {
		ptr = std::make_unique<SimpleSprite>();
		ptr->IntObject(system_);
		ptr->CreateDefaultData();
	}
	sprite_[0];
}

void DetailButton::Update() {
	int mouseX = 0, mouseY = 0;
	mouseX = system_->GetMousePosX();
	mouseY = system_->GetMousePosY();

	if (buttonState_ != LOCK) {
		if (CheckIsSelect({ (float)mouseX,(float)mouseY }, 1, 1)) {
			if (buttonState_ != PRESS) buttonState_ = SELECT;
			if (system_->GetMouseIsPush(0)) {
				buttonState_ = PRESS;
			}
		} else {
			if (!system_->GetMouseIsPush(0)) buttonState_ = NORMAL;
		}
	}
}

void DetailButton::Render() {
	drawButton_[buttonState_]();
}

void DetailButton::DrawNormalButton() {

}

void DetailButton::DrawPressButton() {
}

void DetailButton::DrawSelectButton() {
}

void DetailButton::DrawLockButton() {
}
