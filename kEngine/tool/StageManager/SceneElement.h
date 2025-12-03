#pragma once

enum class SceneNum {
	S_END = -10,
	S_TESTER = -2,
	S_NONE = -1,
	S_SELECT,
	S_EFFECT1,
	S_EFFECT2,
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