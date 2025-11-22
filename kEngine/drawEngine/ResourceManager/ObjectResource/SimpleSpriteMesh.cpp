#include "SimpleSpriteMesh.h"

void SimpleSpriteMesh::SetSize(Vector2 RBpos) {
	coner[TOP_LEFT]		= { 0,		  0 };
	coner[BOTTOM_LEFT]	= { 0,	RBpos.y };
	coner[BOTTOM_RIGHT] = RBpos;
	coner[TOP_RIGHT]	= { RBpos.x,  0 };

	TexcoordLT_.x = 0;
	TexcoordLT_.y = 0;
	TexcoordRB_.x = 1;
	TexcoordRB_.y = 1;

	Mapping();
}

ID3D12Resource* SimpleSpriteMesh::CreateVertexResource_(ID3D12Device* device) {

	vertexResource_->CreateResourceClass_(device, sizeof(VertexData) * 4);

	Mapping();

	return vertexResource_->GetResource().Get();
}

void SimpleSpriteMesh::Mapping() {
	VertexData* vertexDataSprite = nullptr;
	vertexResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	// 四点定義
	vertexDataSprite[TOP_LEFT].position = { coner[BOTTOM_LEFT].x, coner[BOTTOM_LEFT].y, 0.0f, 1.0f };
	vertexDataSprite[TOP_LEFT].texcoord = { TexcoordLT_.x, TexcoordRB_.y };
	vertexDataSprite[TOP_LEFT].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[BOTTOM_LEFT].position = { coner[TOP_LEFT].x, coner[TOP_LEFT].y, 0.0f, 1.0f };
	vertexDataSprite[BOTTOM_LEFT].texcoord = { TexcoordLT_.x, TexcoordLT_.y };
	vertexDataSprite[BOTTOM_LEFT].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[BOTTOM_RIGHT].position = { coner[TOP_RIGHT].x, coner[TOP_RIGHT].y, 0.0f, 1.0f };
	vertexDataSprite[BOTTOM_RIGHT].texcoord = { TexcoordRB_.x, TexcoordLT_.y };
	vertexDataSprite[BOTTOM_RIGHT].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[TOP_RIGHT].position = { coner[BOTTOM_RIGHT].x, coner[BOTTOM_RIGHT].y, 0.0f, 1.0f };
	vertexDataSprite[TOP_RIGHT].texcoord = { TexcoordRB_.x, TexcoordRB_.y };
	vertexDataSprite[TOP_RIGHT].normal = { 0.0f, 0.0f, -1.0f };
	vertexResource_->GetResource()->Unmap(0, nullptr);
}

ID3D12Resource* SimpleSpriteMesh::CreateIndexResource_(ID3D12Device* device) {
	indexResource_->CreateResourceClass_(device, sizeof(uint32_t) * 6);

	indexBufferView = {};
	/// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource_->GetResource()->GetGPUVirtualAddress();
	/// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	///　インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexDataSprite = nullptr;
	indexResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 順番を指定
	indexDataSprite[0] = TOP_LEFT;
	indexDataSprite[1] = BOTTOM_LEFT;
	indexDataSprite[2] = BOTTOM_RIGHT;
	indexDataSprite[3] = TOP_LEFT;
	indexDataSprite[4] = BOTTOM_RIGHT;
	indexDataSprite[5] = TOP_RIGHT;
	indexResource_->GetResource()->Unmap(0, nullptr);
	return indexResource_->GetResource().Get();
}

bool SimpleSpriteMesh::CheckSize(Vector2 RBpos) {
	if (coner[BOTTOM_RIGHT].x == RBpos.x)return false;
	if (coner[BOTTOM_RIGHT].y == RBpos.y)return false;
	return true;
}

bool SimpleSpriteMesh::operator==(const SimpleSpriteMesh target) {
	int i = 0;
	for (auto& cornerPos : coner) { if (cornerPos != target.coner[i])return false; }
	TexcoordLT_ = target.TexcoordLT_;
	TexcoordRB_ = target.TexcoordRB_;
	return true;
}


