#include "kEngine.h"

#pragma region システム管理

kEngine::kEngine() {
	dxComm = new DircetXCommen;
	drawEngine = new DrawEngine;
	inputManager = new InputManager;
	soundManager = new SoundManager;
	timeManager = new TimeManager;

	///Lighting
	DirectionalLight directionalLight{
		.color{1.0f,1.0f,1.0f,1.0f},
		.direction{-0.5f,-0.5f,0.4f},
		.intensity{1.0f}
	};
	SetDirectionalLight(&directionalLight);
}

kEngine::~kEngine() {
	delete drawEngine;
	delete inputManager;
	delete soundManager;
	delete timeManager;
	delete dxComm;
}

void kEngine::Initialize(const char* kClientTitle, int kClientWidth, int kClientHeight) {
	dxComm->InitializeDrive(kClientTitle, kClientWidth, kClientHeight);
	drawEngine->Initialize(kClientTitle, kClientWidth, kClientHeight, dxComm);
	inputManager->Initialize(dxComm, timeManager);
}

void kEngine::StartFrame() {
	dxComm->StartFrame();
	drawEngine->PreDraw();
	inputManager->KeysUpdata();
	timeManager->Update();
}

void kEngine::EndFrame() {
	drawEngine->CommitDraw();
	dxComm->EndFrame();
	drawEngine->EndDraw();
}

#pragma endregion

#pragma region 描画システム

void kEngine::SetDirectionalLight(DirectionalLight* light) {
	drawEngine->SetDirectionalLight(light);
}

void kEngine::DrawTriangle(TransformationMatrix* wvpData, MaterialConfig material) {
	drawEngine->DrawTriangle(wvpData, material);
}

void kEngine::DrawSprite(Vector2 pos, MaterialConfig material) {
	//drawEngine->DrawSpriteDirect(pos, material);
	drawEngine->CollectSprite(pos, material);
}

void kEngine::DrawSprite(Vector2 pos, MaterialConfig material, Vector2 LTpos, Vector2 LBpos, Vector2 RTpos, Vector2 RBpos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos) {
	drawEngine->DrawSpriteDirect(pos, material, LTpos, LBpos, RTpos, RBpos, TsizeX, TsizeY, TCLTPos, TCRBPos);
}

void kEngine::DrawTile(Vector2 pos, MaterialConfig material) {
	drawEngine->Collect2DTile(pos, material);
}

void kEngine::DrawCube(TransformationMatrix* wvpData, MaterialConfig material) {
	drawEngine->CollectCube(wvpData, material);
}

void kEngine::DrawSprete(TransformationMatrix* wvpData, MaterialConfig material) {
	drawEngine->DrawSphere(wvpData, material, 1);
}

void kEngine::DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle) {
	//drawEngine->DrawModel(wvpData, material, modelHandle);
	drawEngine->CollectModel(wvpData, material, modelHandle);
}

void kEngine::DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material) {
	drawEngine->CollectModel(wvpData, material);
}

void kEngine::Draw3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material) {
	drawEngine->Collect3DTile(wvpData, material);
}

int kEngine::GetMuitModelNum(int modelHandle) {
	return drawEngine->GetMuitModelNum(modelHandle);
}

int kEngine::SetModelObj(std::string path) {
	return drawEngine->SetModel(path);
}

int kEngine::commonTextureHandleReader(int handle) {
	return drawEngine->readCommenTextureHandle(handle);
};
int kEngine::commonModelHandleReader(int handle) {
	return drawEngine->readModelTextureHandle(handle);
};

int kEngine::LoadTextrue(const std::string& filePath) {
	int handle = drawEngine->LoadTexture(filePath);
	return handle;
}

#pragma endregion

#pragma region 音関連

int kEngine::SoundLoadSE(const char* filename) {
	return soundManager->SoundLoadSE(filename);
}

void kEngine::SoundPlaySE(int Handle, float volume) {
	soundManager->SoundPlaySE(Handle, volume);
}

void kEngine::SoundPlayBGM(int Handle, float volume) {
	soundManager->SoundPlayBGM(Handle, volume);
}

void kEngine::SoundSetVolume(int Handle, float volume) {
	soundManager->SoundSetVolume(Handle, volume);
}

void kEngine::SoundPause(int Handle) {
	soundManager->SoundPause(Handle);
}

void kEngine::SoundContinue(int Handle) {
	soundManager->SoundContinue(Handle);
}

void kEngine::SoundStop(int Handle) {
	soundManager->SoundStop(Handle);
}

void kEngine::SoundSetMasterVolume(float volume) {
	soundManager->SoundSetMasterVolume(volume);
}

void kEngine::SoundSetSEVolume(float volume) {
	soundManager->SoundSetMasterSEVolume(volume);
}

void kEngine::SoundSetBGMVolume(float volume) {
	soundManager->SoundSetMasterBGMVolume(volume);
}

bool kEngine::SoundIsPlaying(int Handle) {
	if (soundManager->isPlaying(Handle))return true;
	return false;
}

float kEngine::SoundGetVolume(int Handle) { return soundManager->SoundGetVolume(Handle); }
float kEngine::SoundGetMasterVolume() const { return soundManager->SoundGetMasterVolume(); }
float kEngine::SoundGetBGMVolume() const { return soundManager->SoundGetBGMVolume(); }
float kEngine::SoundGetSEVolume() const { return soundManager->SoundGetSEVolume(); }

