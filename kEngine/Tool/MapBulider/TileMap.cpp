#include "TileMap.h"

bool BCThisFile(int binary, int target) {
	if (binary == (target & binary)) {
		return true;
	}
	return false;
}

#pragma region TextureMatcher
void TileMapData::ColumnPush() {
	if (!Column.empty()) {
		Row.push_back(Column);
		Column.clear();
	}
}

TextureMatcher::TextureMatcher() {
	textureStruct = {};
	LTpos_ = {};
	MaxRow_ = {};
	MaxColume_ = {};
}

void TextureMatcher::Init(int texture,
	int textureWidth,
	int textureHeight,
	int eachWidth,
	int eachHeight,
	int row,
	int column) {
	textureStruct.texture = texture;
	textureStruct.textureWidth = textureWidth;
	textureStruct.textureHeight = textureHeight;
	textureStruct.eachWidth = eachWidth;
	textureStruct.eachHeight = eachHeight;
	textureStruct.row = row;
	textureStruct.column = column;
}

//void TextureMatcher::DrawOne(Vector2 pos, int row, int colume, int num, Type type) {
//	Novice::DrawQuad(int(pos.x + textureStruct.eachWidth * (0 + row)), int(pos.y + textureStruct.eachHeight * (0 + colume)),
//		int(pos.x + textureStruct.eachWidth * (1 + row)), int(pos.y + textureStruct.eachHeight * (0 + colume)),
//		int(pos.x + textureStruct.eachWidth * (0 + row)), int(pos.y + textureStruct.eachHeight * (1 + colume)),
//		int(pos.x + textureStruct.eachWidth * (1 + row)), int(pos.y + textureStruct.eachHeight * (1 + colume)),
//		0 + textureStruct.eachWidth * num, 0 + textureStruct.eachHeight * type,
//		textureStruct.eachWidth, textureStruct.eachHeight, textureStruct.texture, 0xffffffFF);
//}
//
//void TextureMatcher::DrawOne(Vector2 pos, int num, Type type) {
//	DrawOne(pos, 0, 0, num, type);
//}
//
//void TextureMatcher::DrawTilpMap(Vector2 pos, Type type) {
//	///円第一層
//	DrawOne(pos, 0, 0, 1, type);
//	DrawOne(pos, 1, 0, 5, type);
//	DrawOne(pos, 2, 0, 2, type);
//
//	/////////////////////////////////
//
//	///円第ニ層
//	DrawOne(pos, 0, 1, 7, type);
//	DrawOne(pos, 1, 1, 0, type);
//	DrawOne(pos, 2, 1, 8, type);
//
//	/////////////////////////////////
//
//	///円第三層
//	DrawOne(pos, 0, 2, 3, type);
//	DrawOne(pos, 1, 2, 6, type);
//	DrawOne(pos, 2, 2, 4, type);
//
//	/////////////////////////////////
//
//	///1ブロックスリーズ
//	DrawOne(pos, 3, 0, 10, type);
//	DrawOne(pos, 4, 0, 9, type);
//	DrawOne(pos, 5, 0, 13, type);
//	DrawOne(pos, 6, 0, 14, type);
//	DrawOne(pos, 7, 0, 11, type);
//	DrawOne(pos, 8, 0, 12, type);
//
//	/////////////////////////////////
//
//	///L字種円
//	DrawOne(pos, 3, 1, 15, type);
//	DrawOne(pos, 4, 1, 16, type);
//	DrawOne(pos, 3, 2, 17, type);
//	DrawOne(pos, 4, 2, 18, type);
//
//	/////////////////////////////////
//
//	///点種円
//	DrawOne(pos, 5, 1, 19, type);
//	DrawOne(pos, 6, 1, 20, type);
//	DrawOne(pos, 5, 2, 21, type);
//	DrawOne(pos, 6, 2, 22, type);
//
//	/////////////////////////////////
//
//	///点種重ね
//	DrawOne(pos, 9, 0, 19, type);
//	DrawOne(pos, 9, 0, 20, type);
//	DrawOne(pos, 9, 0, 21, type);
//	DrawOne(pos, 9, 0, 22, type);
//
//	/////////////////////////////////
//
//	///単1ブロック
//	DrawOne(pos, 10, 0, 23, type);
//
//}
//
//void TextureMatcher::DrawUnit(int LT, int MT, int RT,
//	int LM, int TG, int RM,
//	int LB, int MB, int RB, Vector2 pos, Type type) {
//	pos;
//	//int Num{};
//	int Handle{};
//	enum {
//		LTop = 1,
//		MTop = 2,
//		RTop = 4,
//		LMid = 8,
//		RMid = 16,
//		LBot = 32,
//		MBot = 64,
//		RBot = 128,
//	};
//	if (TG - 1 == type) {
//		if (LT - 1 == type) { Handle |= LTop; }
//		if (MT - 1 == type) { Handle |= MTop; }
//		if (RT - 1 == type) { Handle |= RTop; }
//		if (LM - 1 == type) { Handle |= LMid; }
//		if (RM - 1 == type) { Handle |= RMid; }
//		if (LB - 1 == type) { Handle |= LBot; }
//		if (MB - 1 == type) { Handle |= MBot; }
//		if (RB - 1 == type) { Handle |= RBot; }
//	}
//
//	DrawOne(pos, 0, type);
//
//	//if (BCThisFile((LTop | MTop | RTop | LMid | RMid | LBot | MBot | RBot),Handle)
//	// && !BCThisFile() { DrawOne(pos, 0, type); }
//
//	//3
//	if (BCThisFile((MBot | LMid | RMid), Handle) && !BCThisFile((MTop), Handle)) { DrawOne(pos, 5, type); } //A
//	if (BCThisFile((MBot | LMid | MTop), Handle) && !BCThisFile((RMid), Handle)) { DrawOne(pos, 8, type); } //->
//	if (BCThisFile((MTop | LMid | RMid), Handle) && !BCThisFile((MBot), Handle)) { DrawOne(pos, 6, type); } //V
//	if (BCThisFile((MTop | RMid | MBot), Handle) && !BCThisFile((LMid), Handle)) { DrawOne(pos, 7, type); } //<-
//
//	///2
//	if (BCThisFile((MTop | MBot), Handle) && !BCThisFile(LMid, Handle) && !BCThisFile(RMid, Handle)) { DrawOne(pos, 9, type); }
//	if (BCThisFile((LMid | RMid), Handle) && !BCThisFile(MTop, Handle) && !BCThisFile(MBot, Handle)) { DrawOne(pos, 10, type); }
//
//	if (BCThisFile((MTop | LMid), Handle) && !BCThisFile((MBot), Handle) && !BCThisFile((RMid), Handle)) { DrawOne(pos, 18, type); }
//	if (BCThisFile((MTop | RMid), Handle) && !BCThisFile((MBot), Handle) && !BCThisFile((LMid), Handle)) { DrawOne(pos, 17, type); }
//	if (BCThisFile((MBot | LMid), Handle) && !BCThisFile((MTop), Handle) && !BCThisFile((RMid), Handle)) { DrawOne(pos, 16, type); }
//	if (BCThisFile((MBot | RMid), Handle) && !BCThisFile((MTop), Handle) && !BCThisFile((LMid), Handle)) { DrawOne(pos, 15, type); }
//
//	if (BCThisFile(MTop, Handle) && !BCThisFile((MBot), Handle) && !BCThisFile((LMid), Handle) && !BCThisFile((RMid), Handle)) { DrawOne(pos, 12, type); }
//	if (BCThisFile(MBot, Handle) && !BCThisFile((MTop), Handle) && !BCThisFile((LMid), Handle) && !BCThisFile((RMid), Handle)) { DrawOne(pos, 11, type); }
//	if (BCThisFile(LMid, Handle) && !BCThisFile((MTop), Handle) && !BCThisFile((MBot), Handle) && !BCThisFile((RMid), Handle)) { DrawOne(pos, 14, type); }
//	if (BCThisFile(RMid, Handle) && !BCThisFile((MTop), Handle) && !BCThisFile((MBot), Handle) && !BCThisFile((LMid), Handle)) { DrawOne(pos, 13, type); }
//
//	if (Handle == (MTop)) { DrawOne(pos, 12, type); }
//	if (Handle == (MBot)) { DrawOne(pos, 11, type); }
//	if (Handle == (LMid)) { DrawOne(pos, 14, type); }
//	if (Handle == (RMid)) { DrawOne(pos, 13, type); }
//
//	if ((MTop | LMid) == (Handle & (MTop | LMid)) && (LTop) != (Handle & (LTop))) { DrawOne(pos, 19, type); }
//	if ((MTop | RMid) == (Handle & (MTop | RMid)) && (RTop) != (Handle & (RTop))) { DrawOne(pos, 20, type); }
//	if ((MBot | LMid) == (Handle & (MBot | LMid)) && (LBot) != (Handle & (LBot))) { DrawOne(pos, 21, type); }
//	if ((MBot | RMid) == (Handle & (MBot | RMid)) && (RBot) != (Handle & (RBot))) { DrawOne(pos, 22, type); }
//
//	if (!BCThisFile((MTop), Handle) && !BCThisFile((LMid), Handle) && !BCThisFile((RMid), Handle) && !BCThisFile((MBot), Handle)) { DrawOne(pos, 23, type); }
//
//	//if (Handle == (LTop) || Handle == (RTop) || Handle == (LBot) || Handle == (RBot)) { DrawOne(pos, 23, type); }
//}
//
//void TextureMatcher::DrawMap(TileMapData Map, Vector2 pos, Type type) {
//	type;
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//			if (Map.Row[Row][Col] - 1 == type) {
//				int LTop{ 0 }, MTop{ 0 }, RTop{ 0 }, LMid{ 0 }, TarG{ 0 }, RMid{ 0 }, LBot{ 0 }, MBot{ 0 }, RBot{ 0 };
//
//				if (Row != 0 && Col != 0) { LTop = { Map.Row[Row - 1][Col - 1] }; }
//				if (Row != 0) { MTop = { Map.Row[Row - 1][Col + 0] }; }
//				if (Row != 0 && Col != Map.Row[Row].size() - 1) { RTop = { Map.Row[Row - 1][Col + 1] }; }
//				if (Col != 0) { LMid = { Map.Row[Row + 0][Col - 1] }; }
//				{ TarG = { Map.Row[Row + 0][Col + 0] }; }
//				if (Col != Map.Row[Row].size() - 1) { RMid = { Map.Row[Row + 0][Col + 1] }; }
//				if (Row != Map.Row.size() - 1 && Col != 0) { LBot = { Map.Row[Row + 1][Col - 1] }; }
//				if (Row != Map.Row.size() - 1) { MBot = { Map.Row[Row + 1][Col + 0] }; }
//				if (Row != Map.Row.size() - 1 && Col != Map.Row[Row].size() - 1) { RBot = { Map.Row[Row + 1][Col + 1] }; }
//
//
//				DrawUnit(LTop, MTop, RTop,
//					LMid, TarG, RMid,
//					LBot, MBot, RBot,
//					{ pos.x + (Col * textureStruct.eachWidth),pos.y + (Row * textureStruct.eachHeight) }, type);
//			}
//		}
//	}
//}
//
//void TextureMatcher::DrawMapColor(TileMapData Map, Vector2 pos) {
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//			int Color = 0x01000000 * Map.Row[Row][Col];
//			Color += 0x00010000 * Map.Row[Row][Col];
//			Color += 0x00000100 * Map.Row[Row][Col];
//			Color += 0x000000ff;
//
//			Novice::DrawBox(int(pos.x + (Col * textureStruct.eachWidth)), int(pos.y + (Row * textureStruct.eachHeight)),
//				textureStruct.eachWidth, textureStruct.eachHeight, 0, Color, kFillModeSolid);
//			;
//		}
//	}
//}
//
//void TextureMatcher::DrawMapWord(TileMapData Map, Vector2 pos) {
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//
//			Novice::ScreenPrintf(int(pos.x + (Col * textureStruct.eachWidth)), int(pos.y + (Row * textureStruct.eachHeight)),
//				"%d", Map.Row[Row][Col]);
//		}
//	}
//}
//
//void TextureMatcher::DrawMapWord(TileMapData Map, Vector2 pos, int row, int column, Type type) {
//	pos, type;
//
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//			int LTop{ 0 }, MTop{ 0 }, RTop{ 0 }, LMid{ 0 }, TarG{ 0 }, RMid{ 0 }, LBot{ 0 }, MBot{ 0 }, RBot{ 0 };
//
//			if (Row != 0 && Col != 0) { LTop = { Map.Row[Row - 1][Col - 1] }; }
//			if (Row != 0) { MTop = { Map.Row[Row - 1][Col + 0] }; }
//			if (Row != 0 && Col != Map.Row[Row].size() - 1) { RTop = { Map.Row[Row - 1][Col + 1] }; }
//			if (Col != 0) { LMid = { Map.Row[Row + 0][Col - 1] }; }
//			{ TarG = { Map.Row[Row + 0][Col + 0] }; }
//			if (Col != Map.Row[Row].size() - 1) { RMid = { Map.Row[Row + 0][Col + 1] }; }
//			if (Row != Map.Row.size() - 1 && Col != 0) { LBot = { Map.Row[Row + 1][Col - 1] }; }
//			if (Row != Map.Row.size() - 1) { MBot = { Map.Row[Row + 1][Col + 0] }; }
//			if (Row != Map.Row.size() - 1 && Col != Map.Row[Row].size() - 1) { RBot = { Map.Row[Row + 1][Col + 1] }; }
//
//
//			if (Row == row && Col == column) {
//				Novice::ScreenPrintf(10, 10, "%d  %d  %d", LTop, MTop, RTop);
//				Novice::ScreenPrintf(10, 40, "%d  %d  %d", LMid, TarG, RMid);
//				Novice::ScreenPrintf(10, 70, "%d  %d  %d", LBot, MBot, RBot);
//			}
//		}
//	}
//}

