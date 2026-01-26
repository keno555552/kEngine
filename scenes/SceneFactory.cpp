#include "SceneFactory.h"
#include "Logger.h"
#include "SceneTitle.h" // 修正VCIC001: 明確包含SceneTitle.h

SceneFactory::SceneFactory(kEngine* system) 
    : system_(system) 
{

    sceneRegistry_["TITLE"] = [this]() { return new SceneTitle(system_); };
    sceneRegistry_["STAGE_01"] = [this]() { return new Scene1(system_); };
    sceneRegistry_["RESULT"] = [this]() { return new SceneResult(system_); };
    //sceneRegistry_["TESTER"] = [this]() { return new SceneTester(system_); };
    sceneRegistry_["TEST"] = [this]() { return new SceneTest(system_); };
    sceneRegistry_["TEST2"] = [this]() { return new SceneTest2(system_); };
    sceneRegistry_["ANIMATIONEDITOR"] = [this]() { return new AnimationEditor(system_); };
    sceneRegistry_["EFFECT2"] = [this]() { return new Effect2(system_); };
    sceneRegistry_["CGHK2"] = [this]() { return new SceneCGHK2(system_); };
}

BaseScene* SceneFactory::CreateScene(const std::string& sceneName) {
    auto it = sceneRegistry_.find(sceneName);
    if (it != sceneRegistry_.end()) {
        return it->second();
    }
    Logger::Log("[kError] SF ::CreateScene: Scene not found: " + sceneName);
    return nullptr;
}
