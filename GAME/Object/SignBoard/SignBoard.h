#pragma once
#include <memory>
#include <string>
#include "Object/Sprite.h"
#include "tool/TimeManager/TimeManager.h"

/// ===== 定数 ===== ///
static inline const float   kSignBoardMoveTime = 0.4f;                 // 上がる／下がる時間
static inline const Vector2 kSignBoardSize = { 990.0f, 560.0f };   // woodSign.png のサイズ（位置計算用）
static inline const float   kSignBoardLayer = 50.0f;                // 描画の奥行き（HP UI より手前に）

/// 看板を調べた時に下から浮かんでくる板
class SignBoard {
public:
	explicit SignBoard(kEngine* system);
	~SignBoard() = default;

	void Update();
	void Draw();

	void Open();
	void Close();
	void Toggle() { IsOpen() ? Close() : Open(); }

	/// 表示する画像を変える（読み込み済みのハンドル）
	void SetTexture(int textureHandle);
	/// パスから読み込んで変える
	void SetTexture(const std::string& path);

	bool IsOpen() const { return phase_ == Phase::kEntry || phase_ == Phase::kShow; }
	bool IsVisible() const { return phase_ != Phase::kHidden; }

private:
	enum class Phase { kHidden, kEntry, kShow, kExit };

	kEngine* system_ = nullptr;                 // 借り
	std::unique_ptr<SimpleSprite> sprite_;

	int TH_board_ = -1;

	Phase phase_ = Phase::kHidden;
	Timer moveTimer_;
	float startY_ = 0.0f;                       // 動き始めた時の y（途中で反転してもワープしない）

	Vector2 showPos_{};                         // 表示中の左上
	float hiddenY_ = 0.0f;                      // 画面外（下）
};