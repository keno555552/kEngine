#pragma once

enum class SceneNum {
	S_END = -10,
	S_TESTER = -2,
	S_NONE = -1,
	S_TITLE,
	S_SELECT,
	S_STAGE_REST,
	S_STAGE_01,
	S_STAGE_02,
	S_WIN,
	S_LOSE,
	S_Result,

	S_BOSSTEST,

	S_ANIMATIONEDITOR = 100,
};

enum class ScenePhase {
	PREPARE,
	ENTRY,
	PROGRESS,
	TRANSITION,
	EXIT,
};

enum class IsWin {
	LOSE = -1,
	NONE,
	WIN,
};