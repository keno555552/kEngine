#pragma once
#include "DircetXController.h"
#include "inputManager.h"
#include "soundManager.h"
#include "drawEngine.h"
#include "TransformationMatrix.h"
#include "Vector2.h"
#include "materialconfig.h"
#include "tool/TimeManager/TimeManager.h"

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

	void SetDirectionalLight(DirectionalLight* light);

	bool ProcessMessage();

#pragma endregion

#pragma region 描画システム

	/// <summary>
	/// 3D 三角形を描く
	/// </summary>
	/// <param name="wvpData">: 計算終わったTransformMatrix</param>
	/// <param name="material">: MaterialConfig </param>
	void DrawTriangle(TransformationMatrix* wvpData, MaterialConfig material);

	/// <summary>
	/// 2D 四角形を描く, 左上が0,0, 右下プラス, 座標はピクセル計算
	/// </summary>
	/// <param name="pos"> Vector2座標</param>
	/// <param name="material"> MaterialConfig </param>
	void DrawSprite(Vector2 pos, MaterialConfig material);

	void DrawTile(Vector2 pos, MaterialConfig material);

	/// <summary>
	/// 2D 四角形を描く, 左上が0,0, 右下プラス, 座標はピクセル計算, 
	/// </summary>
	/// <param name="pos"> Vector2座標</param>
	/// <param name="material"> MaterialConfig </param>
	/// <param name="LTpos">描くどころの左上座標</param>
	/// <param name="LBpos">描くどころの左下座標</param>
	/// <param name="RTpos">描くどころの右上座標</param>
	/// <param name="RBpos">描くどころの右下座標</param>
	/// <param name="TsizeX">textrueのサイズx</param>
	/// <param name="TsizeY">textrueのサイズy</param>
	/// <param name="TCLTPos">textrueに対するカットする左上</param>
	/// <param name="TCRBPos">textrueに対するカットする右下</param>
	void DrawSprite(Vector2 pos, MaterialConfig material, Vector2 LTpos, Vector2 LBpos, Vector2 RTpos, Vector2 RBpos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos);

	/// <summary>
	/// 3D 立方体を描く
	/// </summary>
	/// <param name="wvpData">: 計算終わったTransformMatrix</param>
	/// <param name="material">: MaterialConfig </param>
	void DrawCube(TransformationMatrix* wvpData, MaterialConfig material);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="wvpData">: 計算終わったTransformMatrix</param>
	/// <param name="material">: MaterialConfig </param>
	void DrawSprete(TransformationMatrix* wvpData, MaterialConfig material);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="wvpData"></param>
	/// <param name="material"></param>
	/// <param name="modelHandle"></param>
	void DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle);

	void DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="wvpData"></param>
	/// <param name="material"></param>
	void Draw3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material);

	int GetMuitModelNum(int modelHandle);
	int SetModelObj(std::string path);

	int commonTextureHandleReader(int handle);
	int commonModelHandleReader(int handle);

	int LoadTextrue(const std::string& filePath);

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
	DircetXController* dxComm = nullptr;
	DrawEngine* drawEngine = nullptr;
	InputManager* inputManager = nullptr;
	SoundManager* soundManager = nullptr;
	TimeManager* timeManager = nullptr;

};