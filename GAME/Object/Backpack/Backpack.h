#pragma once
#include <vector>
#include <map>
#include <unordered_map>

using ItemQuantity = int;

static inline const int dontHaveItem = -1;

/// ================== バックパック ================== ///

/// アイテムの種類（ビットフラグ：BPItemCantAdd のマスクに使う → 必ず 2 の累乗）
enum class BPItemID {
	None = 0,
	Gun,
	Pickaxe,
	// Cristal = 1 << 2,
	// Gold = 1 << 3,
	kCount,
};

/// 分類：どの値を見るか
enum class BPItemKind {
	Loadable,   // owned と quantity 両方（武器＋弾）
	Unique,     // owned だけ
	Stack,      // quantity だけ
};

/// ID → 分類 の対照表（新しいアイテムはここに 1 行足す）
static inline const std::unordered_map<BPItemID, BPItemKind> BPItemKindTable = {
	{ BPItemID::Gun,     BPItemKind::Loadable },
	{ BPItemID::Pickaxe, BPItemKind::Unique },
};

/// ビットマスクが要る所だけ変換（None は 0）
constexpr int BPItemBit(BPItemID id) {
	return (id == BPItemID::None) ? 0 : (1 << ((int)id - 1));
}

/// 拾えない物（装備類）の既定値。実際に使うのは各 BPItem の cantAdd_
static inline const int kBPItemCantAddDefault = (int)BPItemID::Gun | (int)BPItemID::Pickaxe;

/// 装備できる物と、切り替えの順番（滾輪・選択圏で使う）
static inline const BPItemID kBPEquipOrder[] = { 
	BPItemID::Gun, 
	BPItemID::Pickaxe 
};

/// 1 種類分の状態
struct BPItemSlot {
	bool owned = false;            // 本体を持っているか（Loadable / Unique）
	ItemQuantity quantity = 0;     // 数（Loadable の弾 / Stack）
};

class BPItem {
public:
	BPItem() : itemDatas_((int)BPItemID::kCount, 0) {}

	/// ================= 増減 ================= ///
	/// 拾う：拾えない設定の Unique は false。Unique は 1 個まで、Loadable / Stack は数を足す
	bool AddItem(BPItemID itemID, ItemQuantity quantity);
	/// 数を減らす。足りなければ false
	bool RemoveItem(BPItemID itemID, ItemQuantity quantity);
	/// その種類を 0 に
	bool RemoveItemAll(BPItemID itemID);
	/// 全部 0 に
	bool RemoveAllItems();

	/// ================= 初期化 ================= ///
	/// 本体を手に入れる（初期装備・イベント等。拾えない設定は無視）
	void ObtainItem(BPItemID itemID, int num = 1);

	/// ================= 装備 ================= ///
	BPItemID GetEquipped() const { return equipped_; }
	bool Equip(BPItemID itemID);     // 持っていなければ false
	void EquipNext();                // 持っていない物は飛ばす
	void EquipPrev();

	/// ================= 確認 ================= ///
	int  GetItemQuantity(BPItemID itemID) const;
	bool HasItem(BPItemID itemID) const { return GetItemQuantity(itemID) > 0; }
	/// 本体を持っているか（Loadable / Unique 用）
	bool IsOwned(BPItemID itemID) const;
	int GetLastEquipDir() const { return lastEquipDir_; }   // +1 / -1

	/// ================= 拾えるかのルール ================= ///
	bool CanPickUp(BPItemID itemID) const { return (BPItemBit(itemID) & cantAdd_) == 0; }
	void AllowPickUp(BPItemID itemID) { cantAdd_ &= ~BPItemBit(itemID); }
	void DenyPickUp(BPItemID itemID) { cantAdd_ |= BPItemBit(itemID); }
	void ResetPickUpRule() { cantAdd_ = kBPItemCantAddDefault; }

	/// ================= バックパック本体 ================= ///
	bool HaveBackpack() const { return haveBackpack_; }
	void SetHaveBackpack(bool have) { haveBackpack_ = have; }

private:
	static bool TryGetKind(BPItemID itemID, BPItemKind& out);
	/// ビットフラグ → 配列の番号（1→0, 2→1, 4→2 …）。範囲外は -1
	static int ToIndex(BPItemID itemID);
	void EquipStep(int dir);

private:
	bool haveBackpack_ = false;
	int cantAdd_ = kBPItemCantAddDefault;
	int lastEquipDir_ = +1;
	std::vector<ItemQuantity> itemDatas_;    // [ToIndex(id)] = 数
	BPItemID equipped_ = BPItemID::None;
};