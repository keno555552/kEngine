#include "SceneTester.h"


SceneTester::SceneTester(kEngine* system) {
	system_ = system;
	float kWindowWidth = (float)config::GetClientWidth();
	float kWindowHeight = (float)config::GetClientHeight();
	system_->SetDirectionalLight(&directionalLight);
	debugCamera_ = new DebugCamera(system_, kWindowWidth, kWindowHeight);


	bunnyModelHandle_			= system_->SetModelObj("resources/TemplateResource/object/bunny/bunny.obj");
	teapotModelHandle_			= system_->SetModelObj("resources/TemplateResource/object/teapot/teapot.obj");
	suzanneModelHandle_			= system_->SetModelObj("resources/TemplateResource/object/suzanne/suzanne.obj");
	multiMeshModelHandle_		= system_->SetModelObj("resources/TemplateResource/object/multiMesh/multiMesh.obj");
	multiMaterialModelHandle_	= system_->SetModelObj("resources/TemplateResource/object/multiMaterial/multiMaterial.obj");
	charaterModelHandle_		= system_->SetModelObj("resources/TemplateResource/object/charater/charater.obj");
	needleModelHandle_			= system_->SetModelObj("resources/TemplateResource/object/needle/needle_Body.obj");
	skydomeModelHandle_			= system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");

	uvCheckerHandle1_			= system_->LoadTextrue("resources/TemplateResource/texture/uvChecker.png");
	monsterBallHandle_			= system_->LoadTextrue("resources/TemplateResource/texture/monsterBall.png");
	objHandle_					= system_->LoadTextrue("resources/TemplateResource/texture/obj.png");
	whiteHandle_				= system_->LoadTextrue("resources/TemplateResource/texture/white5x5.png");

	se1_						= system_->SoundLoadSE("resources/TemplateResource/sound/SE/game_start.wav");
	se2_						= system_->SoundLoadSE("resources/TemplateResource/sound/SE/take.wav");
	//{
	//	Object* object = new Object;
	//	object->objectType = MODEL;
	//	MaterialConfig materialConfig;
	//	for (int i = 0; i < system->GetMuitModelNum((int)ModelType::CHARATER); i++) {
	//		MaterialConfig materialConfig;
	//		intMaterialConfig(&materialConfig);
	//		object->materialConfig.push_back(materialConfig);
	//	}
	//	object->modelHandle = ModelType::CHARATER;
	//	objectGroup.push_back(object);
	//}
	{
		InstanceObject* object = new InstanceObject;
		object->objectType = MODEL;
		object->modelHandle = ModelType::SKYDOME;
		MaterialConfig materialConfig;
		InitMaterialConfig(&materialConfig);
		materialConfig.lightModelType = LightModelType::HalfLambert;
		materialConfig.enableLighting = false;
		object->materialConfig.push_back(materialConfig);
		objectGroup.push_back(object);
	}

	//{
	//	MaterialConfig materialConfig;
	//	InitMaterialConfig(&materialConfig);
	//	player->materialConfig.push_back(materialConfig);
	//	player->objectType = MODEL;
	//	player->modelHandle = ModelType::CHARATER;
	//	player->transform.translate = { 0, 0, 0 };
	//	player->transformationMatrix = debugCamera_->transformationMatrixTransform(player->transform);
	//	objectGroup.push_back(player);
	//
	//}

	//for (int i = 0; i < 1; i++) 
	////for (int i = 0; i < 10; i++) 
	for (int i = 0; i < 5; i++) 
	{
		InstanceObject* object = new InstanceObject;
		MaterialConfig materialConfig;
		InitMaterialConfig(&materialConfig);
		object->materialConfig.push_back(materialConfig);
		object->objectType = TILE;
		//object->objectType = SPRITE;
		object->transform.translate = { i * 100.0f, i * 100.0f, 0.0f };
		object->transformationMatrix = debugCamera_->transformationMatrixTransform(object->transform);
		objectGroup.push_back(object);
	}
	//{
	//	Object* object = new Object;
	//	MaterialConfig materialConfig;
	//	InitMaterialConfig(&materialConfig);
	//	object->materialConfig.push_back(materialConfig);
	//	//object->objectType = TILE;
	//	object->objectType = SPRITE;
	//	object->transform.translate = { 0,0,0 };
	//	object->transformationMatrix = debugCamera_->transformationMatrixTransform(object->transform);
	//	objectGroup.push_back(object);
	//}

	//for (int i = 0; i < 1; i++) 
	//for (int i = 0; i < 10; i++) 
	//for (int i = 0; i < 200; i++) 
	// {
	//	Object* object = new Object;
	//	MaterialConfig materialConfig;
	//	InitMaterialConfig(&materialConfig);
	//	object->materialConfig.push_back(materialConfig);
	//	//object->objectType = CUBE;
	//	object->objectType = MODEL;
	//	//object->modelHandle = ModelType::TEAPOT;
	//	object->modelHandle = ModelType::TILE3D;
	//	//object->objectType = SPRITE;
	//	object->transform.translate = { i * 0.1f,i * 0.1f, i * 1.0f };
	//	object->transformationMatrix = debugCamera_->transformationMatrixTransform(object->transform);
	//	objectGroup.push_back(object);
	//}

	timer_->Init0(60.0f,system->GetTimeManager());
};


