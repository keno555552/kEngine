#pragma once
#include "kEngine.h"
#include "SceneElement.h"

class SceneManager;
class Scene
{
public:
	virtual ~Scene() = default;

	virtual ~Scene() = default;

	ScenePhase phase_ = ScenePhase::PREPARE;

public:
	virtual void Update();
	virtual void Draw();

	ScenePhase GetScenePhase() const { return phase_; }
	void SetScenePhase(ScenePhase phase) { phase_ = phase; }

	SceneNum GetNextStage() const { return nextStage; }
	void SetNextStage(SceneNum stageNum) {nextStage = stageNum; }
	
	IsWin GetIsWin() const { return isWin_; }

public:

	bool operator == (const ScenePhase target) { return phase_ == target; }
	bool operator != (const ScenePhase target) { return phase_ != target; }

protected:

	void ChangeNextStage(SceneNum sceneNum);
	void EndGame();

protected:
	kEngine* system_ = nullptr;
	SceneNum nextStage = SceneNum::S_NONE;

	IsWin isWin_ = IsWin::NONE;
	bool isSceneEnd_ = false;
};

