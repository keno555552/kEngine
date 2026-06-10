#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "GAME\Object\Sight\Sight.h"
#include "GAME\Object\Bullet\Bullet.h"
#include "Camera\DebugCamera.h"
#include "GAME\Object\Player\Player.h"
#include "GAME\Object\Enemy\Enemy.h"
#include "Data\Game\MapChip\MapChipField.h"
#include "tool\CollisionManager\CollisionManager.h"
#include "StartTimer\StartTimer.h"
#include "CountdownTimer\CountdownTimer.h"

class SceneTest :public BaseScene
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
	std::weak_ptr<Camera> camera_;
	/// デバックカメラ
	std::weak_ptr<DebugCamera> debugCamera_;
	/// 今使っているカメラ
	std::weak_ptr <Camera> usingCamera_;			// 借り用

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

