#pragma once
#include "Scene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Sight\Sight.h"
#include "Bullet\Bullet.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "Enemy\Enemy.h"
#include "BackPoint\BackPoint.h"
#include "tool\MapChip\MapChipField.h"
#include "tool\CollisionManager\CollisionManager.h"


class Scene1 :public Scene
{
public:
	Scene1(kEngine* system);
	~Scene1();
	void Update() override;
	void Draw() override;


private:
	kEngine* system_ = nullptr;				// �؂�

	/// ============== �J�����֘A ==============///
	/// �X�e�[�W�J����(main�J����)
	Camera* camera_ = nullptr;
	/// �f�o�b�N�J����
	DebugCamera* debugCamera_ = nullptr;
	/// ���g���Ă���J����
	Camera* usingCamera_ = nullptr;			// �؂�p

	/// =========== ���\�[�X�n���h�� ============///
	/// ���f���n���h��
	int MH_skydome_ = 0;
	int MH_player_ = 0;
	int MH_sight_ = 0;
	int MH_underGround_ = 0;
	int MH_bullet_ = 0;
	int MH_object_ = 0;
	int MH_enemy_ = 0;
	int MH_backPoint_ = 0;

	/// �e�L�X�`���[�n���h��
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

	/// =========== �Q�[���I�u�W�F�N�g ==========///
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

	/// ========== �R���W�����}�l�[�W���[ =========///
	CollisionManager collisionManager_;

	/// =============== �t���O ================///
	bool useDebugCamera = false;

private:
	/// ============= �V�[������� =============///
	void CameraPart();
	void ImGuiPart();

	void GenerateMap();
};

