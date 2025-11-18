#include "kEngine.h"			//kEngine
#include "Transform.h"			//Transform.h
#include <cstdint>
#include <windows.h>
#include "DirectionalLight.h"
#include <string>
#include <vector>
#include "SceneManager.h"

const char kWindowTitle[] = "ENGINE";

const int32_t kWindowWidth = 1280;
const int32_t kWindowHeight = 720;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	CoInitializeEx(0, COINIT_MULTITHREADED);
	kEngine* system(new kEngine);
	system->Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	SceneManager* sceneManager = new SceneManager(system);

	///========================Main処理=====================///
	while (true) {

		///====================ゲーム処理====================///
		sceneManager->Update();


		///=====================描画処理=====================///
		system->StartFrame();

		sceneManager->Render();

		system->EndFrame();

		///// 終了条件
		if (system->GetTriggerOn(DIK_ESCAPE) ||
			system->GetGamepadTriggerOn(VK_PAD_START) ||
			system->ProcessMessage()) {
			break;
		}
	}
	delete system;
	return 0;
}
