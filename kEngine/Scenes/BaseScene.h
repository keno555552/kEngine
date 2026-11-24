#pragma once
#include "kEngine.h"
#include "SceneElement.h"
/// ================================================ ///

class SceneManager;
class BaseScene
{
public:

	virtual ~BaseScene() = default;

	ScenePhase phase_ = ScenePhase::PREPARE;

public:

	/// ==================== 基本関数 ==================== ///
	virtual void Update();
	virtual void Draw();

	/// ============= シーンを止まる関連の操作　=============== /// 
	virtual void Pause();
	virtual void Resume();

	/// ================= シーンの状態関連　================ ///
	ScenePhase GetScenePhase() const { return phase_; }
	void SetScenePhase(ScenePhase phase) { phase_ = phase; }

	/// ============== シーンの資料やり取り関連 ============== //
	SceneOutcome GetOutcome() const { return outcome_; }
	void ResetOutcome() { outcome_ = SceneOutcome::NONE; isSceneEnd_ = false; }

public:

	bool operator == (const ScenePhase target) { return phase_ == target; }
	bool operator != (const ScenePhase target) { return phase_ != target; }

protected:

	void EndGame();

protected:
	kEngine* system_ = nullptr;

	Timer* phaseChangeTimer_ = nullptr;

	SceneOutcome outcome_ = SceneOutcome::NONE;
	bool isSceneEnd_ = false;
};