#include "config.h"
#include "string.h"

/// インスタンス設定
char* config::clientTitle_ = nullptr;
int config::clientWidth_ = 0;
int config::clientHeight_ = 0;

bool config::isConnetKeyboard_ = false;
bool config::isConnetMouse_ = false;
bool config::isConnetGamePad_ = false;
gamepadType config::gamePadType_ = NODEVICE;
int config::XInputDriverIP_ = -1;

int config::default_Triangle_MeshBufferHandle_	= 0;
int config::default_Sprite2D_MeshBufferHandle_	= 0;
int config::default_Cube_MeshBufferHandle_		= 0;
int config::default_Sphere_MeshBufferHandle_	= 0;

void config::SaveClientTitle(const char* clientTitle) {
    /// nullCheck
    if (!clientTitle) {
        delete[] clientTitle_;
        clientTitle_ = nullptr;
        return;
    }

    /// 旧資料を消す
    delete[] clientTitle_;

	/// 文字列長取得＋1(終端文字分)
    size_t length = strlen(clientTitle) + 1;
    clientTitle_ = new char[length];

	/// 文字列コピー
    strcpy_s(clientTitle_, length, clientTitle);
}

