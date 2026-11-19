#pragma once

/// 重力をかける：新しい velocityY を返す（下向きが負）
/// gravity：落下加速度、limitFallSpeed：最大落下速度（正の値）、scale：物ごとの重さ（1.0 が標準）
float GravityCalc(float velocityY, float gravity, float limitFallSpeed, float dt, float scale = 1.0f);
