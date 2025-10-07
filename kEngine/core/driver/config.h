#pragma once

enum gamepadType {
	NODEVICE,
	DIRECTINPUT,
	XINPUT
};

class config
{
public:

	static void SaveClientWidth(int clientWidth) { clientWidth_ = clientWidth; }
	static int GetClientWidth() { return clientWidth_; }
	static void SaveClientHeight(int clientHeight) { clientHeight_ = clientHeight; }
	static int GetClientHeight() { return clientHeight_; }
	static void SetConnetKeyboard(bool* state) { isConnetKeyboard_ = *state; }
	static bool GetKeyboardState() { return isConnetKeyboard_; }
	static void SetConnetMouse(bool* state) { isConnetMouse_ = *state; }
	static bool GetMouseState() { return isConnetMouse_; }
	static void SetConnetGamePad(bool* state) { isConnetGamePad_ = *state; }
	static bool GetGamePadState() { return isConnetGamePad_; }
	static void SetGamePadType(gamepadType Type) { gamePadType_ = Type; }
	static gamepadType GetGamePadType() { return gamePadType_; }
	static void SetXInputIp(int IP) { XInputDriverIP_ = IP; }
	static int GetXInputIp() { return XInputDriverIP_; }
	static int GetTileNumInstance() { return kTileNumInstance_; }

private:
	static int clientWidth_;
	static int clientHeight_;

	static bool isConnetKeyboard_;
	static bool isConnetMouse_;
	static bool isConnetGamePad_;
	static gamepadType gamePadType_;
	static int XInputDriverIP_;

private:
	///描画設定
	static const int kTileNumInstance_ = 2000;     /// Tileの最大インスタンス数
};