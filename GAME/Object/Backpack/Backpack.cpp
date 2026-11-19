#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Backpack.h"
#include <algorithm>

bool BPItem::TryGetKind(BPItemID itemID, BPItemKind& out) {
	auto it = BPItemKindTable.find(itemID);
	if (it == BPItemKindTable.end()) return false;
	out = it->second;
	return true;
}

int BPItem::ToIndex(BPItemID itemID) {
	const int index = (int)itemID;
	return (index > 0 && index < (int)BPItemID::kCount) ? index : -1;   // None と範囲外は -1
}

/// ============================== 増減 ============================== ///
bool BPItem::AddItem(BPItemID itemID, ItemQuantity quantity) {
	BPItemKind kind;
	const int i = ToIndex(itemID);
	if (i < 0 || !TryGetKind(itemID, kind)) return false;

	switch (kind) {
	case BPItemKind::Unique:
		if (!CanPickUp(itemID)) return false;      // 装備は拾えない
		itemDatas_[i] = 1;                         // 唯一品は 0 / 1
		return true;

	case BPItemKind::Loadable:                     // 弾
	case BPItemKind::Stack:
		if (quantity <= 0) return false;
		itemDatas_[i] += quantity;
		return true;
	}
	return false;
}

bool BPItem::RemoveItem(BPItemID itemID, ItemQuantity quantity) {
	const int i = ToIndex(itemID);
	if (i < 0 || quantity <= 0 || itemDatas_[i] < quantity) return false;   // 足りない
	itemDatas_[i] -= quantity;
	return true;
}

bool BPItem::RemoveItemAll(BPItemID itemID) {
	const int i = ToIndex(itemID);
	if (i < 0 || itemDatas_[i] == 0) return false;
	itemDatas_[i] = 0;
	return true;
}

bool BPItem::RemoveAllItems() {
	const bool hadAny = std::any_of(itemDatas_.begin(), itemDatas_.end(), [](int q) { return q > 0; });
	std::fill(itemDatas_.begin(), itemDatas_.end(), 0);
	return hadAny;
}

void BPItem::ObtainItem(BPItemID itemID, int num) {
	BPItemKind kind;
	const int i = ToIndex(itemID);
	if (i < 0 || !TryGetKind(itemID, kind)) return;

	switch (kind) {
	case BPItemKind::Unique:
		itemDatas_[i] = 1;                         // 唯一品は 0 / 1
		return;

	case BPItemKind::Loadable:                     // 弾
		itemDatas_[i] += num;
		return;
	case BPItemKind::Stack:
		itemDatas_[i] += num;
		return;
	}
	return;
}

bool BPItem::Equip(BPItemID itemID) {
	if (!IsOwned(itemID)) return false;
	equipped_ = itemID;
	return true;
}

void BPItem::EquipNext() { EquipStep(+1); }

void BPItem::EquipPrev() { EquipStep(-1); }

void BPItem::EquipStep(int dir) {
	lastEquipDir_ = (dir > 0) ? +1 : -1;
	const int count = (int)std::size(kBPEquipOrder);

	/// 今の位置（無ければ -1 → 最初の候補から）
	int cur = -1;
	for (int i = 0; i < count; ++i) if (kBPEquipOrder[i] == equipped_) cur = i;

	/// 持っている次の物を探す（一周したら諦める）
	for (int step = 1; step <= count; ++step) {
		const int i = ((cur + dir * step) % count + count) % count;
		if (IsOwned(kBPEquipOrder[i])) { equipped_ = kBPEquipOrder[i]; return; }
	}
}


/// ============================== 確認 ============================== ///
int BPItem::GetItemQuantity(BPItemID itemID) const {
	const int i = ToIndex(itemID);
	return (i >= 0) ? itemDatas_[i] : 0;
}

bool BPItem::IsOwned(BPItemID itemID) const {
	const int i = ToIndex(itemID);
	return (i >= 0) ? itemDatas_[i] > 0 : false;
}
