#pragma once
#include "VertexData.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexResource.h"
#include "Config.h"

class GameParticle :public MeshBuffer
{
public:
	void SetSize(Vector2 LTpos, Vector2 LBpos, Vector2 RTpos, Vector2 RBpos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos);
	ID3D12Resource* CreateVertexResource_(ID3D12Device* device)override;
	ID3D12Resource* CreateIndexResource_(ID3D12Device* device)override;
	ID3D12Resource* SetWVPResource_(ID3D12Device* device, Vector2 pos);

	bool CheckSize(Vector2 LTpos, Vector2 LBpos, Vector2 RTpos, Vector2 RBpos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos);
	void SetKeep(bool keep) { keep_ = keep; }
	bool GetKeep() { return keep_; }
	int GetNumInstance() { return kNumInstance; }

private:
	enum {
		TOP_LEFT = 0,
		BOTTOM_LEFT,
		BOTTOM_RIGHT,
		TOP_RIGHT,
		CENTER
	};
	void Set2DWVP(Vector2* wvpData);

private:
	Vector2	coner[4] = { {  0,	 0 },
						 {  0, 360 },
						 { 640,360 },
						 { 640,	 0 } };
	Vector2 center = { 320,180 };
	float		TsizeX_ = {};
	float		TsizeY_ = {};
	Vector2 TexcoordLT_ = { 0,0 };
	Vector2 TexcoordRB_ = { 1,1 };

	int kNumInstance = 10;

	bool keep_ = false;
};

