#pragma once
#include <string>
#include <vector>
#include "Tool/MapBulider/Data/MapMakerData.h"

/// ============================================================
/// MapData 調整器（ObjectEditor と同じ 3 カラム構成）
///   1. MapData 一覧（New / Duplicate / Delete / Filter）
///   2. プレビュー（2D グリッド、パン・ズーム、クリックで塗る）
///   3. 詳細（名前・サイズ・開始点・パレット・Fill・CSV 入出力）
/// ============================================================
class MapDataEditor {
public:
	/// list : 編集対象の MapData 一覧（MapMakingEditor が所有）
	/// log  : エラーや結果を書き込む Debug Log
	void Draw(std::vector<MapData>& list, std::string& log);

	int GetSelectedIndex() const { return selected_; }

private:
	/// ===== 選択 ===== ///
	int selected_ = -1;
	int newCounter_ = 0;
	std::string filter_;

	/// ===== プレビュー用カメラ（ピクセル単位） ===== ///
	float cellSize_ = 16.0f;       // 1マスの表示サイズ
	float panX_ = 0.0f;            // キャンバス左上からマップ左上までのずれ
	float panY_ = 0.0f;
	bool fitRequested_ = true;     // 次のフレームで全体表示に合わせる
	bool previewHovered_ = false;
	bool showGrid_ = true;
	bool paintEnabled_ = true;
	std::string hoverText_;

	/// ===== 詳細 ===== ///
	int brushTile_ = ROCK;
	int resizeSize_[2] = { 64, 36 };
	int resizeTarget_ = -1;        // resizeSize_ がどの MapData の値か
	std::string csvPath_;

private:
	void DrawListColumn(std::vector<MapData>& list, std::string& log);
	void DrawPreviewColumn(MapData* data);
	void DrawDetailColumn(MapData* data, std::string& log);

	/// tiles と xSize / ySize がずれていたら合わせる
	static void SyncSize(MapData& data);
	static MapData CreateMapData(const std::string& name, int width, int height);
	static unsigned int TileColor(int type);
};
