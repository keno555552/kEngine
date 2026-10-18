#pragma once
#include <vector>
#include <string>

enum class ScenePhase {
	PREPARE,
	ENTRY,
	PROGRESS,
	TRANSITION,
};

enum class SceneOutcome{
	LOSE = -1,
	NONE,
	WIN,
	MENU,
	NEXT,
	RETRY,
	RETURN,
	EXIT,
};