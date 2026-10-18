#pragma once
#include <string>
#include "MathsIncluder.h"

class Object;
class Transform;
class MaterialConfig;
class kEngine;

class ObjectEditor {
public:
	void Initialize(kEngine* system) { system_ = system; }   
	void ReleaseSolo();                                      
	void Draw();                 
	void ClearSelection() { selected_ = nullptr; }

private:
	kEngine* system_ = nullptr;                 // 借り

	/// セレクト用
	Object* selected_ = nullptr;
	int selectedPart_ = -1;
	std::string filter_;	
	bool soloEnabled_ = false;                   // 新增
	bool isolatePart_ = true;                   // 新增（取代 highlightPart_）

	/// 閲覧用
	void* previewTexture_ = nullptr;
	/// 閲覧用カメラ（DebugCameraと同じ自由カメラ）
	Vector3 camPos_{ 0.0f, 1.0f, -4.0f };
	Vector3 camRot_{ 0.245f, 0.0f, 0.0f };      // x=pitch(正で下向き), y=yaw
	Object* focusedObject_ = nullptr;           // 最後にフォーカスしたオブジェクト
	bool previewHovered_ = false;
	bool showWireframe_ = false;
	bool showAxis_ = true;

private:
	void ApplySolo();

	void UpdatePreviewCamera();     // 入力でカメラを動かす
	void FocusSelected();           // 選択中のオブジェクトの正面へ
	void SubmitPreviewRequest();    // DrawEngineへ描画申請

	void DrawObjectColumn();     
	void DrawPreviewColumn();    
	void DrawDetailColumn();     
	void DrawTransformBody(Transform& t);
	void DrawUVTransformBody(MaterialConfig& m);
};