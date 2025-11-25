#pragma once
#include "VertexData.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexResource.h"
#include "Config.h"
#include "DrawData/CornerData.h"

class SimpleSpriteMesh :public MeshBuffer
{
public:
	void SetSize(Vector2 RBpos);
	ID3D12Resource* CreateVertexResource_(ID3D12Device* device)override;
	void Mapping();
	ID3D12Resource* CreateIndexResource_(ID3D12Device* device)override;

	int GetVertexNum() { return 6; }
	bool CheckSize(Vector2 RBpos);
	void SetKeep(bool keep) { keep_ = keep; }
	bool GetKeep() { return keep_; }

	bool operator==(const SimpleSpriteMesh target);

private:
	CornerData conerData;
	Vector2 TexcoordLT_ = {0,0};
	Vector2 TexcoordRB_ = {1,1};

	bool keep_ = false;
};

