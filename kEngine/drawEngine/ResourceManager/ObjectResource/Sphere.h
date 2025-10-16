#pragma once
#include "VertexData.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexResource.h"
#include "Config.h"

class Sphere :public MeshBuffer
{
public:
	ID3D12Resource* CreateVertexResource_(ID3D12Device* device)override;
	ID3D12Resource* CreateIndexResource_(ID3D12Device* device)override;
	ID3D12Resource* SetWVPResource_(ID3D12Device* device, TransformationMatrix* wvpData);
	int GetVertexNum() { return VertexNum_; }

private:
	void SetWVP(TransformationMatrix* wvpData);
	int kMaxSudivision_ = 18;
	int kSudivision_ = 0;
	int VertexNum_ = 0;
};

