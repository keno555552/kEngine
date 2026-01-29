#pragma once
#include "Object/Object.h"
#include "Object/Sprite.h"

class CountdownTimer : public SimpleSprite
{
public:
	CountdownTimer(kEngine* kEngine);

	//void Update(Camera* camera) override;
	//void Draw() override;
	//
	//void Start();
	//void Stop();
	//bool IsTimeUp();
	//bool IsLessTime();
	//
	//Timer& GetTime();
	//
	//void Reset();

private:

	/// ========= テキスチャーハンドル =========///
	/// 
	/// 0 =1,1=2,2=3
	int numberTextureHandles_;
	int pointTextureHandle_;
	int leftTimeTextureHandle_;


	/// ============ オブジェクト =============///

	SimpleSprite* numberSprites_{};
	SimpleSprite* pointSprite_{};
	SimpleSprite* leftTimeSprite_{};

	/// =========== フラク、内部変数 ===========///
	Vector4 usingColor = { 1.0f,1.0f,1.0f,0.0f };
	float startScale_ = 0.5f;
	Timer* timer_ = nullptr;
	int numberCount_ = 3;
	bool isStart_ = false;

private:

	void ImGuiPart();

};