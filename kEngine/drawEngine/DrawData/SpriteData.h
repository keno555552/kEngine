#pragma once
#include <vector>
#include "MathsIncluder.h"
#include <memory>
#include "Transform.h"
#include "TransformationMatrix.h"
#include "MaterialConfig.h"
#include "DrawData/ObjectData.h"

struct SpritePart
{
	Transform transform = CreateDefaultTransform();

	TransformationMatrix transformationMatrix{};

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
	std::vector<SimpleSpritePart> objectParts_;
	SimpleSpritePart mainPosition;
	int modelHandle_ = 0;
	bool isDelete_ = false;
	SpritePart* followObject_;
};
#pragma endregion



#pragma region DeformableSprite
struct SimpleSprite {
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