void kEngine::SoundSetMute(int Handle, bool isMute) { soundManager->SoundSetMute(Handle, isMute); }
void kEngine::SoundSetMasterMute(bool isMute) { soundManager->SoundSetMasterMute(isMute); }
void kEngine::SoundSetBGMMute(bool isMute) { soundManager->SoundSetBGMMute(isMute); };
void kEngine::SoundSetSEMute(bool isMute) { soundManager->SoundSetSEMute(isMute); };

bool kEngine::SoundGetMute(int Handle) { return soundManager->SoundGetMute(Handle); }
bool kEngine::SoundGetMasterMute() const { return soundManager->SoundGetMasterMute(); }
bool kEngine::SoundGetBGMMute()const { return soundManager->SoundGetBGMMute(); }
bool kEngine::SoundGetSEMute()const { return soundManager->SoundGetSEMute(); }


#pragma endregion

#pragma region 入力関連

int kEngine::GetMousePosX() {
	return inputManager->mousePosX();
}

int kEngine::GetMousePosY() {
	return inputManager->mousePosY();
}

int kEngine::GetMousePosXIns() {
	return inputManager->mousePosXIns();
}

int kEngine::GetMousePosYIns() {
	return inputManager->mousePosYIns();
}

bool kEngine::GetTriggerOn(int key) {
	if (inputManager->keyTriggerOn(key)) { return true; }
	return false;
}

bool kEngine::GetTriggerOff(int key) {
	if (inputManager->keyTriggerOff(key)) { return true; }
	return false;
}

bool kEngine::GetIsPush(int key) {
	if (inputManager->keyIsPush(key)) { return true; }
	return false;
}

bool kEngine::GetMouseTriggerOn(int key) {
	if (inputManager->mouseTriggerOn(key)) { return true; }
	return false;
}

bool kEngine::GetMouseTriggerOff(int key) {
	if (inputManager->mouseTriggerOff(key)) { return true; }
	return false;
}

bool kEngine::GetMouseIsPush(int key) {
	if (inputManager->mouseIsPush(key)) { return true; }
	return false;
}

int kEngine::GetMouseScroll() {
	return inputManager->mouseScrollSpeed();
}

int kEngine::GetMouseScrollOrigin() {
	return inputManager->mouseScrollSpeedOrigin();
}

bool kEngine::GetGamepadTriggerOn(int key) {
	return inputManager->gamepadTriggerOn(key);
}

bool kEngine::GetGamepadTriggerOff(int key) {
	return inputManager->gamepadTriggerOff(key);
}

bool kEngine::GetGamepadIsPush(int key) {
	return inputManager->gamepadIsPush(key);
}

int kEngine::GetGamepad4Direction() {
	return inputManager->gamepad4Direction();
}

int kEngine::GetGamepad4DirectionTriggerOn() {
	return inputManager->gamepad4DirectionTriggerOn();
}

int kEngine::GetGamepad4DirectionTriggerOff() {
	return inputManager->gamepad4DirectionTriggerOff();
}

int kEngine::GetGamepad8Direction() {
	return inputManager->gamepad8Direction();
}

int kEngine::GetGamepad8DirectionTriggerOn() {
	return inputManager->gamepad8DirectionTriggerOn();
}

int kEngine::GetGamepad8DirectionTriggerOff() {
	return inputManager->gamepad8DirectionTriggerOff();
}

int kEngine::GetGamepadDirectionFloat() {
	return inputManager->gamepadDirectionFloat();
}

float kEngine::GetGamepadLStick01X() {
	return inputManager->gamepadLStick01X();
}

float kEngine::GetGamepadLStick01Y() {
	return inputManager->gamepadLStick01Y();
}

int kEngine::GetGamepadLStickX() {
	return inputManager->gamepadLStickX();
}

int kEngine::GetGamepadLStickY() {
	return inputManager->gamepadLStickY();
}

float kEngine::GetGamepadRStick01X() {
	return inputManager->gamepadRStick01X();
}

float kEngine::GetGamepadRStick01Y() {
	return inputManager->gamepadRStick01Y();
}

int kEngine::GetGamepadRStickX() {
	return inputManager->gamepadRStickX();
}

int kEngine::GetGamepadRStickY() {
	return inputManager->gamepadRStickY();
}

float kEngine::GetGamepadL201() {
	return inputManager->gamepadL201();
}

float kEngine::GetGamepadR201() {
	return inputManager->gamepadR201();
}

int kEngine::GetGamepadL2() {
	return inputManager->gamepadL2();
}

int kEngine::GetGamepadR2() {
	return inputManager->gamepadR2();
}


#pragma endregion

#pragma region TimeManager

float kEngine::GetFPS() {
	return timeManager->getInstantFPS();
}

float kEngine::GetFPSPerSecond() {
	return timeManager->getFPSPerSecond();
}

float kEngine::GetDeltaTime() {
	return timeManager->getDeltaTime();
}

TimeManager* kEngine::GetTimeManager() const {
	return timeManager;
}


#pragma endregion