#pragma once
#include "VertexData.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexResource.h"
#include "Config.h"

class SimpleSpriteMesh :public MeshBuffer
{
public:
	void SetSize(Vector2 RBpos);
	ID3D12Resource* CreateVertexResource_(ID3D12Device* device)override;
	ID3D12Resource* CreateIndexResource_(ID3D12Device* device)override;

	bool CheckSize(Vector2 RBpos);
	void SetKeep(bool keep) { keep_ = keep; }
	bool GetKeep() { return keep_; }

	bool operator==(const SimpleSpriteMesh target);

private:
	enum {
		TOP_LEFT = 0,
		BOTTOM_LEFT,
		BOTTOM_RIGHT,
		TOP_RIGHT,
		CENTER
	};

private:
	Vector2	coner[4] = { {  0,	 0 },
						 {  0, 1 },
						 { 1,1 },
						 { 1,	 0 }};
	Vector2 TexcoordLT_ = {0,0};
	Vector2 TexcoordRB_ = {1,1};

	bool keep_ = false;
};

