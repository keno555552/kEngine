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
	MSG msg = {};



	SceneManager* sceneManager = new SceneManager(system);

	///========================Main処理=====================///
	while (true) {
		// ゲームの処理で固定サイズ整数型を使用
		int32_t score = 0;   // 32ビット幅の符号付き整数

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
