#include "CountdownTimer.h"

CountdownTimer::CountdownTimer(kEngine* kEngine) {
	SimpleSprite::IntObject(kEngine);
	timer_ = new Timer();
	timer_->Init0(1.0f, kEngine->GetTimeManager());

	/// ========= テキスチャーハンドル =========///

	std::string basePath = "resources/texture/StartTimer/";

	//numberTextureHandles_[2] = kEngine->LoadTexture(basePath + "3.png");
	//numberTextureHandles_[1] = kEngine->LoadTexture(basePath + "2.png");
	//numberTextureHandles_[0] = kEngine->LoadTexture(basePath + "1.png");
	//startTextureHandle_ = kEngine->LoadTexture(basePath + "Start.png");

	/// ============ オブジェクト =============///

	//for (int i = 0; i < 3; i++) {
	//	numberSprites_[i] = new SimpleSprite();
	//	numberSprites_[i]->IntObject(kEngine);
	//	numberSprites_[i]->CreateDefaultData();
	//	numberSprites_[i]->mainPosition.transform.scale = { startScale_,startScale_,1.0f };
	//	numberSprites_[i]->mainPosition.transform.translate = { 640.0f,360.0f,0.0f };
	//	numberSprites_[i]->objectParts_[0].materialConfig->textureHandle = numberTextureHandles_[i];
	//	numberSprites_[i]->objectParts_[0].materialConfig->textureColor = usingColor;
	//	numberSprites_[i]->objectParts_[0].anchorPoint = { 422.0f,178.0f };
	//}
	leftTimeSprite_ = new SimpleSprite();
	leftTimeSprite_->IntObject(kEngine);
	leftTimeSprite_->CreateDefaultData();
	leftTimeSprite_->mainPosition.transform.scale = { startScale_,startScale_,1.0f };
	leftTimeSprite_->mainPosition.transform.translate = { 640.0f,360.0f,0.0f };
	//leftTimeSprite_->objectParts_[0].materialConfig->textureHandle = startTextureHandle_;
	leftTimeSprite_->objectParts_[0].materialConfig->textureColor = usingColor;
	leftTimeSprite_->objectParts_[0].anchorPoint = { 422.0f,178.0f };
}



void CountdownTimer::ImGuiPart() {
}
