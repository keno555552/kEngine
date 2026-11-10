#pragma once
#include <string>
#include <vector>
#include "Tool/EffectSystem/ParticleSystem/EffectData.h"

#include <fstream>
#include "externals/nlohmann/json.hpp"

/// 保存先の計画（書く前に「どこに書くか」を全部決めておく → 上書き確認に使う）
struct EffectSavePlan {
	std::string effectPath;                  // Effectファイル
	std::vector<std::string> particlePaths;  // effect.prototypes と同じ順番
	std::vector<std::string> linkPaths;      // effect.links と同じ順番
};

class kEngine;
class EffectAdapter
{
public:
	EffectAdapter(kEngine* system) : system_(system) {}

	/// ファイルのバージョン（形式を変えたら上げる）
	static constexpr int kFileVersion = 1;
	/// 各ファイルの kind（Load時に種類をチェックする）
	static constexpr const char* kKindEffect = "Effect";
	static constexpr const char* kKindParticle = "Particle";
	static constexpr const char* kKindLink = "EmitterLink";

	/// json形式のファイルからEffectDataを読み込む（次のステップ）
		/// --- 読み込み ---
	/// log には警告・エラーを1行ずつ追記する（UIの Debug Log 用）
	/// Effect：中の Particle / Link も一緒に読む。読めなかった単元は飛ばす
	bool LoadEffect(const std::string& effectPath,
						   EffectData& out, 
						  std::string* log = nullptr);
	/// 単体（Import用。LoadEffect の中でも使う）。savePath = 読んだファイル
	bool LoadParticle(const std::string& path, 
					  ParticlePrototype& out, 
							std::string* log = nullptr);
	bool LoadEmitterLink(const std::string& path, 
							   EmitterLink& out, 
							   std::string* log = nullptr);

	/// --- 保存（2段階） ---
	/// 保存先を決める（ファイルは書かない）
	///    savePath がある単元はそこへ、無い単元は <Effectフォルダ>/Particle/, EmitterLink/ へ
	EffectSavePlan PlanEffect(const EffectData& effect, const std::string& effectPath) const;
	/// 書いたパスは各 savePath に書き戻す（だから非 const）
	bool WriteEffect(EffectData& effect,
		   const EffectSavePlan& plan,
							bool useRelativePath, 
					std::string* error = nullptr);
	/// 単体で書く（Particle / EmitterLink のSave用。WriteEffect の中でも使う）
	bool WriteParticle(const ParticlePrototype& proto, 
							 const std::string& path,
										   bool useRelativePath, 
								   std::string* error = nullptr);
	bool WriteEmitterLink(const EmitterLink& link, 
						  const std::string& path,
								std::string* error = nullptr);

private:
	kEngine* system_ = nullptr;		// 借り
	bool useRelativePath_ = true;	// 書いている間だけ使う

	/// --- 単体 → json ---
	/// baseDir = そのjsonが置かれるフォルダ（相対パスの基準）
	nlohmann::json ParticleToJson(const ParticlePrototype& proto, const std::string& baseDir) const;
	nlohmann::json EmitterLinkToJson(const EmitterLink& link) const;
	nlohmann::json ObjectToJson(const ObjectData& obj, const std::string& baseDir) const;
	nlohmann::json ModelToJson(int modelHandle, const std::string& baseDir) const;

	/// --- パス関連 ---
	std::string GetModelPath(int modelHandle) const;
	std::string ToSavePath(const std::string& path, const std::string& baseDir) const;

	/// --- json → 単体 ---
	/// baseDir = そのjsonが置かれているフォルダ（相対パスの基準）
	ParticlePrototype JsonToParticle(const nlohmann::json& j, const std::string& baseDir, std::string* log);
	EmitterLink JsonToEmitterLink(const nlohmann::json& j) const;
	ObjectData JsonToObject(const nlohmann::json& j, const std::string& baseDir, std::string* log);
	/// model の json → モデルハンドル。ファイルモデルなら isFileModel = true
	int JsonToModel(const nlohmann::json& model, const std::string& baseDir, bool* isFileModel, std::string* log);
	/// 保存されたテクスチャパス → ハンドル（空・無いファイルは白）
	int LoadTextureFrom(const std::string& rawPath, const std::string& baseDir, std::string* log);
};