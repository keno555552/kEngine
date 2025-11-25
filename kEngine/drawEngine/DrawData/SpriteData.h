#pragma once
#include <vector>
#include "MathsIncluder.h"
#include <memory>
#include "Transform.h"
#include "TransformationMatrix.h"
#include "MaterialConfig.h"
#include "DrawData/ObjectData.h"
#include "DrawData/CornerData.h"

struct SpritePart
{
	///================= デイタ保存 =================///
	Transform transform = CreateDefaultTransform();

	Transform worldTransform = CreateDefaultTransform();

	Vector2 anchorPoint = { 0.0f, 0.0f };

	CornerData conerData;

	///=============== 計算、資料渡し用 ==============///
	std::shared_ptr<MaterialConfig>materialConfig;

	ObjectPart* parentPart = nullptr;
};


#pragma region SimpleSprite
struct SimpleSpritePart : SpritePart
{
	
};

class SpriteData {
	public:
	std::vector<SpritePart> objectParts_;
	SpritePart mainPosition;
	int modelHandle_ = 0;
	bool isDelete_ = false;
	SpritePart* followObject_;
};

class SimpleSpriteData : public SpriteData {
public:

};
#pragma endregion



#pragma region DeformableSprite
struct DeformableSpriteVertex {
	Vector2 LTpos;
	Vector2 LBpos;
	Vector2 RTpos;
	Vector2 RBpos;
	float TsizeX;
	float TsizeY;
};

struct DeformableSpritePart : SpritePart
{

};

class DeformableSpriteData : public SpriteData {
public:
	std::vector<DeformableSpritePart> objectParts_;
	DeformableSpritePart mainPosition;
	int modelHandle_ = 0;
	bool isDelete_ = false;
	SpritePart* followObject_;
};
#pragma endregion