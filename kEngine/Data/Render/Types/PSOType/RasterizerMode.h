#pragma once
enum class RasterizerMode{

    CullBack = 0,           // ノーマルモデル
    CullFront,              // フロントカリングモデル
    CullNone,               // ダブルサイドモデル / パーティクル
    Wireframe,              // デバッグ
    SkyCube,                // スカイキューブ専用
    //Shadow,               // シャドウマップ専用
    NumRasterizerModes
};

