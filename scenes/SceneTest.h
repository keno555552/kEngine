#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Sight\Sight.h"
#include "Bullet\Bullet.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "Enemy\Enemy.h"
#include "tool\MapChip\MapChipField.h"
#include "tool\CollisionManager\CollisionManager.h"

class SceneTest :public BaseScene
{
public:
	SceneTest(kEngine* system);
	~SceneTest();
	void Update() override;
	void Draw() override;

	void DrawBlock();


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
	int MH_skydome_ = 0;
	int MH_player_ = 0;
	int MH_sight_ = 0;
	int MH_underGround_ = 0;
	int MH_bullet_ = 0;
	int MH_object_{};
	int MH_enemy_{};

	/// テキスチャーハンドル
	int TH_box_ = 0;
	int TH_try_ = 0;
	int TH_uv_ = 0;

	int TH_dirt_ = 0;
	int TH_rock_ = 0;

	int TH_buleSkySkydome_ = 0;

	/// =========== ゲームオブジェクト ==========///
	Object* skydome_ = nullptr;
	//Object* object_ = nullptr;

	Object* underGround_BG_ = nullptr;

	Sight* sight_ = nullptr;

	Player* player_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	StartTimer* startTimer_ = nullptr;
	CountdownTimer* countdownTimer_ = nullptr;
	std::vector<std::vector<Object*>> blockObjectList_;
	std::vector<Bullet*> bulletList_;
	std::vector<Enemy*> enemyList_;

	/// ========== コリジョンマネージャー =========///
	CollisionManager collisionManager_;

	/// =============== フラグ / 変数 ================///
	bool useDebugCamera = false;
	float timerScale = 1.0f;
	Vector4 countdownColor = { 1.0f,0.0f,0.0f,1.0f };

private:
	/// ============= シーン内命令 =============///
	void CameraPart();
	void ImGuiPart();

	void GenerateMap();
};

