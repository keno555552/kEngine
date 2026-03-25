#pragma once

/// <summary>
/// 線形補間
/// </summary>
/// <param name="a">A point Vector2構造体</param>
/// <param name="b">C point Vector2構造体</param>
/// <param name="c">補間用変数t</param>
/// <param name="t">総分割数</param>
/// <returns></returns>
float Lerp(float a, float b, int c, int t);

//void Draw3DLine(const Line& line, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, int color);

//void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);
