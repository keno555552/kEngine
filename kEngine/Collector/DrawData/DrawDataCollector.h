#pragma once
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include "materialConfig.h"
#include "DrawData/ObjectData.h"
#include "DrawData/SpriteData.h"
#include "PSOType.h"
#include "Camera/Camera.h"
#include "VertexResource.h"
#include "Render/Queue/RenderData.h"
using ModelID = int;

inline const float layerDepth_Sprite = 0.0001f;
inline const float layeredSpriteDepth_ = 0.4f;

class ResourceManager;
class InstanceManager;
class CameraManager;
class DrawDataCollector
{
public:
	DrawDataCollector(ResourceManager* rm, InstanceManager* im, CameraManager* cm);

	void PreCollect();

	void Collect2D(SpriteData* sprite);
	void Collect3D(ObjectData* object);

	Vector3 GetCameraPosition() const;

	std::unordered_map <PSOType,
		std::unordered_map <MaterialID,
		std::unordered_map <ModelID,
		std::vector<RenderData>>>
		>&GetOpaqueBuckets2D() { return opaqueBuckets2D_; }

	std::vector<RenderData>& GetTransparentObjectParts2D() { return transparentObjectParts2D_; }

	std::unordered_map <PSOType,
		std::unordered_map <MaterialID,
		std::unordered_map <ModelID,
		std::vector<RenderData>>>
		>&GetOpaqueBuckets3D() { return opaqueBuckets3D_; }

	std::vector<RenderData>& GetTransparentObjectParts3D() { return transparentObjectParts3D_; }


private:

	/// ============ スプライト関連 ==============///
	/// ｚバッファ調整
	float SpriteLayerManagement(float zBuffer);
	/// フォローマトリックス作成
	Matrix4x4 MakeFollowObjectMatrix(SpriteData* sprite);
	/// WVP調整
	TransformationMatrix SpriteWVPAdjustment(SpriteData& sprite, SpritePart& part);
	/// バケット追加
	void AddSpriteToBucket(RenderData& renderData,int meshID);

	/// ============= PSO関連 ====================///
	/// フォローマトリックス作成 
	Matrix4x4 MakeFollowObjectMatrix(ObjectData* object);
	/// WVP調整
	TransformationMatrix ObjectWVPAdjustment(ObjectData& object, ObjectPart& part);
	/// バケット追加
	void AddObjectToBucket(RenderData& renderData,int meshID);

	/// ============ マテリアル関連 ==============///

	uint32_t PSODecision(MaterialConfig& material);

private:
	ResourceManager* resourceManager_ = nullptr; /*借り*/
	InstanceManager* instanceManager_ = nullptr; /*借り*/

	/// ============ カメラ関連 ==============///
	CameraManager* cameraManager_ = nullptr; /*借り*/

	/// ==================== 2Dデータ ====================///
	/// 不透明オブジェクトバケット
	std::unordered_map <PSOType,
		std::unordered_map <MaterialID,
		std::unordered_map <ModelID,
		std::vector<RenderData>>>
		> opaqueBuckets2D_;


	/// 透明オブジェクトリスト
	std::vector<RenderData> transparentObjectParts2D_;

	/// 2Dスブライドカンター
	int simpleSpriteCounter_{};
	int unlayeredSpriteCounter_{};

	/// ==================== 3Dデータ ====================///
	/// 不透明オブジェクトバケット
	std::unordered_map <PSOType,
		std::unordered_map <MaterialID,
		std::unordered_map <ModelID,
		std::vector<RenderData>>>
		> opaqueBuckets3D_;

	/// 透明オブジェクトバケット
	std::vector<RenderData> transparentObjectParts3D_;


};