void TextureMatcher::clipMapTo01(TileMapData Map, TileMapData* MapSave, int type, int type2, int clipNum, bool isReverse) {
	int T1 = type, T2 = type2;
	if (isReverse) { T1 = type2, T2 = type; }
	for (int Row = 0; Row < Map.Row.size(); Row++) {
		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
			if (Map.Row[Row][Col] <= clipNum) { MapSave->Row[Row][Col] = T1; } else { MapSave->Row[Row][Col] = T2; }
		}
	}
}

#pragma endregion

#pragma region ColorMap
void ColorMapData::ColumnPush() {
	if (!Column.empty()) {
		Row.push_back(Column);
		Column.clear();
	}
}

ColorMatcher::ColorMatcher() {
	colorStruct = {};
	LTpos_ = {};
	MaxRow_ = {};
	MaxColume_ = {};
}

void ColorMatcher::Init(
	int eachWidth,
	int eachHeight,
	int row,
	int column) {
	colorStruct.eachWidth = eachWidth;
	colorStruct.eachHeight = eachHeight;
	colorStruct.row = row;
	colorStruct.column = column;
}

//void ColorMatcher::DrawMapColor(TileMapData Map, Vector2 pos) {
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//			int Color = 0x01000000 * Map.Row[Row][Col];
//			Color += 0x00010000 * Map.Row[Row][Col];
//			Color += 0x00000100 * Map.Row[Row][Col];
//			Color += 0x000000ff;
//
//			Novice::DrawBox(int(pos.x + (Col * colorStruct.eachWidth)), int(pos.y + (Row * colorStruct.eachHeight)),
//				colorStruct.eachWidth, colorStruct.eachHeight, 0, Color, kFillModeSolid);
//			;
//		}
//	}
//}
//
//void ColorMatcher::DrawTilpMapColor(TileMapData Map, Vector2 pos) {
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//			int color = typeColor[Map.Row[Row][Col]];
//
//			Novice::DrawBox(int(pos.x + (Col * colorStruct.eachWidth)), int(pos.y + (Row * colorStruct.eachHeight)),
//				colorStruct.eachWidth, colorStruct.eachHeight, 0, color, kFillModeSolid);
//			;
//		}
//	}
//}
//
//void ColorMatcher::DrawMapWord(TileMapData Map, Vector2 pos) {
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//
//			Novice::ScreenPrintf(int(pos.x + (Col * colorStruct.eachWidth)), int(pos.y + (Row * colorStruct.eachHeight)),
//				"%d", Map.Row[Row][Col]);
//		}
//	}
//}
//
//void ColorMatcher::DrawMapWord(TileMapData Map, Vector2 pos, int row, int column, Type type) {
//	pos, type;
//
//	for (int Row = 0; Row < Map.Row.size(); Row++) {
//		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
//			int LTop{ 0 }, MTop{ 0 }, RTop{ 0 }, LMid{ 0 }, TarG{ 0 }, RMid{ 0 }, LBot{ 0 }, MBot{ 0 }, RBot{ 0 };
//
//			if (Row != 0 && Col != 0) { LTop = { Map.Row[Row - 1][Col - 1] }; }
//			if (Row != 0) { MTop = { Map.Row[Row - 1][Col + 0] }; }
//			if (Row != 0 && Col != Map.Row[Row].size() - 1) { RTop = { Map.Row[Row - 1][Col + 1] }; }
//			if (Col != 0) { LMid = { Map.Row[Row + 0][Col - 1] }; }
//			{ TarG = { Map.Row[Row + 0][Col + 0] }; }
//			if (Col != Map.Row[Row].size() - 1) { RMid = { Map.Row[Row + 0][Col + 1] }; }
//			if (Row != Map.Row.size() - 1 && Col != 0) { LBot = { Map.Row[Row + 1][Col - 1] }; }
//			if (Row != Map.Row.size() - 1) { MBot = { Map.Row[Row + 1][Col + 0] }; }
//			if (Row != Map.Row.size() - 1 && Col != Map.Row[Row].size() - 1) { RBot = { Map.Row[Row + 1][Col + 1] }; }
//
//
//			if (Row == row && Col == column) {
//				Novice::ScreenPrintf(10, 10, "%d  %d  %d", LTop, MTop, RTop);
//				Novice::ScreenPrintf(10, 40, "%d  %d  %d", LMid, TarG, RMid);
//				Novice::ScreenPrintf(10, 70, "%d  %d  %d", LBot, MBot, RBot);
//			}
//		}
//	}
//}

void ColorMatcher::clipMap(TileMapData Map, TileMapData* MapSave, int type, int type2, int clipNum, bool isReverse) {
	int T1 = type, T2 = type2;
	if (isReverse) { T1 = type2, T2 = type; }
	for (int Row = 0; Row < Map.Row.size(); Row++) {
		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
			if (Map.Row[Row][Col] <= clipNum) { MapSave->Row[Row][Col] = T1; } else { MapSave->Row[Row][Col] = T2; }
		}
	}
}

void ColorMatcher::clipMap2D(TileMapData Map, TileMapData* MapSave, int type, int clipNum) {
	for (int Row = 0; Row < Map.Row.size(); Row++) {
		for (int Col = 0; Col < Map.Row[Row].size(); Col++) {
			if (Map.Row[Row][Col] == clipNum) { MapSave->Row[Row][Col] = type; }
		}
	}
}

#pragma endregion

