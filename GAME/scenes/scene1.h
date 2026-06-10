#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "GAME\Object\Sight\Sight.h"
#include "GAME\Object\Bullet\Bullet.h"
#include "GAME\Object\Player\Player.h"
#include "GAME\Object\Enemy\Enemy.h"
#include "GAME\Object\BackPoint\BackPoint.h"
#include "Data\Game\MapChip\MapChipField.h"
#include "tool\CollisionManager\CollisionManager.h"


class Scene1 :public BaseScene
{
public:
	Scene1(kEngine* system);
	~Scene1();
	void Update() override;
	void Draw() override;


private:
	kEngine* system_ = nullptr;				// �؂�

	/// ============== カメラ関連 ==============///
	/// メーンカメラ
	std::weak_ptr<Camera> camera_;
	/// デバックカメラ
	std::weak_ptr<DebugCamera> debugCamera_;
	/// 今使っているカメラ
	std::weak_ptr<Camera> usingCamera_;			// �؂�p

	/// =========== テキスチャハンドル ============///
	/// モデルハンドル
	int MH_skydome_ = 0;
	int MH_player_ = 0;
	int MH_sight_ = 0;
	int MH_underGround_ = 0;
	int MH_bullet_ = 0;
	int MH_object_ = 0;
	int MH_enemy_ = 0;
	int MH_backPoint_ = 0;

	/// テキスチャハンドル
	int TH_box_ = 0;
	int TH_try_ = 0;
	int TH_uv_ = 0;

	int TH_dirt_ = 0;
	int TH_rock_ = 0;

	int TH_buleSkySkydome_ = 0;

	int TH_hpUI_ = 0;
	int TH_hpUI_empty_ = 0;

	int SH_BGM_ = 0;
	int SE_killEnemy_ = 0;

	/// =========== 実体 ==========///
	Object* skydome_ = nullptr;
	//Object* object_ = nullptr;

	Object* underGround_BG_ = nullptr;

	Sight* sight_ = nullptr;

	Player* player_ = nullptr;

	BackPoint* backPoint_ = nullptr;
	
	Object* backSign_ = nullptr;

	SimpleSprite* hpUI_[3] = { nullptr,nullptr,nullptr };

	MapChipField* mapChipField_ = nullptr;
	std::vector<std::vector<Object*>> blockObjectList_;
	std::vector<Bullet*> bulletList_;
	std::vector<Enemy*> enemyList_;

	/// ========== 当たり判定マネージャー =========///
	CollisionManager collisionManager_;

	/// =============== �t���O ================///
	bool useDebugCamera = false;

private:
	/// ============= �V�[������� =============///
	void CameraPart();
	void ImGuiPart();

	void GenerateMap();
};

