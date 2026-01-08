#pragma once
#include "Scene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Targeter\Targeter.h"
#include "Bullet\Bullet.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "Enemy\Enemy.h"
#include "tool\MapChip\MapChipField.h"
#include "tool\CollisionManager\CollisionManager.h"

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
	int MH_skydome_ = 0;
	int MH_player_ = 0;
	int MH_targeter_ = 0;
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

	Targeter* targeter_ = nullptr;

	Player* player_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	std::vector<std::vector<Object*>> blockObjectList_;
	std::vector<Bullet*> bulletList_;
	std::vector<Enemy*> enemyList_;

	/// ========== コリジョンマネージャー =========///
	CollisionManager collisionManager_;

	/// =============== フラグ ================///
	bool useDebugCamera = false;

private:
	/// ============= シーン内命令 =============///
	void CameraPart();
	void ImguiPart();

	void GenerateMap();
};

