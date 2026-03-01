#include "kEngine.h"			//kEngine
#include "Transform.h"			//Transform.h
#include <cstdint>
#include <windows.h>
#include "Data/DirectionalLightGPU.h"
#include <string>
#include <vector>
#include "SceneManager.h"

const char kWindowTitle[] = "LE2B_05_カン_ケンリャン";

const int32_t kWindowWidth = 1280;
const int32_t kWindowHeight = 720;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	CoInitializeEx(0, COINIT_MULTITHREADED);
	kEngine* system(new kEngine);
	system->Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	//SceneManager* sceneManager = new SceneManager(system);
	SceneManager::Initialize(system);


	///========================Main処理=====================///
	while (system->ProcessMessage() && kEngine::GameOn()) {

		///====================ゲーム処理====================///
		SceneManager::GetInstance().Update();

		///=====================描画処理=====================///
		system->StartFrame();

		SceneManager::GetInstance().Render();

		system->EndFrame();

	}
	delete system;
	return 0;
}
