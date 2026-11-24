#pragma once
#include <memory>
#include <unordered_map>
#include "Object/Sprite.h"
#include "tool/TimeManager/TimeManager.h"
#include "GAME/Object/Backpack/Backpack.h"

/// ============== 手持ち表示（左下）============== ///
static inline const Vector2 kInHandCenter = { 92.0f, 635.0f };   // リングの中心（inHandSelecter.png 上）
static inline const Vector2 kInHandPivot = { 0.0f, 720.0f };    // 回転の軸（左下の角）
static inline const Vector2 kInHandIconSize = { 95.0f, 95.0f };    // アイコンの大きさ（中心で回すため）
static inline const float   kInHandWheelR = 110.0f;              // 回転の半径（軸はリング中心の真下）
static inline const float   kInHandSwapAngle = 1.0f;                // 隣のアイコンとの角度（rad）
static inline const float   kInHandSpin = 1.5f;                // アイコン自身の回転の倍率
static inline const float   kInHandSwapTime = 0.25f;               // 切り替え時間
static inline const float   kLayerInHandRing = 60.0f;               // リング（手前）
static inline const float   kLayerInHandIcon = 59.0f;               // アイコン（リングの後ろ → はみ出た所は隠れる）

class kEngine;
class Player;
class ScreenUI
{
public:
	ScreenUI(kEngine* kEngine);

	void Initialize();
	void Finalize();

	void SetPlayer(Player* player) { player_ = player; }

	void Update();
	void Draw();

private:

	kEngine* system_ = nullptr;				// 借り
	Player* player_ = nullptr;				// 借り

private:
	/// ============== テキスチャハンドル ============== ///
	int TH_InHandSelecter{};
	std::vector<int> TH_InHandIcon_{};

private:
	/// =================== 実体 =================== ///

	std::unique_ptr <SimpleSprite> inHandSelecter_{};
	std::unordered_map<BPItemID, std::unique_ptr<SimpleSprite>> icons_;
	std::unordered_map<BPItemID, bool> iconVisible_;   // このフレーム描くか

private:

	/// =================== 調整値（ImGui で触る）=================== ///
	Vector2 center_ = kInHandCenter;
	Vector2 pivot_ = kInHandPivot;
	float swapAngle_ = kInHandSwapAngle;
	float spin_ = kInHandSpin;
	float swapTime_ = kInHandSwapTime;
	bool  reverse_ = false;          // 回る向きを逆に
	bool  previewAll_ = false;          // 止まっていても隣を表示（調整用）

	/// =================== 切り替え演出 =================== ///
	BPItemID shown_ = BPItemID::None;    // 今リングにいる物
	int swapDir_ = +1;
	Timer swapTimer_;

private:
	/// アイコンを置く（angle = 0 でリング中心）
	void PlaceIcon(SimpleSprite* icon, float angle);
	/// 持っている装備の一覧（kBPEquipOrder の順）
	std::vector<BPItemID> GetOwnedEquips() const;

#ifdef USE_IMGUI
	void ImGuiPart();
#endif
};

