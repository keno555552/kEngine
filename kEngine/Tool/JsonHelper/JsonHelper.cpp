#include "JsonHelper.h"
#include "kEngine.h"
#include "Renderer\Resource\TextureManager.h"


/// ========================================== 書き ========================================== ///

nlohmann::json JsonHelper::V3ToJson(const Vector3& v) {
	return nlohmann::json::array({ v.x, v.y, v.z });
}

nlohmann::json JsonHelper::V4ToJson(const Vector4& v) {
	return nlohmann::json::array({ v.x, v.y, v.z, v.w });
}

nlohmann::json JsonHelper::ObjectDataToJson(const ObjectData& data, kEngine* system) {
	nlohmann::json ObjectDataJson;


	// ============= ObjectPartList ================
	int partIndex = 0;
	nlohmann::json partsJson = nlohmann::json::array();
	for (const auto& part : data.objectParts_) {
		partsJson.push_back(ObjectPartToJson(part, system));
	}
	ObjectDataJson["objectParts"] = partsJson;

	// ================ mainPosition ===============

	ObjectDataJson["mainPosition"] = ObjectPartToJson(data.mainPosition, system);

	// ========== ModelPath (ModelHandle) ==========
	if (data.modelHandle_ >= 0) {
		auto model = system->GetResourceManager()->GetModelData(data.modelHandle_);
		if (model) {
			ObjectDataJson["modelPath"] = model->filePath;
		} else {
			ObjectDataJson["modelPath"] = "";
		}
	} else {
		ObjectDataJson["modelPath"] = "";
	}

	// ============= 基本変数 =============
	ObjectDataJson["isDelete"] = data.isDelete_;
	ObjectDataJson["isBillboard"] = data.isBillboard_;



	// ========== followObject ==========
	if (data.followObject_) {
		ObjectDataJson["followObject"] = data.followObject_->name;
	} else {
		ObjectDataJson["followObject"] = "";
	}

	return ObjectDataJson;
}

nlohmann::json JsonHelper::ObjectPartToJson(const ObjectPart& part, kEngine* system) {

	nlohmann::json partJson;

	partJson["name"] = part.name;

	partJson["Transform"] = TransformToJson(part.transform);

	if (part.materialConfig) {
		partJson["MaterialConfig"] = MaterialConfigToJson(*part.materialConfig, system);
	} else {
		partJson["MaterialConfig"] = "";
	}

	partJson["forward"] = V3ToJson(part.forward);

	return partJson;
}

nlohmann::json JsonHelper::TransformToJson(const Transform& transform) {
	nlohmann::json transformJson;
	transformJson["translate"] = V3ToJson(transform.translate);
	transformJson["rotate"] = V3ToJson(transform.rotate);
	transformJson["scale"] = V3ToJson(transform.scale);
	return transformJson;
}

nlohmann::json JsonHelper::MaterialConfigToJson(const MaterialConfig& mat, kEngine* system) {

	nlohmann::json MaterialJson;

	MaterialJson["useModelTexture"] = mat.useModelTexture;
	MaterialJson["textureHandle"] = TextureManager::GetInstance()->GetTextureFilePath(mat.textureHandle);

	MaterialJson["uvTranslate"] = V3ToJson(mat.uvTranslate);
	MaterialJson["uvScale"] = V3ToJson(mat.uvScale);
	MaterialJson["uvRotate"] = V3ToJson(mat.uvRotate);
	MaterialJson["textureColor"] = V4ToJson(mat.textureColor);

	MaterialJson["lightModelType"] = (int)mat.lightModelType;
	MaterialJson["renderModelType"] = (int)mat.renderModelType;
	MaterialJson["blendModeType"] = (int)mat.blendModeType;
	MaterialJson["rasterizerMode"] = (int)mat.rasterizerMode;
	MaterialJson["depthStencilType"] = (int)mat.depthStencilType;

	MaterialJson["reflectiveStrength"] = mat.reflectiveStrength;

	MaterialJson["enableLighting"] = mat.enableLighting;

	MaterialJson["textureColor2"] = V4ToJson(mat.textureColor2);
	MaterialJson["textureColor3"] = V4ToJson(mat.textureColor3);
	MaterialJson["shininess"] = mat.shininess;
	MaterialJson["intensity"] = mat.intensity;
	MaterialJson["heightScale"] = mat.heightScale;

	return MaterialJson;
}

/// ========================================== 読み ========================================== ///

Vector3 JsonHelper::JsonToV3(const nlohmann::json& j) {
	Vector3 v{ 0, 0, 0 };

	if (j.is_array() && j.size() >= 3) {
		v.x = j[0].get<float>();
		v.y = j[1].get<float>();
		v.z = j[2].get<float>();
	}

	return v;
}

Vector4 JsonHelper::JsonToV4(const nlohmann::json& j) {
	Vector4 v{ 0, 0, 0, 0 };

	if (j.is_array() && j.size() >= 4) {
		v.x = j[0].get<float>();
		v.y = j[1].get<float>();
		v.z = j[2].get<float>();
		v.w = j[3].get<float>();
	}

	return v;
}

