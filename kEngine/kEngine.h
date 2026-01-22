#pragma once
#include "DirectXController.h"
#include "inputManager.h"
#include "soundManager.h"
#include "SrvManager.h"
#include "drawEngine.h"
#include "TransformationMatrix.h"
#include "Vector2.h"
#include "materialconfig.h"
#include "tool/TimeManager/TimeManager.h"
#include "DrawDataCollector.h"
#include "CameraManager.h"

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif // USE_IMGUI
#include <string>
#include "DrawData/ObjectData.h"
#include "DrawData/SpriteData.h"
#include "InstanceManager.h"
#include "ResourceManager/ResourceManager.h"
#include "TextureManager/TextureManager.h"
#include "Data/DirectionalLightGPU.h"
#include "Camera.h"


class kEngine
{
public:
	kEngine();
	~kEngine();

#pragma region システム管理


	/// System関連

	void Initialize(const char* kClientTitle, int kClientWidth, int kClientHeight);

	void StartFrame();

	void EndFrame();

	void SetDirectionalLight(DirectionalLightGPU* light);

	bool ProcessMessage();

#pragma endregion

#pragma region 描画システム

	/// VVVV====== SpriteData/ObjectData描画,今上使えない ======VVVV///
	void Draw2D(SpriteData* spriteData);
	void Draw3D(ObjectData* objectData);

	int GetModelTextureHandle(int modelHandle, int part);

	int GetMutiModelNum(int modelHandle);
	int SetModelObj(std::string path);

	DebugCamera* CreateDebugCamera();
	Camera* CreateCamera();
	void DestroyCamera(Camera* camera);
	void SetCamera(Camera* camera);
	void ResetToDefaultCamera();

	int commonTextureHandleReader(int handle);
	int commonModelHandleReader(int handle);

	int LoadTexture(const std::string& filePath);

#pragma endregion

#pragma region 音関連

	/// 音関連

	int SoundLoadSE(const char* filename);
	void SoundPlaySE(int Handle, float volume = 1.0f);
	void SoundPlayBGM(int Handle, float volume = 1.0f);

	void SoundSetVolume(int Handle, float volume = 1.0f);
	void SoundPause(int Handle);
	void SoundContinue(int Handle);
	void SoundStop(int Handle);

	void SoundSetMasterVolume(float volume);
	void SoundSetSEVolume(float volume);
	void SoundSetBGMVolume(float volume);

	bool SoundIsPlaying(int Handle);
	float SoundGetVolume(int Handle);
	float SoundGetMasterVolume() const;
	float SoundGetBGMVolume() const;
	float SoundGetSEVolume() const;

	void SoundSetMute(int Handle, bool isMute);
	void SoundSetMasterMute(bool isMute);
	void SoundSetBGMMute(bool isMute);
	void SoundSetSEMute(bool isMute);

	bool SoundGetMute(int Handle);
	bool SoundGetMasterMute() const;
	bool SoundGetBGMMute()const;
	bool SoundGetSEMute()const;

#pragma endregion

#pragma region 入力関連

	/// 入力関連

	Vector2 GetMousePosVector2();
	int GetMousePosX();
	int GetMousePosY();
	int GetMousePosXIns();
	int GetMousePosYIns();
	bool GetTriggerOn(int key);
	bool GetTriggerOff(int key);
	bool GetIsPush(int key);

	bool GetMouseTriggerOn(int key);
	bool GetMouseTriggerOff(int key);
	bool GetMouseIsPush(int key);
	int GetMouseScroll();
	int GetMouseScrollOrigin();

	bool GetGamepadTriggerOn(int key);
	bool GetGamepadTriggerOff(int key);
	bool GetGamepadIsPush(int key);
	int GetGamepad4Direction();
	int GetGamepad4DirectionTriggerOn();
	int GetGamepad4DirectionTriggerOff();
	int GetGamepad8Direction();
	int GetGamepad8DirectionTriggerOn();
	int GetGamepad8DirectionTriggerOff();
	int GetGamepadDirectionFloat();
	float GetGamepadLStick01X();
	float GetGamepadLStick01Y();
	int GetGamepadLStickX();
	int GetGamepadLStickY();
	float GetGamepadRStick01X();
	float GetGamepadRStick01Y();
	int GetGamepadRStickX();
	int GetGamepadRStickY();
	float GetGamepadL201();
	float GetGamepadR201();
	int GetGamepadL2();
	int GetGamepadR2();

#pragma endregion

#pragma region TimeManager

	/// Timer
	float GetFPS();
	float GetFPSPerSecond();
	float GetDeltaTime();
	TimeManager* GetTimeManager()const;

#pragma endregion

private:
	/// ============ コアシステム ============///
	DirectXController* dxComm = nullptr;

	/// ============ 描画関連 ============///

	/// 資源管理
	SrvManager* srvManager = nullptr;
	ResourceManager* resourceManager = nullptr;
	InstanceManager* instanceManager = nullptr;
	TextureManager* textureManager = nullptr;
	DrawDataCollector* drawDataCollector = nullptr;

	/// カメラ管理
	CameraManager* cameraManager = nullptr;

	/// 描画ロジック
	DrawEngine* drawEngine = nullptr;

	/// ============ 入力関連 ============///
	InputManager* inputManager = nullptr;

	/// ============ 音関連 ============///
	SoundManager* soundManager = nullptr;

	/// ============ 時間関連 ============///
	TimeManager* timeManager = nullptr;
};