SceneTester::~SceneTester() {
	if (!objectGroup.empty()) {
		for (auto& ptr : objectGroup) {
			delete ptr, ptr = nullptr;
		}
	}
	objectGroup.clear();
}

void SceneTester::Update() {

	///====================入力処理====================///
	{
		int handle = 0;
		if (textSubject == 0) { handle = VK_PAD_X; }				/// Button[00] = Direct(PS4): []
		if (textSubject == 1) { handle = VK_PAD_A; }				/// Button[01] = Direct(PS4): X
		if (textSubject == 2) { handle = VK_PAD_B; }				/// Button[02] = Direct(PS4): 0
		if (textSubject == 3) { handle = VK_PAD_Y; }				/// Button[03] = Direct(PS4): A
		if (textSubject == 4) { handle = VK_PAD_RSHOULDER; }		/// Button[04] = Direct(PS4): L1
		if (textSubject == 5) { handle = VK_PAD_LSHOULDER; }		/// Button[05] = Direct(PS4): R1
		if (textSubject == 6) { handle = VK_PAD_LTRIGGER; }			/// Button[06] = Direct(PS4): L2
		if (textSubject == 7) { handle = VK_PAD_RTRIGGER; }			/// Button[07] = Direct(PS4): R2
		if (textSubject == 8) { handle = VK_PAD_DPAD_UP; }
		if (textSubject == 9) { handle = VK_PAD_DPAD_DOWN; }
		if (textSubject == 10) { handle = VK_PAD_DPAD_LEFT; }
		if (textSubject == 11) { handle = VK_PAD_DPAD_RIGHT; }
		if (textSubject == 12) { handle = VK_PAD_START; }			/// Button[08] = Direct(PS4): Share
		if (textSubject == 13) { handle = VK_PAD_BACK; }			/// Button[09] = Direct(PS4): Start
		if (textSubject == 14) { handle = VK_PAD_LTHUMB_PRESS; }	/// Button[10] = Direct(PS4): 左ステック押し
		if (textSubject == 15) { handle = VK_PAD_RTHUMB_PRESS; }	/// Button[11] = Direct(PS4): 右ステック押し
		if (textSubject > 16) { handle = textSubject; }

		if (system_->GetGamepadTriggerOn(handle)) {
			A++;
		}
		//A = system->GetGamepadL201();

		if (system_->GetGamepadTriggerOff(handle)) {
			B++;
		}
		//B = system->GetGamepadR201();

		if (system_->GetGamepadIsPush(handle)) {
			C++;
		}
		//C = system->GetGamepadL2();
		//E = system->GetGamepadR2();
	}

	D = config::GetGamePadType();

	if (system_->GetTriggerOn(DIK_SPACE) ||
		system_->GetGamepadTriggerOn(VK_PAD_A)) {
		system_->SoundPlaySE(se1_, seVolume);
	}

	if (system_->GetTriggerOn(DIK_L) ||
		system_->GetGamepadTriggerOn(VK_PAD_B)) {
		system_->SoundPlayBGM(se2_, iVolume);
	}

	if (system_->GetTriggerOn(DIK_K)) {
		if (system_->SoundIsPlaying(se2_)) {
			system_->SoundPause(se2_);
		} else {
			system_->SoundContinue(se2_);
		}
	}

	if (system_->GetTriggerOn(DIK_I)) {
		system_->SoundStop(se2_);
	}


	system_->SoundSetVolume(se2_, iVolume);
	system_->SoundSetMasterVolume(mVolume);
	system_->SoundSetSEVolume(seVolume);
	system_->SoundSetBGMVolume(bgmVolume);

	system_->SoundSetMute(se2_, iMute);
	system_->SoundSetMasterMute(masterMute);
	system_->SoundSetBGMMute(bgmMute);
	system_->SoundSetSEMute(seMute);


	/// カメラ移動

	///==================カメラアップデート=================///
	debugCamera_->MouseControlUpdate();
	debugCamera_->Update();

	//player->materialConfig[0].uvRotate.z += 0.5f;

	for (auto& ptr : objectGroup) {
		ptr->transformationMatrix = debugCamera_->transformationMatrixTransform(ptr->transform);
		for (auto& ptr2 : ptr->materialConfig) {
			ptr2.uvTransformMatrix = MakeAffineMatrix(ptr2.uvScale,
				ptr2.uvRotate,
				ptr2.uvTranslate);
		}
	}

	for (int i = 0; i < objectGroup.size(); i++) {
		if (objectGroup[i]->isDelete == true) {
			delete objectGroup[i];
			objectGroup.erase(objectGroup.begin() + i);
		}
	}

	if (system_->GetTriggerOn(DIK_Q)) {
		timer_->parameter_ = 0;
	}
	timer_->ToMix();
}

