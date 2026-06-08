#pragma once
#include <vector>
#include <string>
#include "PSOKeys.h"

class PSORecorder {
public:
void Load(const std::string& path = "");
void Save(const std::string& path = "");

    void AddKey(const PSOKey& key);
    const std::vector<PSOKey>& GetAllKeys() const { return keylist_; }

private:
    std::string psoCacheFilePath_;
    std::vector<PSOKey> keylist_; // 去重後的 key 列表

private:

    void CreateDefaultPSOcacheJson(std::string fullPath);
    void AppendPSOKeyRecord(const PSOKey& key);
    std::string FormatPSOJson(const nlohmann::json& file);

};

