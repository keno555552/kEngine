#include "kEngine.h"

#pragma region システム管理

kEngine::kEngine() {
}


kEngine::~kEngine() {

#ifdef USE_IMGUI
	ImGuiManager::Shutdown();
#endif
	delete timeManager;
	delete soundManager;
	delete inputManager;
	TextureManager::GetInstance()->Finalize();
	delete instanceManager;
	delete resourceManager;
	delete drawEngine;
	delete drawDataCollector;
	delete cameraManager;
	delete srvManager;
	delete dxComm;
}

void kEngine::Initialize(const char* kClientTitle, int kClientWidth, int kClientHeight) {
	config::SaveClientTitle(kClientTitle);
	config::SaveClientWidth(kClientWidth);
	config::SaveClientHeight(kClientHeight);

	dxComm = new DirectXController;
	dxComm->InitializeDrive(kClientTitle, kClientWidth, kClientHeight);

	srvManager = new SrvManager;
	srvManager->Initialize(dxComm);

#ifdef USE_IMGUI
	ImGuiManager::Initialize(dxComm, srvManager);
#endif

	TextureManager::GetInstance()->Initialize(dxComm, srvManager);
	instanceManager = new InstanceManager();
	resourceManager = new ResourceManager(dxComm, instanceManager);

	cameraManager = new CameraManager;

	drawDataCollector = new DrawDataCollector(resourceManager, instanceManager, cameraManager);
	drawEngine = new DrawEngine;
	drawEngine->Initialize(dxComm, srvManager, resourceManager, drawDataCollector);

	soundManager = new SoundManager;
	timeManager = new TimeManager;

	inputManager = new InputManager;
	inputManager->Initialize(dxComm, timeManager);
}

void kEngine::StartFrame() {
	dxComm->StartFrame();
	drawEngine->PreDraw();
	inputManager->KeysUpdata();
	timeManager->Update();
	drawDataCollector->PreCollect();
}

void kEngine::EndFrame() {
	drawEngine->CommitDraw();
	dxComm->EndFrame();
	drawEngine->EndDraw();
}

void kEngine::SetDirectionalLight(DirectionalLight* light) {
	drawEngine->SetDirectionalLight(light);
}

bool kEngine::ProcessMessage() {
	return dxComm->ProcessMessage();
}

#pragma endregion

#pragma region 描画システム

void kEngine::Draw2D(SpriteData* spriteData) {
	//resourceManager->Collet2D(spriteData);
	drawDataCollector->Collect2D(spriteData);
}

void kEngine::Draw3D(ObjectData* object) {
	//resourceManager->Collet3D(object);
	drawDataCollector->Collect3D(object);
}

int kEngine::GetModelTextureHandle(int modelHandle, int part) {
	return resourceManager->GetTextureHandleFromModelGroup(modelHandle, part);
}

int kEngine::GetMutiModelNum(int modelHandle) {
	return resourceManager->modelGroupList_[modelHandle]->GetModelNum();
}

int kEngine::SetModelObj(std::string path) {
	//return drawEngine->SetModel(path);
	return resourceManager->LoadModel(path);
}

DebugCamera* kEngine::CreateDebugCamera() {
	return cameraManager->CreateDebugCamera(this);
}

Camera* kEngine::CreateCamera() {
	return cameraManager->CreateCamera();
}

void kEngine::DestroyCamera(Camera* camera) {
	cameraManager->DestroyCamera(camera);
}

void kEngine::SetCamera(Camera* camera) {
	cameraManager->SetActiveCamera(camera);
}

void kEngine::ResetToDefaultCamera() {
	cameraManager->ResetActiveCamera();
}


int kEngine::commonTextureHandleReader(int handle) {
	return resourceManager->GetTextureHandleFromCommonList(handle);
};
int kEngine::commonModelHandleReader(int handle) {
	return resourceManager->ReadModelTextureHandle(handle);
};

int kEngine::LoadTexture(const std::string& filePath) {
	return resourceManager->LoadCommonTexture(filePath);
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

Vector2 kEngine::GetMousePosVector2() {
	return Vector2{ (float)inputManager->mousePosX(), (float)inputManager->mousePosY() };
}

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