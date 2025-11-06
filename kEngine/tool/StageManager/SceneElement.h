#pragma once

enum class SceneNum {
	S_END = -10,
	S_TESTER = -1,
	S_NONE = 0,
	S_TITLE = 1,
	S_SELECT,
	S_STAGE01,
	S_STAGE02,
	S_STAGE03,
	S_STAGE04,
	S_STAGE05,
	S_STAGE06,
	S_STAGE07,
	S_STAGE08,
	S_STAGE09,
	S_STAGE10,
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