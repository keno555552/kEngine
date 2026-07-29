#pragma once
#include <fstream>
#include "externals/nlohmann/json.hpp"

#include "MathsIncluder.h"
#include "Object/Object.h"

class kEngine;
class JsonHelper
{
public:
	static nlohmann::json V3ToJson(const Vector3& v);
	static nlohmann::json V4ToJson(const Vector4& v);
	static nlohmann::json ObjectDataToJson(const ObjectData& obj, kEngine* system);
	static nlohmann::json ObjectPartToJson(const ObjectPart& part, kEngine* system);
	static nlohmann::json TransformToJson(const Transform& transform);
	static nlohmann::json MaterialConfigToJson(const MaterialConfig& mat, kEngine* system);


public:
	static Vector3 JsonToV3(const nlohmann::json& j);
	static Vector4 JsonToV4(const nlohmann::json& j);
	static Transform JsonToTransform(const nlohmann::json& j);
	static MaterialConfig JsonToMaterialConfig(const nlohmann::json& j, kEngine* system);
	static ObjectData JsonToObjectData(const nlohmann::json& j, kEngine* system);
	static ObjectPart JsonToObjectPart(const nlohmann::json& j, kEngine* system);
};

