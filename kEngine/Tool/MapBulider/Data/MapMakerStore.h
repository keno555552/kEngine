#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Tool/MapBulider/Adapter/ProgramAdapter.h"
#include "Tool/MapBulider/Data/GeneratorVariable.h"

/// ======================================================================
/// MapMaker / Function / Variables をファイル単位で持つ倉庫（ResourceManager と同じ考え方）
///  - ファイルは「プロジェクトフォルダからの相対パス」で指す（これがキー）
///  - 1 回読んだファイルは handle（int）で持ち、2 回目からは同じ handle を返す
///  - MapMaker → Function（共有できる：同じ做法を別の地図でも使う）
///  - MapMaker → Variables（その MapMaker 専用：<MapMaker名>_variables.json）
///  - Editor もゲームもこのクラスを使う（ゲームは LoadProject → Generate だけ）
/// ======================================================================

/// ---------- MapMakerData/Variables/<MapMaker名>_variables.json：変数 1 組 ----------
struct MapVariablesAsset {
	std::string path;                 // 相対パス（例 "MapMakerData/Variables/Cave_variables.json"）
	std::string name;
	GeneratorVariable vars;
	bool isSaved = true;
	bool shared = false;
};

/// ---------- MapMakerData/Function/<函數名>.json：函數 1 つ（変数は持たない = どの MapMaker でも使える） ----------
struct MapFunctionAsset {
	std::string path;                 // 相対パス（例 "MapMakerData/Function/Earth.json"）
	std::string name;
	std::string code;                 // 腳本（Editor 用）
	nlohmann::json script = nlohmann::json::array();   // 実行用の AST（ゲームはこれだけ使う）
	bool isSaved = true;
};

/// ---------- MapMakerData/MapMaker/<MapMaker名>.json：函數の並び + 自分の変数 ----------
struct MapMakerAsset {
	struct Entry {
		int functionHandle = -1;
		bool enabled = true;          // false = 生成の時に飛ばす
	};
	std::string path;                 // 相対パス（例 "MapMakerData/MapMaker/Cave.json"）
	std::string name;
	std::vector<Entry> functions;     // 上から順に実行
	int variablesHandle = -1;         // この MapMaker 専用の Variables
	std::vector<int> sharedVariables; // 掛けている共用 Variables（上が優先。自分の Variables が一番優先）
	std::string targetMapName;        // 出力先（空 = Editor 内のマップ）
	int width = 32;
	int height = 32;
	int seed = 0;
	bool isSaved = true;
};

class MapMakerStore {
public:
	/// 保存フォルダ（プロジェクトフォルダから）。3 種類まとめて MapMakerData の下
	static constexpr const char* kMakerFolder = "MapMakerData/MapMaker";
	static constexpr const char* kFunctionFolder = "MapMakerData/Function";
	static constexpr const char* kVariablesFolder = "MapMakerData/Variables";
	static constexpr const char* kSharedFolder = "MapMakerData/Variables/Shared";

	/// ==================== プロジェクト ==================== ///
	/// プロジェクトフォルダ（相対パスの基準）
	void SetRoot(const std::string& root) { root_ = root; }
	const std::string& GetRoot() const { return root_; }
	/// 全部消す（New Project / Load の前）
	void Clear();

	/// mmproj を読んで、"mapMaker" / "functions" / "variables" に書かれたファイルを全部読む（ゲームはこれだけ）
	bool LoadProject(const std::string& mmprojPath);
	/// 既に開いた mmproj の json から読む（Editor：mapData / maps は Editor 側が読む）
	void ReadFromProject(const nlohmann::json& project);
	/// 3 種類のファイルを書いて、mmproj の json に一覧を入れる（未保存のものだけ書く = false で全部）
	/// ファイル名は名前に合わせる（名前を変えた・古い場所にある → 新しい場所に書いて、古いファイルは消す）
	bool WriteToProject(nlohmann::json& project, bool onlyUnsaved = false);

	/// ==================== 読み込み（同じファイルは 1 回だけ） ==================== ///
	int LoadMaker(const std::string& relPath);      // 使っている Function / Variables も一緒に読む
	int LoadFunction(const std::string& relPath);

	/// ==================== 共用 Variables ==================== ///
	int CreateSharedVariables(const std::string& name);
	int LoadSharedVariables(const std::string& relPath);
	void RemoveSharedVariables(int handle);                  // 掛けている MapMaker からも外す
	void AttachShared(int makerHandle, int varsHandle);      // 一番下（優先度低）に追加
	void DetachShared(int makerHandle, int varsHandle);
	std::vector<int> SharedVariablesHandles() const;
	std::vector<int> MakersUsingVariables(int varsHandle) const;
	MapVariablesAsset* GetSharedVariables(int handle);       // shared でなければ nullptr
	/// 実際に生成で使う変数（自分 → 共用の順でまとめる）
	GeneratorVariable BuildRunVariables(int makerHandle);

