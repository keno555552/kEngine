#include "soundUnit.h"
#include <algorithm>

SoundUnit::SoundUnit() {

}

SoundUnit::~SoundUnit() {
	SoundUnload();
}



SoundData SoundUnit::SoundLoadWave(const char* filename) {

	/// 1.フェイルオープン
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());
	/// 2..wavデータ読み込みsdf
	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}
	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	/// 3.フェイルクローズ
	// Dataチャンクの読み込み
	ChunkHeader data;
	while (true) {
		file.read((char*)&data, sizeof(data));
		if (strncmp(data.id, "data", 4) == 0) {
			break;
		}
		// スキップ
		file.seekg(data.size, std::ios_base::cur);
	}


	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	// Dataチャンクのデータ型(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	// Waveファイルを閉じる
	file.close();

	/// 4.読み込んだ音声データをreturn
	soundData->wfex = format.fmt;
	soundData->pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData->bufferSize = data.size;
	return *soundData;
}

SoundData SoundUnit::SoundLoad(const char* filename) {
	delete[] filename_; // 釋放舊的
	size_t len = std::strlen(filename) + 1;
	filename_ = new char[len];
	errno_t err = strcpy_s(filename_, len, filename);
	if (err != 0) {
	}
	return SoundLoadWave(filename);

}

void SoundUnit::SoundUnload() {
	/// バッファのメモリを解放
	if (pSourceVoice_) {
		pSourceVoice_->Stop();
		pSourceVoice_->FlushSourceBuffers();
		pSourceVoice_->DestroyVoice();
		pSourceVoice_ = nullptr;
	}
	if (!pSourceVoiceGroup.empty()) {
		for (auto ptr : pSourceVoiceGroup) {
			if (ptr) {
				ptr->Stop();
				ptr->FlushSourceBuffers();
				ptr->DestroyVoice();
			}
		}
		pSourceVoiceGroup.clear();
	}

	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}



void SoundUnit::SoundPlaySE(IXAudio2* xAudio2, float cVolume, float volume) {
	HRESULT result;

	/// 波形フォーマットを元にSourceVoiceの生成（SE は再生中フラグ不要なので callback なし）
// >>6
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData->wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr);
	pSourceVoiceGroup.push_back(pSourceVoice);
// <<6
	assert(SUCCEEDED(result));

	soundType = Type::SE;

	/// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData->pBuffer;
	buf.AudioBytes = soundData->bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	/// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	volume_ = volume;
	cVolume_ = cVolume;
	iVolume_ = std::clamp(volume, 0.0f, 2.0f);
	fVolume_ = std::clamp(iVolume_ * cVolume, 0.0f, 2.0f);
	if (!isMute_) {
		pSourceVoice->SetVolume(fVolume_);
	} else {
		pSourceVoice->SetVolume(0.0f);
	}
}

void SoundUnit::SoundPlayBGM(IXAudio2* xAudio2, float cVolume, float volume) {
	HRESULT result;

	/// 波形フォーマットを元にSourceVoiceの生成
	pSourceVoice_ = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice_, &soundData->wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voiceCallBack_);
	assert(SUCCEEDED(result));

	soundType = Type::BGM;

	/// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData->pBuffer;
	buf.AudioBytes = soundData->bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;

	/// 波形データの再生
	result = pSourceVoice_->SubmitSourceBuffer(&buf);
	result = pSourceVoice_->Start();
	assert(SUCCEEDED(result));

	volume_ = volume;
	cVolume_ = cVolume;
	iVolume_ = std::clamp(volume, 0.0f, 2.0f);
	fVolume_ = std::clamp(iVolume_ * cVolume, 0.0f, 2.0f);
	if (!isMute_) {
		pSourceVoice_->SetVolume(fVolume_);
	} else {
		pSourceVoice_->SetVolume(0.0f);
	}
}

void SoundUnit::SoundSetVolume(float cVolume, float volume) {
	if (!isMute_) {
		if (pSourceVoice_) {
			if (volume == volume_ && cVolume_ == cVolume)return;
			volume_ = volume;
			cVolume_ = cVolume;
			iVolume_ = std::clamp(volume, 0.0f, 2.0f);
			fVolume_ = std::clamp(iVolume_ * cVolume, 0.0f, 2.0f);
			pSourceVoice_->SetVolume(fVolume_);
		}
		if (!pSourceVoiceGroup.empty()) {
			for (auto ptr : pSourceVoiceGroup) {
				if (cVolume_ == cVolume)return;
				cVolume_ = cVolume;
				fVolume_ = std::clamp(iVolume_ * cVolume, 0.0f, 2.0f);
				ptr->SetVolume(fVolume_);
			}
		}
	}
}

void SoundUnit::SoundSetMasterVolume(float cVolume) {
	if (!isMute_) {
		if (pSourceVoice_) {
			if (cVolume_ == cVolume)return;
			cVolume_ = cVolume;
			fVolume_ = std::clamp(iVolume_ * cVolume, 0.0f, 2.0f);
			pSourceVoice_->SetVolume(fVolume_);
		}
		if (!pSourceVoiceGroup.empty()) {
			if (cVolume_ == cVolume)return;
			cVolume_ = cVolume;
			fVolume_ = std::clamp(iVolume_ * cVolume, 0.0f, 2.0f);
			for (auto ptr : pSourceVoiceGroup) {
				ptr->SetVolume(fVolume_);
			}
		}
	}
}

void SoundUnit::SoundPause() {
	if (pSourceVoice_) {
		pSourceVoice_->Stop();
		isPause_ = true;
		if (voiceCallBack_) { voiceCallBack_->isBGMPlaying = false; }
	}
	if (!pSourceVoiceGroup.empty()) {
		for (auto ptr : pSourceVoiceGroup) {
			if (ptr) { ptr->Stop(); }
			isPause_ = true;
		}
	}
}

void SoundUnit::SoundContinue() {
	if (pSourceVoice_) {
		pSourceVoice_->Start();
		isPause_ = false;
		if (voiceCallBack_) { voiceCallBack_->isBGMPlaying = true; }
	}
	if (!pSourceVoiceGroup.empty()) {
		for (auto ptr : pSourceVoiceGroup) {
			if (ptr) { ptr->Start(); }
			isPause_ = false;
		}
	}
}

void SoundUnit::SoundStop() {
	if (pSourceVoice_) {
		pSourceVoice_->Stop();
		pSourceVoice_->FlushSourceBuffers();
		isPause_ = false;
		if (voiceCallBack_) { voiceCallBack_->isBGMPlaying = false; }
	}
	if (!pSourceVoiceGroup.empty()) {
		for (auto ptr : pSourceVoiceGroup) {
			if (ptr) { ptr->Stop(); }
			isPause_ = false;
		}
	}
}

bool SoundUnit::isPlaying() {
	if (soundType == Type::BGM && voiceCallBack_ != nullptr) {
		if (voiceCallBack_->isBGMPlaying && !isPause_) {
			return true;
		}
	}
	return false;
}


void SoundUnit::SoundSetMute(bool isMute) {
	if (isMute == isMute_)return;
	isMute_ = isMute;
	float fVolume = 0.0f;
	if (!isMute)fVolume = fVolume_;

	if (pSourceVoice_) {
			pSourceVoice_->SetVolume(fVolume);
	}
	if (!pSourceVoiceGroup.empty()) {
		for (auto ptr : pSourceVoiceGroup) {
			if (ptr) { ptr->SetVolume(fVolume); }
		}
	}
}
