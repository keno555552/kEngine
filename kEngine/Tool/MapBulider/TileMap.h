#pragma once
#include "MathsIncluder.h"
#include <vector>//////必須
#include <string>
//#define ENABLE_PERLIN

#ifdef ENABLE_PERLIN
#include "PerlinNoise/PerlinNoise.h"
#endif

///texture種類のenum
enum Type {
	NONE,
	AIR,
	GRASS,
	EARTH,
	ROCK,
	IRON,
	GOLD,
	DIAMOND,
	MONSTERSPAWNER,
	DUNGEONDOOR,
	MAX_TYPE
};

/// <summary>
/// 二進数比較器(ローカル)
/// </summary>
/// <param name="binary">比較基準</param>
/// <param name="target">目標</param>
/// <returns>=ならtrue</returns>
bool BCThisFile(int binary, int target);

/// Type <-> 名前（"Rock" など、大文字小文字は区別しない）
/// 見つからない場合 TileTypeFromName は -1 を返す
const char* TileTypeToName(int type);
int TileTypeFromName(const std::string& name);

#pragma region TileMap
typedef struct TileMapData {
	std::string name;
	std::vector<int> Column;
	std::vector<std::vector<int>> Row;
	void ColumnPush();
	std::string TileMapDataToString();

	/// ===== グリッド操作（Row[y][x]） ===== ///
	int Width() const { return Row.empty() ? 0 : (int)Row[0].size(); }
	int Height() const { return (int)Row.size(); }
	bool InBounds(int x, int y) const { return y >= 0 && y < Height() && x >= 0 && x < (int)Row[y].size(); }
	/// サイズ変更（既存の内容は残す、増えた分は fillType）
	void Resize(int width, int height, int fillType);
	/// 全マスを type で埋める
	void Fill(int type);

	/// ===== CSV（1行 = y、カンマ区切り = x） ===== ///
	bool ExportCsv(const std::string& path, const std::string& mapDataName = "") const;
	bool ImportCsv(const std::string& path, std::string* outMapDataName = nullptr);
}TileMapData;
class TextureMatcher
{
public:
	TextureMatcher();

	struct TextureStruct {
		int texture;
		int textureWidth;
		int textureHeight;
		int eachWidth;
		int eachHeight;
		int row;
		int column;
	};

	void Init(const int texture,
		const int textureWidth,
		const int textureHeight,
		const int eachWidth,
		const int eachHeight,
		const int row,
		const int column);

	///// <summary>
	///// マップを元ついて1ますを描く、自分でロールとコロンを入力
	///// </summary>
	///// <param name="pos"></param>
	///// <param name="row"></param>
	///// <param name="colume"></param>
	///// <param name="num"></param>
	///// <param name="type"></param>
	//void DrawOne(Vector2 pos, int row, int colume, int num, Type type);
	//
	///// <summary>
	///// 1ますを描く,マップに元つかない
	///// </summary>
	///// <param name="pos"></param>
	///// <param name="num"></param>
	///// <param name="type"></param>
	//void DrawOne(Vector2 pos, int num, Type type);
	//
	///// <summary>
	///// テストTilpMapを描く
	///// </summary>
	///// <param name="pos"></param>
	///// <param name="type"></param>
	//void DrawTilpMap(Vector2 pos, Type type);
	//
	///// <summary>
	///// となりのunitによって描く
	///// </summary>
	///// <param name="LT"></param>
	///// <param name="MT"></param>
	///// <param name="RT"></param>
	///// <param name="LM"></param>
	///// <param name="TG"></param>
	///// <param name="RM"></param>
	///// <param name="LB"></param>
	///// <param name="MB"></param>
	///// <param name="RB"></param>
	///// <param name="pos"></param>
	///// <param name="type"></param>
	//void DrawUnit(int LT, int MT, int RT,
	//	int LM, int TG, int RM,
	//	int LB, int MB, int RB, Vector2 pos, Type type);
	//
	///// <summary>
	///// マップチップデイタに元ついてマップを描く
	///// </summary>
	///// <param name="Map"></param>
	///// <param name="pos"></param>
	///// <param name="type"></param>
	//void DrawMap(TileMapData Map, Vector2 pos, Type type);
	//
	//void DrawMapColor(TileMapData Map, Vector2 pos);
	//void DrawMapWord(TileMapData Map, Vector2 pos);
	//
	///// <summary>
	///// マップチップデイタに元ついてマップを文字列で表現する
	///// </summary>
	///// <param name="Map"></param>
	///// <param name="pos"></param>
	///// <param name="row"></param>
	///// <param name="column"></param>
	///// <param name="type"></param>
	//void DrawMapWord(TileMapData Map, Vector2 pos, int row, int column, Type type);

	void clipMapTo01(TileMapData Map, TileMapData* MapSave, int type, int type2, int clipNum, bool isReverse);

private:
	TextureStruct textureStruct;

	Vector2 LTpos_;
	int MaxRow_;
	int MaxColume_;

private:
#pragma region PerlinNoise関連
#ifdef ENABLE_PERLIN
	PerlinNoise* perlinNoise = new PerlinNoise;
#endif
#pragma endregion
};
#pragma endregion

#pragma region ColorMap
typedef struct ColorMapData {
	std::vector<int> Column;
	std::vector<std::vector<int>> Row;
	void ColumnPush();
}ColorMapData;

class ColorMatcher
{
public:
	ColorMatcher();

	unsigned int typeColor[10] = {
		0x000000FF, //NONE
		0x000000FF, //AIR
		0x00BB00FF, //GRASS
		0xBB5E00FF, //Earth
		0x6C6C6CFF, //ROCK
		0xE0E0E0FF, //IRON
		0xFFD700FF, //GOLD
		0x8080C0FF, //DIAMOND
		0xff00ffFF, //MONSTERSPAWNER,
		0xff00ffFF  //DONGEONDOOR,
	};


	struct ColorStruct {
		int eachWidth;
		int eachHeight;
		int row;
		int column;
	};

	void Init(
		const int eachWidth,
		const int eachHeight,
		const int row,
		const int column);

	//void DrawMapColor(TileMapData Map, Vector2 pos);
	//
	//void DrawTilpMapColor(TileMapData Map, Vector2 pos);
	//
	//void DrawMapWord(TileMapData Map, Vector2 pos);
	//
	///// <summary>
	///// マップチップデイタに元ついてマップを文字列で表現する
	///// </summary>
	///// <param name="Map"></param>
	///// <param name="pos"></param>
	///// <param name="row"></param>
	///// <param name="column"></param>
	///// <param name="type"></param>
	//void DrawMapWord(TileMapData Map, Vector2 pos, int row, int column, Type type);

	void clipMap(TileMapData Map, TileMapData* MapSave, int type, int type2, int clipNum, bool isReverse);

	void clipMap2D(TileMapData Map, TileMapData* MapSave, int type, int clipNum);

private:
	ColorStruct colorStruct;

	Vector2 LTpos_;
	int MaxRow_;
	int MaxColume_;

private:
#pragma region PerlinNoise関連
#ifdef ENABLE_PERLIN
	PerlinNoise* perlinNoise = new PerlinNoise;
#endif
#pragma endregion

};
#pragma endregion