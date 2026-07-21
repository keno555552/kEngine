#pragma once
#include <vector>

class ItemStateCountingSystem
{
public:
	ItemStateCountingSystem(int maxItemsNum = 1000);

	/// アイテムを削除するカウンターを更新する
	bool DeleteItemsCounter(float deltaTime);

	/// 使っていないアイテムを削除リストに移動する
	void DeleteUnusedItems();

	/// 形なく全部リセットする
	void ResetAll();

	/// deleteItemListにあるアイテムを見る
	std::vector<int> GetDeletedItemList() { return deletedItemList; }

	/// itemNumをusingItemListに戻す
	void ReuseItem(int itemNum);

	/// 次で使えるアイテムの番号を取得する
	int GetUsableItemNum();

	/// 今使っているカウンターの値を取得する
	int GetUsingCounter();

	/// 今のカンター値を取って、1つ増やす、最大値を超えたらtrueを返す
	int CounterPlus();


private:
	std::vector<int> usingItemList;
	std::vector<int> usedItemList;
	std::vector<int> deletedItemList;

	int counter_;
	int maxItemsNum_;

	float timer_;
	float updateInterval_ = 1.0f;
};