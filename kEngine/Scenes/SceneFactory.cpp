#include "SceneFactory.h"
#include "Logger.h"
#include <memory>
#include <utility>
#include <AnimationSystem/AnimationEditor.h>
#include <CG3_HK_2/SceneCGHK2.h>
#include <CG4_HK_1/Effect2.h>
#include "GAME/scenes/UITest.h"
#include "Tool/EffectSystem/ParticleEditor.h"
#include "GAME/scenes/SceneTitle.h"
#include "GAME/scenes/Scene1.h"
#include "GAME/scenes/SceneWin.h"
#include "GAME/scenes/SceneLose.h"


SceneFactory::SceneFactory(kEngine* system)
    : system_(system)
{
    sceneRegistry_["ANIMATIONEDITOR"] = [this]() { return std::make_unique<AnimationEditor>(system_); };
    sceneRegistry_["EFFECT2"] = [this]() { return std::make_unique<Effect2>(system_); };
    sceneRegistry_["CGHK2"] = [this]() { return std::make_unique<SceneCGHK2>(system_); };
    sceneRegistry_["UITest"] = [this]() { return std::make_unique<UITest>(system_); };
	sceneRegistry_["ParticleEditor"] = [this]() { return std::make_unique<ParticleEditor>(system_); };
    sceneRegistry_["TITLE"] = [this]() { return std::make_unique<SceneTitle>(system_); };
    sceneRegistry_["STAGE_01"] = [this]() { return std::make_unique<Scene1>(system_); };
    sceneRegistry_["WIN"] = [this]() { return std::make_unique<SceneWin>(system_); };
    sceneRegistry_["LOSE"] = [this]() { return std::make_unique<SceneLose>(system_); };

}

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName) {
    auto it = sceneRegistry_.find(sceneName);
    if (it != sceneRegistry_.end()) {
        return std::move(it->second());
    }
    Logger::Log("[kError] SF ::CreateScene: Scene not found: " + sceneName);
    return nullptr;
}
