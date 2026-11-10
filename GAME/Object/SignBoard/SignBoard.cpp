#define NOMINMAX
#include "SignBoard.h"
#include "kEngine.h"
#include "config.h"

SignBoard::SignBoard(kEngine* system) : system_(system) {
	TH_board_ = system_->LoadTexture("GAME/resources/texture/woodSign.png");

	/// 位置：画面中央 ／ 隠れている時は画面の下の外
	const float w = static_cast<float>(config::GetClientWidth());
	const float h = static_cast<float>(config::GetClientHeight());
	showPos_ = { (w - kSignBoardSize.x) * 0.5f, (h - kSignBoardSize.y) * 0.5f };
	hiddenY_ = h;

	sprite_ = std::make_unique<SimpleSprite>();
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = TH_board_;
	sprite_->mainPosition.transform.translate = { showPos_.x, hiddenY_, kSignBoardLayer };
	sprite_->Update(nullptr);

	moveTimer_.InitM(kSignBoardMoveTime, system_->GetTimeManager());
}

void SignBoard::Open() {
	if (IsOpen()) return;
	startY_ = sprite_->mainPosition.transform.translate.y;
	moveTimer_.Reset0();
	phase_ = Phase::kEntry;
}

void SignBoard::Close() {
	if (!IsOpen()) return;
	startY_ = sprite_->mainPosition.transform.translate.y;
	moveTimer_.Reset0();
	phase_ = Phase::kExit;
}

void SignBoard::SetTexture(int textureHandle) {
	TH_board_ = textureHandle;
	sprite_->objectParts_[0].materialConfig->textureHandle = TH_board_;
}

void SignBoard::SetTexture(const std::string& path) {
	SetTexture(system_->LoadTexture(path));
}

void SignBoard::Update() {
	float& y = sprite_->mainPosition.transform.translate.y;
	float T;

	switch (phase_) {
	case Phase::kHidden:
	case Phase::kShow:
		break;

	case Phase::kEntry:   /// 下から浮かぶ（だんだん遅く）
		moveTimer_.ToMix();
		T = moveTimer_.easyIn(2.0f);
		y = easyIn(startY_, showPos_.y, T, 2.0f);
		if (moveTimer_.GetIsMax()) phase_ = Phase::kShow;
		break;

	case Phase::kExit:    /// 下に沈む（だんだん速く）
		moveTimer_.ToMix();
		T = moveTimer_.easyOut(2.0f);
		y = easyOut(startY_, hiddenY_, T, 2.0f);
		if (moveTimer_.GetIsMax()) phase_ = Phase::kHidden;
		break;
	}

	sprite_->Update(nullptr);
}

void SignBoard::Draw() {
	if (!IsVisible()) return;
	sprite_->Draw();
}