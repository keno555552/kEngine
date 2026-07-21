#include "ItemStateCountingSystem.h"
#include "Logger.h"

ItemStateCountingSystem::ItemStateCountingSystem(int maxItemsNum) :
	maxItemsNum_(maxItemsNum),
	counter_(0) {}

bool ItemStateCountingSystem::DeleteItemsCounter(float deltaTime) {
	bool result = false;

	/// 1秒ごとにusingItemListをusedItemListに移動する
	/// Timer更新、もしupdateInterval_を超えたらusedItemListに移動する
	timer_ += deltaTime;
	if (timer_ >= updateInterval_) {
		timer_ = 0.0f;
		result = true;
	}

	/// usingItemListをusedItemListに移動する
	usedItemList.insert(usedItemList.end(), usingItemList.begin(), usingItemList.end());
	usingItemList.clear();
	return result;
}

void ItemStateCountingSystem::DeleteUnusedItems() {
	/// 使えないアイテムをdeletedItemListに移動する
	deletedItemList.insert(deletedItemList.end(), usedItemList.begin(), usedItemList.end());
	usedItemList.clear();
}

void ItemStateCountingSystem::ResetAll() {
	deletedItemList.clear();
	usedItemList.clear();
	usingItemList.clear();
	counter_ = 0;
}

void ItemStateCountingSystem::ReuseItem(int itemNum) {

	/// usedItemListにitemNumがある場合、usingItemListに移動する
	auto it = std::find(usedItemList.begin(), usedItemList.end(), itemNum);
	if (it != usedItemList.end()) {
		usingItemList.push_back(itemNum);
		usedItemList.erase(it);
	}

	/// ない場合、deletedItemListにitemNumがある場合、usingItemListに移動する
	auto it2 = std::find(deletedItemList.begin(), deletedItemList.end(), itemNum);
	if (it2 != deletedItemList.end()) {
		usingItemList.push_back(itemNum);
		deletedItemList.erase(it2);
	}

	/// 元とusingItemListにitemNumがある場合、何もしない

	/// 未分配のitemNumが入ってきた場合はエラー（必ず GetUsableItemNum() を経由する）
	if (itemNum >= counter_) {
		Logger::Log("ItemStateCountingSystem: Invalid item number: %d", itemNum);
		return;
	}
}

int ItemStateCountingSystem::GetUsableItemNum() {

	/// deletedItemListがない場合、新しいintを返す
	if (deletedItemList.empty()) {
		return CounterPlus();
	} else {
		/// deletedItemListがある場合、deletedItemListの最後の要素を返す
		int lastDeletedItem = deletedItemList.back();
		usingItemList.push_back(lastDeletedItem);
		deletedItemList.pop_back();
		return lastDeletedItem;
	}
	return 0;
}

int ItemStateCountingSystem::GetUsingCounter() {
	return counter_ - 1;
}

int ItemStateCountingSystem::CounterPlus() {
	if (counter_ < maxItemsNum_) {
		usingItemList.push_back(counter_);
		return counter_++;
	}
	return -1;
}
