#pragma once

enum class SceneNum {
	S_END = -10,
	S_TESTER = -1,
	S_NONE = 0,
	S_TITLE = 1,
	S_SELECT,
	S_STAGE,
	S_WIN,
	S_GAMEOVER,
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