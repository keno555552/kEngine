#include "MapDatas.h"
#include <algorithm>

const MapUnit* MapData::FindUnit(const std::string& name) const {
	for (const auto& u : units) if (u.name == name) return &u;
	return nullptr;
}
const MapUnit* MapData::FindUnit(int id) const {
	for (const auto& u : units) if (u.id == id) return &u;
	return nullptr;
}
/// 非 const 版は const 版を呼ぶだけ（同じ処理を2回書かない）
MapUnit* MapData::FindUnit(const std::string& name) {
	return const_cast<MapUnit*>(static_cast<const MapData*>(this)->FindUnit(name));
}
MapUnit* MapData::FindUnit(int id) {
	return const_cast<MapUnit*>(static_cast<const MapData*>(this)->FindUnit(id));
}

int MapData::NextUnitId() const {
	int maxId = blankId;
	for (const auto& u : units) maxId = (std::max)(maxId, u.id);
	return maxId + 1;
}

void MapData::ChangeId(int oldId, int newId) {
	if (oldId == newId) return;
	/// 先に両方を探しておく（書き換えた後だと見つけ直せない）
	const bool selfIsBlank = (oldId == blankId);
	const bool otherIsBlank = (newId == blankId);
	MapUnit* self = selfIsBlank ? nullptr : FindUnit(oldId);
	MapUnit* other = otherIsBlank ? nullptr : FindUnit(newId);

	if (selfIsBlank) blankId = newId;  else if (self)  self->id = newId;
	if (otherIsBlank) blankId = oldId; else if (other) other->id = oldId;
}