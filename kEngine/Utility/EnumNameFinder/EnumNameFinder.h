#pragma once
#include <string>
#include <map>

/// 主に enum class の名前を文字列に変換するためのユーティリティ
///	まず定義する時必ず EnumNameMap<T> を特殊化して、enum 値と文字列のマッピングを提供する必要があります。
/// そして使用するときは必ず対応の.hファイルをincludeしないと使えない

template <typename T>
inline const std::map<T, std::string> EnumNameMap{};

template <typename T>
std::string GetEnumName(T value) {
	const auto& nameMap = EnumNameMap<T>;
	auto it = nameMap.find(value);
	if (it != nameMap.end()) {
		return it->second;
	}
	return "Unknown";
}