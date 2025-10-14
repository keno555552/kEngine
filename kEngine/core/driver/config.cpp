#include "config.h"

/// インスタンス設定
int config::clientWidth_ = 0;
int config::clientHeight_ = 0;

bool config::isConnetKeyboard_ = false;
bool config::isConnetMouse_ = false;
bool config::isConnetGamePad_ = false;
gamepadType config::gamePadType_ = NODEVICE;
int config::XInputDriverIP_ = -1;