	/// ==================== 作る（Editor） ==================== ///
	/// 名前からファイル名を作る（同じ名前があれば _1, _2 …）
	int CreateMaker(const std::string& name);       // 自分の Variables も作る
	int CreateFunction(const std::string& name);
	/// 複製（新しいファイルになる）。MapMaker の複製：函數は共有、Variables はコピー
	int DuplicateMaker(int handle);
	int DuplicateFunction(int handle);

	/// ==================== 消す（handle は欠番になる。ファイルは消さない） ==================== ///
	void RemoveMaker(int handle);         // その MapMaker の Variables も一緒に
	void RemoveFunction(int handle);      // 使っている MapMaker からも外す
	/// 消す前の確認用：この函數を使っている MapMaker の handle
	std::vector<int> MakersUsingFunction(int functionHandle) const;

	/// ==================== 取得 ==================== ///
	MapMakerAsset* GetMaker(int handle);
	MapFunctionAsset* GetFunction(int handle);
	/// MapMaker の Variables（無ければ作る）
	MapVariablesAsset* GetMakerVariables(int makerHandle);
	/// 使える handle の一覧（欠番は入らない）。UI のリスト表示用
	std::vector<int> MakerHandles() const;
	std::vector<int> FunctionHandles() const;
	/// 名前で探す（無ければ -1）
	int FindMaker(const std::string& name) const;
	int FindFunction(const std::string& name) const;

	/// ==================== 編集（Editor） ==================== ///
	/// 腳本を変えて AST を作り直す
	void SetFunctionCode(int handle, const std::string& code);
	/// 未保存が 1 つでもあるか
	bool HasUnsaved() const;

	/// ==================== 生成 ==================== ///
	/// MapMaker を実行（seed < 0 なら MapMaker の seed）
	TileMapData Generate(int makerHandle, int blankId, int seed = -1, std::vector<std::string>* outErrors = nullptr);

	/// 読み込み・保存のエラー（読んだら Editor が消す）
	std::vector<std::string>& Log() { return log_; }

	/// ファイルに書く中身（Editor の Compile タブでも表示に使う）
	nlohmann::json MakerToJson(const MapMakerAsset& m) const;
	nlohmann::json FunctionToJson(const MapFunctionAsset& f) const;
	nlohmann::json VariablesToJson(const MapVariablesAsset& v) const;

private:
	int LoadVariables(const std::string& relPath);
	int CreateVariables(const std::string& name);
	MapVariablesAsset* GetVariables(int handle);
	/// 古い形式の MapMaker（函數・変数がファイルの中に入っている）→ 分けて作り直す
	int ImportOldMaker(const nlohmann::json& file, const std::string& relPath);
	/// 変数をまとめる（同じ名前は into にある方を残す）
	static void MergeVariables(GeneratorVariable& into, const GeneratorVariable& from);

	/// ---------- パス ----------
	std::string FullPath(const std::string& relPath) const;
	static std::string NormalizePath(const std::string& relPath);
	/// folder + "/" + 名前 + ".json"（使われていない名前にする）
	std::string MakeUniquePath(const std::string& folder, const std::string& name) const;
	bool PathInUse(const std::string& relPath) const;
	/// ファイル名を名前・フォルダに合わせる（変えたら古いパスを oldFiles に入れる）
	void SyncFileNames(std::vector<std::string>& oldFiles);

	static bool ReadJson(const std::string& fullPath, nlohmann::json& out);
	static bool WriteJson(const std::string& fullPath, const nlohmann::json& j);

private:
	std::string root_;

	/// handle = vector の index。消した所は nullptr（欠番）
	std::vector<std::unique_ptr<MapMakerAsset>> makers_;
	std::vector<std::unique_ptr<MapFunctionAsset>> functions_;
	std::vector<std::unique_ptr<MapVariablesAsset>> variables_;

	/// 相対パス → handle（同じファイルを 2 回読まない）
	std::unordered_map<std::string, int> makerPathMap_;
	std::unordered_map<std::string, int> functionPathMap_;
	std::unordered_map<std::string, int> variablesPathMap_;

	/// 少し前の形式：Function ファイルが持っていた Variables（MapMaker を読む時にまとめる）
	std::unordered_map<int, int> legacyFunctionVars_;

	std::vector<std::string> log_;
};