Transform JsonHelper::JsonToTransform(const nlohmann::json& j) {
	Transform t;
	if (j.contains("translate"))	t.translate = JsonToV3(j["translate"]);
	if (j.contains("rotate"))		t.rotate	= JsonToV3(j["rotate"]);
	if (j.contains("scale"))		t.scale		= JsonToV3(j["scale"]);
	return t;
}

MaterialConfig JsonHelper::JsonToMaterialConfig(const nlohmann::json& j, kEngine* system) {

	MaterialConfig mat;

	// --- useModelTexture ---
	mat.useModelTexture = j.value("useModelTexture", false);

	// --- useModelTextureによってテキスチャハンドルをとる ---
	if (j.contains("textureHandle")) {
		std::string filePath = j["textureHandle"].get<std::string>();

		if (!filePath.empty()) {
			// 依照 useModelTexture 決定載入方式
			if (mat.useModelTexture)
				mat.textureHandle = TextureManager::GetInstance()->LoadModelTexture(filePath);
			else
				mat.textureHandle = TextureManager::GetInstance()->LoadCommonTexture(filePath);
		} else {
			mat.textureHandle = 0; // defaultTextureHandle
		}
	}

	// --- UV ---
	mat.uvTranslate = JsonToV3(j["uvTranslate"]);
	mat.uvScale = JsonToV3(j["uvScale"]);
	mat.uvRotate = JsonToV3(j["uvRotate"]);

	// --- 色 ---
	mat.textureColor = JsonToV4(j["textureColor"]);
	mat.textureColor2 = JsonToV4(j["textureColor2"]);
	mat.textureColor3 = JsonToV4(j["textureColor3"]);

	// --- PSO ---
	mat.lightModelType = (LightModelType)j.value("lightModelType", 0);
	mat.renderModelType = (RenderModelType)j.value("renderModelType", 0);
	mat.blendModeType = (BlendModeType)j.value("blendModeType", 0);
	mat.rasterizerMode = (RasterizerMode)j.value("rasterizerMode", 0);
	mat.depthStencilType = (DepthStencilType)j.value("depthStencilType", 0);

	// --- 普通変数 ---
	mat.reflectiveStrength = j.value("reflectiveStrength", 0.0f);
	mat.enableLighting = j.value("enableLighting", true);
	mat.shininess = j.value("shininess", 32.0f);
	mat.intensity = j.value("intensity", 1.0f);
	mat.heightScale = j.value("heightScale", 0.0f);

	return mat;
}

ObjectData JsonHelper::JsonToObjectData(const nlohmann::json& j, kEngine* system) {
	ObjectData data;

	// ================ modelPath -> modelHandle ==============
	if (j.contains("modelPath")) {
		std::string modelPath = j["modelPath"].get<std::string>();

		if (!modelPath.empty()) {
			data.modelHandle_ = system->GetResourceManager()->LoadModel(modelPath);
		} else {
			data.modelHandle_ = 0;
		}
	}

	// ===================== objectParts =====================
	if (j.contains("objectParts") && j["objectParts"].is_array()) {
		for (const auto& partJson : j["objectParts"]) {
			ObjectPart part = JsonToObjectPart(partJson, system);
			data.objectParts_.push_back(part);
		}
	}

	// ===================== mainPosition =====================
	if (j.contains("mainPosition")) {
		data.mainPosition = JsonToObjectPart(j["mainPosition"], system);
	}

	// ===================== 基本變數 =====================
	data.isDelete_ = j.value("isDelete", false);
	data.isBillboard_ = j.value("isBillboard", false);

	// ===================== followObject（用 name 找） =====================
	if (j.contains("followObject")) {
		std::string followName = j["followObject"].get<std::string>();

		if (!followName.empty()) {
			for (auto& part : data.objectParts_) {
				if (part.name == followName) {
					data.followObject_ = &part;
					break;
				}
			}
		} else {
			data.followObject_ = nullptr;
		}
	}

	return data;
}

ObjectPart JsonHelper::JsonToObjectPart(const nlohmann::json& j, kEngine* system) {
	ObjectPart part;

	// ===================== name =====================
	part.name = j.value("name", "");

	// ===================== Transform =====================
	if (j.contains("Transform")) {
		part.transform = JsonToTransform(j["Transform"]);
	}

	// ===================== MaterialConfig =====================
	if (j.contains("MaterialConfig") && !j["MaterialConfig"].is_null()) {
		part.materialConfig = std::make_unique<MaterialConfig>(
			JsonToMaterialConfig(j["MaterialConfig"], system)
		);
	} else {
		part.materialConfig = nullptr;
	}

	// ===================== forward =====================
	if (j.contains("forward")) {
		part.forward = JsonToV3(j["forward"]);
	}

	return part;
}

