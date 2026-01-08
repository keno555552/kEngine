#include "soundManager.h"
#include <algorithm>

SoundManager::SoundManager() {
	HRESULT result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

SoundManager::~SoundManager() {
	for(auto& ptr:sounds_){
		delete ptr;
	}
	sounds_.clear();
	resourceCounter = 0;

	if (masterVoice) { masterVoice->DestroyVoice(); masterVoice = nullptr; }
	xAudio2.Reset();
}

int SoundManager::SoundLoadSE(const char* filename) {
	{
		int i = 0;
		std::string fname = filename;
		for (auto& ptr : sounds_) {
			if(ptr->GetFileName() == filename) {
				return i;
			}
			i++;
		}
	}
	SoundUnit* sound = new SoundUnit;
	sound->SoundLoad(filename);
	sounds_.push_back(sound);
	return resourceCounter++;
}


void SoundManager::SoundPlaySE(int Handle, float volume) {
	float nSeVolume = cMasterVolume_ * cSeVolume_;
	nSeVolume = std::clamp(nSeVolume, 0.0f, 2.0f);
	if(masterMute_ || seMute_){
		sounds_[Handle - 1]->SoundPlaySE(xAudio2.Get(), 0.0f, volume);
	} else {
		sounds_[Handle - 1]->SoundPlaySE(xAudio2.Get(), nSeVolume, volume);
	}
}

void SoundManager::SoundPlayBGM(int Handle, float volume) {
	if (isPlaying(Handle))return;
	float nBGMVolume = cMasterVolume_ * cBgmVolume_;
	nBGMVolume = std::clamp(nBGMVolume, 0.0f, 2.0f);
	if (masterMute_ || bgmMute_) {
		sounds_[Handle - 1]->SoundPlayBGM(xAudio2.Get(), 0.0f, volume);
	}else {
		sounds_[Handle - 1]->SoundPlayBGM(xAudio2.Get(), nBGMVolume, volume);
	}
}


void SoundManager::SoundSetVolume(int Handle, float volume) {

	auto* sound = sounds_[Handle - 1];
	SoundUnit::Type type = sound->GetSoundType();


	float finalVolume = 0.0f;

	if (type == SoundUnit::Type::SE) {

		if (!masterMute_ &&
			!seMute_     &&
			!sound->SoundGetMute()) {
			finalVolume = std::clamp(cMasterVolume_ * cSeVolume_, 0.0f, 2.0f);
		}

		sound->SoundSetVolume(finalVolume, volume);
		return;
	}

	if (type == SoundUnit::Type::BGM) {

		if (!masterMute_ &&
			!bgmMute_ &&
			!sound->SoundGetMute()) {
			finalVolume = std::clamp(cMasterVolume_ * cSeVolume_, 0.0f, 2.0f);

		}

		sound->SoundSetVolume(finalVolume, volume);
		return;
	}
}

void SoundManager::SoundPause(int Handle) {
	sounds_[Handle - 1]->SoundPause();
};

void SoundManager::SoundContinue(int Handle) {
	sounds_[Handle - 1]->SoundContinue();
}
void SoundManager::SoundStop(int Handle) {
	sounds_[Handle - 1]->SoundStop();
}

void SoundManager::SoundSetMasterVolume(float volume) {
	if (volume == masterVolume_)return;

	masterVolume_ = volume;
	cMasterVolume_ = std::clamp(volume, 0.0f, 2.0f);

	nSeVolume_ = cMasterVolume_ * cSeVolume_;
	nSeVolume_ = std::clamp(nSeVolume_, 0.0f, 2.0f);

	nBGMVolume_ = cMasterVolume_ * cBgmVolume_;
	nBGMVolume_ = std::clamp(nBGMVolume_, 0.0f, 2.0f);

	if(masterMute_)return;
	for (auto ptr : sounds_) {
		if (ptr->GetSoundType() == SoundUnit::Type::SE) {
			if (!seMute_) {
				ptr->SoundSetMasterVolume(nSeVolume_);
			}
		}
		if (ptr->GetSoundType() == SoundUnit::Type::BGM) {
			if (!bgmMute_){
				ptr->SoundSetMasterVolume(nBGMVolume_);
			}
		}

	}

}

void SoundManager::SoundSetMasterSEVolume(float volume) {
	if (volume == seVolume_)return;

	seVolume_ = volume;
	cSeVolume_ = std::clamp(volume, 0.0f, 2.0f);

	nSeVolume_ = cMasterVolume_ * cSeVolume_;
	nSeVolume_ = std::clamp(nSeVolume_, 0.0f, 2.0f);

	if (masterMute_)return;
	if (seMute_)return;
	for (auto ptr : sounds_) {
		if (ptr->GetSoundType() == SoundUnit::Type::SE) {
			ptr->SoundSetMasterVolume(nSeVolume_);
		}
	}
}

void SoundManager::SoundSetMasterBGMVolume(float volume) {
	if (volume == bgmVolume_)return;

	bgmVolume_ = volume;
	cBgmVolume_ = std::clamp(volume, 0.0f, 2.0f);

	nBGMVolume_ = cMasterVolume_ * cBgmVolume_;
	nBGMVolume_ = std::clamp(nBGMVolume_, 0.0f, 2.0f);

	if (masterMute_)return;
	if (bgmMute_)return;
	for (auto ptr : sounds_) {
		if (ptr->GetSoundType() == SoundUnit::Type::BGM) {
			ptr->SoundSetMasterVolume(nBGMVolume_);
		}
	}
}


bool SoundManager::isPlaying(int Handle) {
	if (sounds_[Handle - 1]->isPlaying())return true;
	return false;
}

float SoundManager::SoundGetVolume(int Handle) const {
	return sounds_[Handle - 1]->GetfVolume();
}

void SoundManager::SoundSetMute(int Handle, bool isMute) {
	sounds_[Handle - 1]->SoundSetMute(isMute);
}

void SoundManager::SoundSetMasterMute(bool isMute) {
	if (isMute == masterMute_)return;
	masterMute_ = isMute;
	for (auto ptr : sounds_) {
		if (ptr->GetSoundType() == SoundUnit::Type::SE) {
			if (isMute) {
				ptr->SoundSetMasterVolume(0.0f);
			} else {
				if(!seMute_){
					ptr->SoundSetMasterVolume(nSeVolume_);
				}
			}
		}
		if (ptr->GetSoundType() == SoundUnit::Type::BGM) {
			if (isMute) {
				ptr->SoundSetMasterVolume(0.0f);
			} else {
				if (!bgmMute_) {
					ptr->SoundSetMasterVolume(nBGMVolume_);
				}
			}
		}
	}
}

void SoundManager::SoundSetBGMMute(bool isMute) {
	if (isMute == bgmMute_)return;
	bgmMute_ = isMute;
	if (masterMute_)return;
	for (auto ptr : sounds_) {
		if (ptr->GetSoundType() == SoundUnit::Type::BGM) {
			if (isMute) {
				ptr->SoundSetMasterVolume(0.0f);
			} else {
				ptr->SoundSetMasterVolume(nBGMVolume_);
			}
		}
	}
}

void SoundManager::SoundSetSEMute(bool isMute) {
	if (isMute == seMute_)return;
	seMute_ = isMute;
	if (masterMute_)return;
	for (auto ptr : sounds_) {
		if (ptr->GetSoundType() == SoundUnit::Type::SE) {
			if (isMute) {
				ptr->SoundSetMasterVolume(0.0f);
			} else {
				ptr->SoundSetMasterVolume(nSeVolume_);
			}
		}
	}
}

bool SoundManager::SoundGetMute(int Handle) {
	return sounds_[Handle - 1]->SoundGetMute();
}