void SceneTester::Draw() {

	///=====================描画処理=====================///

	if (!objectGroup.empty()) {
		for (auto& ptr : objectGroup) {
			switch (ptr->objectType) {
			case TRIANGLE:
				system_->DrawTriangle(&ptr->transformationMatrix, ptr->materialConfig[0]);
				break;
			case SPRITE:
				system_->DrawSprite({ ptr->transform.translate.x,ptr->transform.translate.y }, ptr->materialConfig[0]);
				break;
			case TILE:
				system_->DrawTile({ ptr->transform.translate.x,ptr->transform.translate.y }, ptr->materialConfig[0]);
				break;
			case CUBE:
				system_->DrawCube(&ptr->transformationMatrix, ptr->materialConfig[0]);
				break;
			case SPHERE:
				system_->DrawSprete(&ptr->transformationMatrix, ptr->materialConfig[0]);
				break;
			case MODEL:

				switch (ptr->modelHandle) {
				case ModelType::DEFAULT:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig);
					break;
				case ModelType::BUNNY:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, bunnyModelHandle_);
					break;
				case ModelType::TEAPOT:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, teapotModelHandle_);
					break;
				case ModelType::SUZANNE:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, suzanneModelHandle_);
					break;
				case ModelType::MULTIMESH:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, multiMeshModelHandle_);
					break;
				case ModelType::MULTIMATERIAL:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, multiMaterialModelHandle_);
					break;
				case ModelType::CHARATER:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, charaterModelHandle_);
					break;
				case ModelType::NEEDLE:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, needleModelHandle_);
					break;
				case ModelType::TILE3D:
					system_->Draw3DTile(&ptr->transformationMatrix, ptr->materialConfig);
					break;
				case ModelType::SKYDOME:
					system_->DrawModel(&ptr->transformationMatrix, ptr->materialConfig, skydomeModelHandle_);
					break;
				}

			}
		}
	}

	ImGui::Begin("ControlPanel");
	//ImGui::Checkbox("isTemplate", &isTemplate);
	{
		const char* items[] = { "Triangle", "Sprite" ,"Tile","Cube","Sphere" ,"Plant","BUNNY","TEAPOT","SUZANNE","multiMesh","multiMaterial","charater","needle" };
		int selectedItem = CreateObjectHandle;
		if (ImGui::Combo("Model", &selectedItem, items, IM_ARRAYSIZE(items))) {
			CreateObjectHandle = selectedItem;
		}
		if (ImGui::Button("Add Object")) {
			InstanceObject* object = new InstanceObject;
			MaterialConfig materialConfig;
			InitMaterialConfig(&materialConfig);
			object->materialConfig.push_back(materialConfig);
			if (selectedItem == TRIANGLE) { object->objectType = TRIANGLE; }

			else if (selectedItem == SPRITE) { object->objectType = SPRITE; } else if (selectedItem == CUBE) { object->objectType = CUBE; } else if (selectedItem == SPHERE) { object->objectType = SPHERE; } else if (selectedItem == MODEL) { object->objectType = MODEL;	object->modelHandle = ModelType::DEFAULT; } else if (selectedItem == BUNNY) { object->objectType = MODEL;	object->modelHandle = ModelType::BUNNY; } else if (selectedItem == TEAPOT) { object->objectType = MODEL;	object->modelHandle = ModelType::TEAPOT; } else if (selectedItem == SUZANNE) { object->objectType = MODEL;	object->modelHandle = ModelType::SUZANNE; } else if (selectedItem == MULTIMESH) {
				object->objectType = MODEL;
				object->modelHandle = ModelType::MULTIMESH;
				for (int i = 0; i < system_->GetMuitModelNum((int)ModelType::MULTIMESH) - 1; i++) {
					MaterialConfig materialConfig;
					InitMaterialConfig(&materialConfig);
					object->materialConfig.push_back(materialConfig);
				}
			} else if (selectedItem == MULTIMATERIAL) {
				object->objectType = MODEL;
				object->modelHandle = ModelType::MULTIMATERIAL;
				for (int i = 0; i < system_->GetMuitModelNum((int)ModelType::MULTIMATERIAL) - 1; i++) {
					MaterialConfig materialConfig;
					InitMaterialConfig(&materialConfig);
					object->materialConfig.push_back(materialConfig);
				}
			} else if (selectedItem == CHARATER) {
				object->objectType = MODEL;
				object->modelHandle = ModelType::CHARATER;
				for (int i = 0; i < system_->GetMuitModelNum((int)ModelType::CHARATER) - 1; i++) {
					MaterialConfig materialConfig;
					InitMaterialConfig(&materialConfig);
					object->materialConfig.push_back(materialConfig);
				}
			} else if (selectedItem == NEEDLE) {
				object->objectType = MODEL;
				object->modelHandle = ModelType::NEEDLE;
				for (int i = 0; i < system_->GetMuitModelNum((int)ModelType::NEEDLE) - 1; i++) {
					MaterialConfig materialConfig;
					InitMaterialConfig(&materialConfig);
					object->materialConfig.push_back(materialConfig);
				}
			}
			objectGroup.push_back(object);
		}
	}

	int i = 0;
	for (auto& ptr : objectGroup) {
		std::string name;
		std::string name1;
		std::string name2;
		std::string name3;
		std::string name4;
		std::string name5;
		std::string name6;
		std::string name7;
		std::string name8;
		std::string name9;
		std::string name10;
		std::string name11;
		std::string name12;
		std::string name13;
		const char* items[] = { "uvChecker", "monsterBall","can'sLightBox","white" };
		switch (ptr->objectType) {
		case TRIANGLE:
			ImGui::Separator();
			name = "Triangle_" + std::to_string(i);
			name1 = "Tri.trans.translate_" + std::to_string(i);
			name2 = "Tri.trans.rotate_" + std::to_string(i);
			name3 = "Tri.trans.scale_" + std::to_string(i);
			name4 = "Material_" + std::to_string(i);
			name5 = "useOriginalTexture_" + std::to_string(i);
			name6 = "objectMaterialHandle_" + std::to_string(i);
			name7 = "UVTranslate_" + std::to_string(i);
			name8 = "UVScale_" + std::to_string(i);
			name9 = "UVRotate_" + std::to_string(i);
			name10 = "Color_" + std::to_string(i);
			name11 = "EnableLighting_" + std::to_string(i);
			name12 = "LightingItem_" + std::to_string(i);
			name13 = "Delete_" + std::to_string(i);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::SliderFloat3(name1.c_str(), &ptr->transform.translate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name2.c_str(), &ptr->transform.rotate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name3.c_str(), &ptr->transform.scale.x, -5.0f, 5.0f);
				for (auto& ptr2 : ptr->materialConfig) {
					if (ImGui::CollapsingHeader(name4.c_str())) {
						ImGui::Checkbox(name5.c_str(), &ptr2.useOriginalTexture);
						ImGui::Combo(name6.c_str(), &ptr2.textureHandle, items, IM_ARRAYSIZE(items));
						ImGui::SliderFloat2(name7.c_str(), &ptr2.uvTranslate.x, 0.0f, 640.0f);
						ImGui::SliderFloat2(name8.c_str(), &ptr2.uvScale.x, 0.0f, 10.0f);
						ImGui::SliderAngle(name9.c_str(), &ptr2.uvRotate.z, 0.0f, 400.0f);
						ImGui::ColorEdit4(name10.c_str(), &ptr2.textureColor.x);
						ImGui::Checkbox(name11.c_str(), &ptr2.enableLighting);
						const char* LightingItem[] = { "Lambert", "HalfLambert" };
						int selectedItem2 = (int)ptr2.lightModelType;
						if (ImGui::Combo(name12.c_str(), &selectedItem2, LightingItem, IM_ARRAYSIZE(LightingItem))) {
							ptr2.lightModelType = (LightModelType)selectedItem2;
						}
					}
				}
			}
			ImGui::Separator();
			if (ImGui::Button(name13.c_str())) {
				ptr->isDelete = true;
			}
			i++;
			break;
		case SPRITE:
			ImGui::Separator();
			name = "Sprite" + std::to_string(i);
			name1 = "Sprite.trans.translate" + std::to_string(i);
			name2 = "Sprite.trans.rotate" + std::to_string(i);
			name3 = "Sprite.trans.scale" + std::to_string(i);
			name4 = "Material_" + std::to_string(i);
			name5 = "useOriginalTexture_" + std::to_string(i);
			name6 = "objectMaterialHandle_" + std::to_string(i);
			name7 = "UVTranslate_" + std::to_string(i);
			name8 = "UVScale_" + std::to_string(i);
			name9 = "UVRotate_" + std::to_string(i);
			name10 = "Color_" + std::to_string(i);
			name11 = "EnableLighting_" + std::to_string(i);
			name12 = "LightingItem_" + std::to_string(i);
			name13 = "Delete_" + std::to_string(i);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::SliderFloat3(name1.c_str(), &ptr->transform.translate.x, 0, 640.0f);
				ImGui::SliderFloat3(name2.c_str(), &ptr->transform.rotate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name3.c_str(), &ptr->transform.scale.x, -5.0f, 5.0f);
				for (auto& ptr2 : ptr->materialConfig) {
					if (ImGui::CollapsingHeader(name4.c_str())) {
						ImGui::Checkbox(name5.c_str(), &ptr2.useOriginalTexture);
						ImGui::Combo(name6.c_str(), &ptr2.textureHandle, items, IM_ARRAYSIZE(items));
						ImGui::SliderFloat2(name7.c_str(), &ptr2.uvTranslate.x, 0.0f, 640.0f);
						ImGui::SliderFloat2(name8.c_str(), &ptr2.uvScale.x, 0.0f, 10.0f);
						ImGui::SliderAngle(name9.c_str(), &ptr2.uvRotate.z, 0.0f, 400.0f);
						ImGui::ColorEdit4(name10.c_str(), &ptr2.textureColor.x);
						ImGui::Checkbox(name11.c_str(), &ptr2.enableLighting);
					}
				}
			}
			ImGui::Separator();
			if (ImGui::Button(name13.c_str())) {
				ptr->isDelete = true;
			}
			i++;
			break;
		case TILE:
			ImGui::Separator();
			name = "Tile" + std::to_string(i);
			name1 = "Tile.trans.translate" + std::to_string(i);
			name2 = "Tile.trans.rotate" + std::to_string(i);
			name3 = "Tile.trans.scale" + std::to_string(i);
			name4 = "Material_" + std::to_string(i);
			name5 = "useOriginalTexture_" + std::to_string(i);
			name6 = "objectMaterialHandle_" + std::to_string(i);
			name7 = "UVTranslate_" + std::to_string(i);
			name8 = "UVScale_" + std::to_string(i);
			name9 = "UVRotate_" + std::to_string(i);
			name10 = "Color_" + std::to_string(i);
			name11 = "EnableLighting_" + std::to_string(i);
			name12 = "LightingItem_" + std::to_string(i);
			name13 = "Delete_" + std::to_string(i);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::SliderFloat3(name1.c_str(), &ptr->transform.translate.x, 0, 640.0f);
				ImGui::SliderFloat3(name2.c_str(), &ptr->transform.rotate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name3.c_str(), &ptr->transform.scale.x, -5.0f, 5.0f);
				for (auto& ptr2 : ptr->materialConfig) {
					if (ImGui::CollapsingHeader(name4.c_str())) {
						ImGui::Checkbox(name5.c_str(), &ptr2.useOriginalTexture);
						ImGui::Combo(name6.c_str(), &ptr2.textureHandle, items, IM_ARRAYSIZE(items));
						ImGui::SliderFloat2(name7.c_str(), &ptr2.uvTranslate.x, 0.0f, 640.0f);
						ImGui::SliderFloat2(name8.c_str(), &ptr2.uvScale.x, 0.0f, 10.0f);
						ImGui::SliderAngle(name9.c_str(), &ptr2.uvRotate.z, 0.0f, 400.0f);
						ImGui::ColorEdit4(name10.c_str(), &ptr2.textureColor.x);
						ImGui::Checkbox(name11.c_str(), &ptr2.enableLighting);
					}
				}
			}
			ImGui::Separator();
			if (ImGui::Button(name13.c_str())) {
				ptr->isDelete = true;
			}
			i++;
			break;
		case CUBE:
			ImGui::Separator();
			name = "Cube" + std::to_string(i);
			name1 = "Cub.trans.translate" + std::to_string(i);
			name2 = "Cub.trans.rotate" + std::to_string(i);
			name3 = "Cub.trans.scale" + std::to_string(i);
			name4 = "Material" + std::to_string(i);
			name5 = "objectMaterialHandle" + std::to_string(i);
			name6 = "UVTranslate" + std::to_string(i);
			name7 = "UVScale" + std::to_string(i);
			name8 = "UVRotate" + std::to_string(i);
			name9 = "Color" + std::to_string(i);
			name10 = "EnableLighting" + std::to_string(i);
			name11 = "LightingItem" + std::to_string(i);
			name12 = "LightingItem_" + std::to_string(i);
			name13 = "Delete_" + std::to_string(i);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::SliderFloat3(name1.c_str(), &ptr->transform.translate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name2.c_str(), &ptr->transform.rotate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name3.c_str(), &ptr->transform.scale.x, -5.0f, 5.0f);
				for (auto& ptr2 : ptr->materialConfig) {
					if (ImGui::CollapsingHeader(name4.c_str())) {
						ImGui::Checkbox(name5.c_str(), &ptr2.useOriginalTexture);
						ImGui::Combo(name6.c_str(), &ptr2.textureHandle, items, IM_ARRAYSIZE(items));
						ImGui::SliderFloat2(name7.c_str(), &ptr2.uvTranslate.x, 0.0f, 640.0f);
						ImGui::SliderFloat2(name8.c_str(), &ptr2.uvScale.x, 0.0f, 10.0f);
						ImGui::SliderAngle(name9.c_str(), &ptr2.uvRotate.z, 0.0f, 400.0f);
						ImGui::ColorEdit4(name10.c_str(), &ptr2.textureColor.x);
						ImGui::Checkbox(name11.c_str(), &ptr2.enableLighting);
						const char* LightingItem[] = { "Lambert", "HalfLambert" };
						int selectedItem2 = (int)ptr2.lightModelType;
						if (ImGui::Combo(name12.c_str(), &selectedItem2, LightingItem, IM_ARRAYSIZE(LightingItem))) {
							ptr2.lightModelType = (LightModelType)selectedItem2;
						}
					}
				}
			}
			ImGui::Separator();
			if (ImGui::Button(name13.c_str())) {
				ptr->isDelete = true;
			}
			i++;
			break;
		case SPHERE:
			ImGui::Separator();
			name = "Sphere" + std::to_string(i);
			name1 = "Sphere.trans.translate" + std::to_string(i);
			name2 = "Sphere.trans.rotate" + std::to_string(i);
			name3 = "Sphere.trans.scale" + std::to_string(i);
			name4 = "Material_" + std::to_string(i);
			name5 = "useOriginalTexture_" + std::to_string(i);
			name6 = "objectMaterialHandle_" + std::to_string(i);
			name7 = "UVTranslate_" + std::to_string(i);
			name8 = "UVScale_" + std::to_string(i);
			name9 = "UVRotate_" + std::to_string(i);
			name10 = "Color_" + std::to_string(i);
			name11 = "EnableLighting_" + std::to_string(i);
			name12 = "LightingItem_" + std::to_string(i);
			name13 = "Delete_" + std::to_string(i);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::SliderFloat3(name1.c_str(), &ptr->transform.translate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name2.c_str(), &ptr->transform.rotate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name3.c_str(), &ptr->transform.scale.x, -5.0f, 5.0f);
				for (auto& ptr2 : ptr->materialConfig) {
					if (ImGui::CollapsingHeader(name4.c_str())) {
						ImGui::Checkbox(name5.c_str(), &ptr2.useOriginalTexture);
						ImGui::Combo(name6.c_str(), &ptr2.textureHandle, items, IM_ARRAYSIZE(items));
						ImGui::SliderFloat2(name7.c_str(), &ptr2.uvTranslate.x, 0.0f, 640.0f);
						ImGui::SliderFloat2(name8.c_str(), &ptr2.uvScale.x, 0.0f, 10.0f);
						ImGui::SliderAngle(name9.c_str(), &ptr2.uvRotate.z, 0.0f, 400.0f);
						ImGui::ColorEdit4(name10.c_str(), &ptr2.textureColor.x);
						ImGui::Checkbox(name11.c_str(), &ptr2.enableLighting);
						const char* LightingItem[] = { "Lambert", "HalfLambert" };
						int selectedItem2 = (int)ptr2.lightModelType;
						if (ImGui::Combo(name12.c_str(), &selectedItem2, LightingItem, IM_ARRAYSIZE(LightingItem))) {
							ptr2.lightModelType = (LightModelType)selectedItem2;
						}
					}
				}
			}
			ImGui::Separator();
			if (ImGui::Button(name12.c_str())) {
				ptr->isDelete = true;
			}
			i++;
			break;
		case MODEL:
			if (ptr->modelHandle == ModelType::SKYDOME) continue;
			ImGui::Separator();
			name = "Model" + std::to_string(i);
			name1 = "Model.trans.translate" + std::to_string(i);
			name2 = "Model.trans.rotate" + std::to_string(i);
			name3 = "Model.trans.scale" + std::to_string(i);
			name13 = "Delete" + std::to_string(i);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::SliderFloat3(name1.c_str(), &ptr->transform.translate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name2.c_str(), &ptr->transform.rotate.x, -5.0f, 5.0f);
				ImGui::SliderFloat3(name3.c_str(), &ptr->transform.scale.x, -5.0f, 5.0f);
				int j = 0;
				for (auto& ptr2 : ptr->materialConfig) {
					std::string nName4 = "Material_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName5 = "useOriginalTexture" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName6 = "objectMaterialHandle_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName7 = "UVTranslate_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName8 = "UVScale_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName9 = "UVRotate_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName10 = "Color_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName11 = "EnableLighting_" + std::to_string(i) + '_' + std::to_string(j);
					std::string nName12 = "LightingItem_" + std::to_string(i) + '_' + std::to_string(j);
					if (ImGui::CollapsingHeader(nName4.c_str())) {
						ImGui::Checkbox(nName5.c_str(), &ptr2.useOriginalTexture);
						ImGui::Combo(nName6.c_str(), &ptr2.textureHandle, items, IM_ARRAYSIZE(items));
						ImGui::SliderFloat2(nName7.c_str(), &ptr2.uvTranslate.x, 0.0f, 640.0f);
						ImGui::SliderFloat2(nName8.c_str(), &ptr2.uvScale.x, 0.0f, 10.0f);
						ImGui::SliderAngle(nName9.c_str(), &ptr2.uvRotate.z, 0.0f, 400.0f);
						ImGui::ColorEdit4(nName10.c_str(), &ptr2.textureColor.x);
						ImGui::Checkbox(nName11.c_str(), &ptr2.enableLighting);
						const char* LightingItem[] = { "Lambert", "HalfLambert" };
						int selectedItem2 = (int)ptr2.lightModelType;
						if (ImGui::Combo(nName12.c_str(), &selectedItem2, LightingItem, IM_ARRAYSIZE(LightingItem))) {
							ptr2.lightModelType = (LightModelType)selectedItem2;
						}
					}
					j++;
				}

			}
			ImGui::Separator();
			if (ImGui::Button(name13.c_str())) {
				ptr->isDelete = true;
			}
			i++;
			break;
		}
	}

	ImGui::ColorEdit4("directionalLight.direction", &directionalLight.color.x);
	ImGui::SliderFloat3("directionalLight.direction", &directionalLight.direction.x, -1.0f, 1.0f);
	ImGui::SliderFloat("directionalLight.intensity", &directionalLight.intensity, -1.0f, 1.0f);
	ImGui::SliderInt("textSubject", &textSubject, 0, 32);
	ImGui::SliderFloat("Text1", &A, -1, 1);
	ImGui::SliderFloat("Text2", &B, -1, 1);
	ImGui::SliderInt("Text3", &C, 0, 65535);
	ImGui::SliderInt("Text4", &E, 0, 65535);
	if (D == 1) { ImGui::Text("DirectInput ON"); }
	if (D == 2) { ImGui::Text("XInput ON"); }
	if (!config::GetKeyboardState()) { ImGui::Text("Keyboard Notfound"); }
	if (!config::GetMouseState()) { ImGui::Text("Mouse Notfound"); }
	if (!config::GetGamePadState()) { ImGui::Text("Gamepad Notfound"); }
	ImGui::End();
	// ImGui::ShowDemoWindow();

	{
		bool isSEPlaying = system_->SoundIsPlaying(se1_);
		bool isBGMPlaying = system_->SoundIsPlaying(se2_);
		float iVolumeNow = system_->SoundGetVolume(se2_);
		float mVolumeNow = system_->SoundGetMasterVolume();
		float seVolumeNow = system_->SoundGetSEVolume();
		float bgmVolumeNow = system_->SoundGetBGMVolume();
		bool masterMuteNow = system_->SoundGetMasterMute();
		bool seMuteNow = system_->SoundGetSEMute();
		bool bgmMuteNow = system_->SoundGetBGMMute();
		bool iMuteNow = system_->SoundGetMute(se2_);

		ImGui::Begin("TrySound");
		ImGui::Checkbox("IsSEPlaying", &isSEPlaying);
		ImGui::Checkbox("IsBGMPlaying", &isBGMPlaying);
		ImGui::InputFloat("iVolume-", &iVolumeNow);
		ImGui::InputFloat("MasterVolume-", &mVolumeNow);
		ImGui::InputFloat("SEVolume-", &seVolumeNow);
		ImGui::InputFloat("BGMVolume-", &bgmVolumeNow);
		ImGui::SliderFloat("iVolume", &iVolume, -1.0f, 3.0f);
		ImGui::SliderFloat("MasterVolume", &mVolume, -1.0f, 3.0f);
		ImGui::SliderFloat("SEVolume", &seVolume, -1.0f, 3.0f);
		ImGui::SliderFloat("BGMVolume", &bgmVolume, -1.0f, 3.0f);
		ImGui::Checkbox("MuteSe2", &iMute);
		ImGui::Checkbox("MuteMaster", &masterMute);
		ImGui::Checkbox("MuteSE", &seMute);
		ImGui::Checkbox("MuteBGM", &bgmMute);
		ImGui::Checkbox("GetMuteSe2", &iMuteNow);
		ImGui::Checkbox("GetMuteMaster", &masterMuteNow);
		ImGui::Checkbox("GetMuteSE", &seMuteNow);
		ImGui::Checkbox("GetMuteBGM", &bgmMuteNow);
		ImGui::End();
	}

	{
		float fps = system_->GetFPS();
		float fps1s = system_->GetFPSPerSecond();
		float deltaTime = system_->GetDeltaTime();
		ImGui::Begin("Try FPS");
		ImGui::InputFloat("FPS", &fps);
		ImGui::InputFloat("FPS_1s", &fps1s);
		ImGui::InputFloat("deltaTime", &deltaTime);
		ImGui::End();
	}

	{
		ImGui::Begin("Timer");
		ImGui::Text("%.1f,%.1f", timer_->parameter_, timer_->maxTime_);
		ImGui::End();
	}

}
