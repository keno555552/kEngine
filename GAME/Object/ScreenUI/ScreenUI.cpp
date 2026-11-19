#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "ScreenUI.h"
#include "core/kEngine.h"
#include "GAME/Object/Player/Player.h"
#include <cmath>
#include <algorithm>

ScreenUI::ScreenUI(kEngine* kEngine) : system_(kEngine) {

	/// ============== テキスチャハンドル ============== ///
	TH_InHandSelecter = system_->LoadTexture("GAME/resources/texture/UI/inHandSelecter.png");

	TH_InHandIcon_.assign((int)BPItemID::kCount, -1);
	TH_InHandIcon_[(int)BPItemID::Pickaxe] = system_->LoadTexture("GAME/resources/texture/UI/pickaxe_ui.png");
	TH_InHandIcon_[(int)BPItemID::Gun] = system_->LoadTexture("GAME/resources/texture/UI/handgun_ui.png");

	/// ============== 実体 ============== ///
	inHandSelecter_ = std::make_unique<SimpleSprite>();
	inHandSelecter_->IntObject(system_);
	inHandSelecter_->CreateDefaultData();
	inHandSelecter_->objectParts_[0].materialConfig->textureHandle = TH_InHandSelecter;
	inHandSelecter_->mainPosition.transform.translate = { 0.0f, 0.0f, kLayerInHandRing };

	for (int i = 0; i < (int)BPItemID::kCount; ++i) {
		if (TH_InHandIcon_[i] < 0) continue;
		const BPItemID id = (BPItemID)i;

		auto icon = std::make_unique<SimpleSprite>();
		icon->IntObject(system_);
		icon->CreateDefaultData();
		icon->objectParts_[0].materialConfig->textureHandle = TH_InHandIcon_[i];
		/// 中心で回るように anchor を画像の半分に（ピクセル単位）
		icon->objectParts_[0].anchorPoint = { kInHandIconSize.x * 0.5f, kInHandIconSize.y * 0.5f };
		icon->mainPosition.transform.translate.z = kLayerInHandIcon;

		icons_[id] = std::move(icon);
		iconVisible_[id] = false;
	}

	swapTimer_.InitM(swapTime_, system_->GetTimeManager());   // 最初は止まっている
}

void ScreenUI::Initialize() {
	/// 最初は演出なしで今の装備を出す
	shown_ = player_ ? player_->GetInventory().GetEquipped() : BPItemID::None;
}

void ScreenUI::Finalize() {
	player_ = nullptr;
	inHandSelecter_.reset();
	icons_.clear();
}

std::vector<BPItemID> ScreenUI::GetOwnedEquips() const {
	std::vector<BPItemID> list;
	if (!player_) return list;
	const BPItem& bp = player_->GetInventory();
	for (BPItemID id : kBPEquipOrder) {
		if (bp.IsOwned(id) && icons_.count(id)) list.push_back(id);
	}
	return list;
}

/// ============================== 更新 ============================== ///
void ScreenUI::Update() {
	if (!player_) return;
	const BPItem& bp = player_->GetInventory();

	/// 装備が変わった → 回し始める
	const BPItemID now = bp.GetEquipped();
	if (now != shown_) {
		shown_ = now;
		swapDir_ = bp.GetLastEquipDir();
		swapTimer_.Reset0();
	}
	if (!swapTimer_.GetIsMax()) swapTimer_.ToMix();
	const bool isSwapping = !swapTimer_.GetIsMax();

	/// 全部いったん非表示
	for (auto& [id, visible] : iconVisible_) visible = false;

	const std::vector<BPItemID> owned = GetOwnedEquips();
	const int n = (int)owned.size();
	const int cur = (int)(std::find(owned.begin(), owned.end(), shown_) - owned.begin());
	if (n == 0 || cur >= n) return;   // 手ぶら

	const float t = swapTimer_.easyIn(2.0f);          // だんだん遅く
	const float dir = (float)swapDir_;
	const float offset = dir * (1.0f - t);            // 1 → 0

	for (int i = 0; i < n; ++i) {
		/// 今の装備から見た位置 k（一番近い向き。2 個の時は「さっき出ていった側 = -dir」）
		int k = i - cur;
		if (k > n / 2)  k -= n;
		if (k < -n / 2) k += n;
		if (n % 2 == 0 && std::abs(k) == n / 2) k = -swapDir_ * (n / 2);

		/// 止まっている時は今の装備だけ、回っている時は隣まで
		if (!isSwapping && k != 0) continue;
		if (std::abs(k) > 2) continue;

		SimpleSprite* icon = icons_[owned[i]].get();
		PlaceIcon(icon, (k + offset) * kInHandSwapAngle);
		iconVisible_[owned[i]] = true;
	}
}

void ScreenUI::PlaceIcon(SimpleSprite* icon, float angle) {
	/// 軸 → リング中心 のベクトルから半径と基準角を出す（angle = 0 でちょうどリング中心）
	const float dx = center_.x - pivot_.x;
	const float dy = center_.y - pivot_.y;
	const float radius = std::sqrt(dx * dx + dy * dy);
	const float baseAngle = std::atan2(dx, -dy);          // 軸から見た「真上 = 0」の角度（画面は y 下向き）

	const float a = (reverse_ ? -angle : angle);
	auto& tr = icon->mainPosition.transform;
	tr.translate.x = pivot_.x + radius * std::sin(baseAngle + a);
	tr.translate.y = pivot_.y - radius * std::cos(baseAngle + a);
	tr.rotate.z = a * spin_;                              // リング中心では 0（まっすぐ）
}

/// ============================== 描画 ============================== ///
void ScreenUI::Draw() {
	inHandSelecter_->Draw();
	for (auto& [id, icon] : icons_) {
		if (iconVisible_[id]) icon->Draw();
	}
}


#ifdef USE_IMGUI
void ScreenUI::ImGuiPart() {
	ImGui::Begin("ScreenUI / InHand");

	ImGui::DragFloat2("Pivot", &pivot_.x, 1.0f);
	ImGui::DragFloat2("Center", &center_.x, 1.0f);
	ImGui::DragFloat("SwapAngle", &swapAngle_, 0.01f, 0.0f, 3.14f);
	ImGui::DragFloat("Spin", &spin_, 0.05f, -5.0f, 5.0f);
	if (ImGui::DragFloat("SwapTime", &swapTime_, 0.01f, 0.05f, 2.0f)) {
		swapTimer_.InitM(swapTime_, system_->GetTimeManager());
	}
	ImGui::Checkbox("Reverse", &reverse_);
	ImGui::Checkbox("Preview neighbors", &previewAll_);

	/// 計算結果（確認用）
	const float dx = center_.x - pivot_.x, dy = center_.y - pivot_.y;
	ImGui::Text("Radius %.1f  BaseAngle %.2f rad", std::sqrt(dx * dx + dy * dy), std::atan2(dx, -dy));

	/// その場で切り替えて確認
	if (player_) {
		if (ImGui::Button("Prev")) player_->GetInventory().EquipPrev();
		ImGui::SameLine();
		if (ImGui::Button("Next")) player_->GetInventory().EquipNext();
	}

	/// 決まった値をコピペ用に表示
	ImGui::Separator();
	ImGui::Text("Center { %.1ff, %.1ff }  Pivot { %.1ff, %.1ff }", center_.x, center_.y, pivot_.x, pivot_.y);
	ImGui::Text("SwapAngle %.2ff  Spin %.2ff  SwapTime %.2ff", swapAngle_, spin_, swapTime_);

	ImGui::End();
}
#endif