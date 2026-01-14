#pragma once
#include "Scene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "tool\MapChip\MapChipField.h"

class SceneTest :public Scene
{
public:
	SceneTest(kEngine* system);
	~SceneTest();
	void Update() override;
	void Draw() override;


private:
	kEngine* system_ = nullptr;				// 借り

	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;
	/// デバックカメラ
	DebugCamera* debugCamera_ = nullptr;
	/// 今使っているカメラ
	Camera* usingCamera_ = nullptr;			// 借り用

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int skydomeModelHandle_ = 0;
	int playerModelHandle_ = 0;

	/// テキスチャーハンドル
	int boxTextureHandle_ = 0;
	int tryTextureHandle_ = 0;
	int uvTextureHandle_ = 0;

	/// =========== ゲームオブジェクト ==========///
	Object* skydome_ = nullptr;
	Player* player_ = nullptr;
	SimpleSprite* sprite_ = nullptr;
	SimpleSprite* sprite2_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	std::vector<std::vector<Object*>> blockObjectList_;


	/// =============== フラグ ================///
	bool useDebugCamera = false;

private:
	/// ============= シーン内命令 =============///
	void CameraPart();
	void ImGuiPart();

	void GenerateBlocks();
};

