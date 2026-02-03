#include "SceneFactory.h"
#include "Logger.h"
#include "SceneTitle.h" // 修正VCIC001: 明確包含SceneTitle.h

SceneFactory::SceneFactory(kEngine* system) 
    : system_(system) 
{

    sceneRegistry_["TITLE"] = [this]() { return new SceneTitle(system_); };
    sceneRegistry_["STAGE_01"] = [this]() { return new Scene1(system_); };
	sceneRegistry_["WIN"] = [this]() { return new SceneWin(system_); };
	sceneRegistry_["LOSE"] = [this]() { return new SceneLose(system_); };
}

BaseScene* SceneFactory::CreateScene(const std::string& sceneName) {
    auto it = sceneRegistry_.find(sceneName);
    if (it != sceneRegistry_.end()) {
        return it->second();
    }
    Logger::Log("[kError] SF ::CreateScene: Scene not found: " + sceneName);
    return nullptr;
}
