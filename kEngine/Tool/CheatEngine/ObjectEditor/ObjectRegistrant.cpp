#include "ObjectRegistrant.h"
#include <algorithm>

ObjectRegistrant& ObjectRegistrant::GetInstance() {
	static ObjectRegistrant* instance = new ObjectRegistrant();   // 故意不釋放，避免結束時的解構順序問題
	return *instance;
}

void ObjectRegistrant::Register(Object* obj) {
	if (!obj) return;
	if (std::find(objects_.begin(), objects_.end(), obj) != objects_.end()) return;   // IntObject 被重複呼叫也安全
	objects_.push_back(obj);
}

void ObjectRegistrant::Unregister(Object* obj) {
	objects_.erase(std::remove(objects_.begin(), objects_.end(), obj), objects_.end());
}