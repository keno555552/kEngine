#pragma once
#include <string>
#include <unordered_map>

#include "BaseSceneFactory.h"

#include "BaseScene.h"
#include "SceneTitle.h"
#include "Scene1.h"
#include "SceneWin.h"
#include "SceneLose.h"

// このゲーム用のシーン工場
class SceneFactory : public BaseSceneFactory
{
public:

    SceneFactory(kEngine* system);
	~SceneFactory() override = default;

    /// <summary>
    /// シーン生成
    /// </summary>
    /// <param name="sceneName">シーン名</param>
    /// <returns>生成したシーン</returns>
    BaseScene* CreateScene(const std::string& sceneName) override;


private:
	kEngine* system_ = nullptr;
	std::unordered_map<std::string, std::function<BaseScene*()>> sceneRegistry_;
};
