#pragma once
#include <vector>
#include <string>

#include <iostream>
#include <fstream>
#include "externals/nlohmann/json.hpp"

#include "PSOKeys.h"

class PSORecorder {
public:
    void Load(const std::string& path);
    void AppendPSOKeyRecord(const PSOKey& key);
    std::vector<PSOKey>& GetAllKeys() { return keylist_; }

private:
    std::string psoCacheFilePath_;
    std::vector<PSOKey> keylist_;

private:

    void CreateDefaultPSOcacheJson(std::string fullPath);
    std::string FormatPSOJson(const nlohmann::json& file);

    nlohmann::json findAndCreatePSOCache();
    size_t FindStartIndex(const nlohmann::json& arr);

};

