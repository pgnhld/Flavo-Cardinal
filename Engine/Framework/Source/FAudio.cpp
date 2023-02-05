#include "FAudio.h"
#include "Assertion.h"

framework::FAudio::~FAudio() {
	backgroundAudioEngine_->Suspend();
	ambientAudioEngine_->Suspend();
	clipAudioEngine_->Suspend();
}

framework::FAudio& framework::FAudio::getInstance() {
	static FAudio instance;
	return instance;
}

void framework::FAudio::playOnce2D(AudioClip2DType type) {
	if (type == AudioClip2DType::SHOOT) {
		shootClipInstance_->Stop();
		shootClipInstance_->Play();
		return;
	} 
	
	if (type == AudioClip2DType::BULLET_HIT_REACTION) {
		bulletHitReactionClipInstance_->Stop();
		bulletHitReactionClipInstance_->Play();
		return;
	}

	if (type == AudioClip2DType::PILL) {
		pillClipInstance_->Stop();
		pillClipInstance_->Play();
		return;
	}

	if (type == AudioClip2DType::PILL_REACTION) {
		pillReactionClipInstance_->Stop();
		pillReactionClipInstance_->Play();
		return;
	}

	const auto it = loadedSounds2D_.find(type);
	if (it != loadedSounds2D_.end()) {
		DirectX::SoundEffect* foundSound = it->second.get();
		foundSound->Play();
		return;
	}

	auto insertIt = loadedSounds2D_.insert({ type, std::make_unique<DirectX::SoundEffect>(backgroundAudioEngine_.get(), audioClip2DPaths_.at(type).c_str()) });
	ASSERT_CRITICAL(insertIt.second, "Couldn't insert AudioClip2D to map; You shouldn't ever see this error");
	insertIt.first->second->Play();
}

void framework::FAudio::playBackgroundMusic(BackgroundMusicType type) {
	if (currentBackgroundMusic_ != nullptr) {
		if (type == currentBackgroundMusicType_) {
			return;
		}
		currentBackgroundMusic_->Stop();
	}
	if (type == BackgroundMusicType::NONE) return;

	const auto it = loadedBackgroundMusic_.find(type);
	if (it != loadedBackgroundMusic_.end()) {
		DirectX::SoundEffect* foundSound = it->second.get();
		currentBackgroundMusic_ = foundSound->CreateInstance();
		currentBackgroundMusic_->Play(true);
		return;
	}

	auto insertIt = loadedBackgroundMusic_.insert({ type, std::make_unique<DirectX::SoundEffect>(backgroundAudioEngine_.get(), backgroundMusicPaths_.at(type).c_str()) });
	ASSERT_CRITICAL(insertIt.second, "Couldn't insert AudioClip2D to map; You shouldn't ever see this error");
	currentBackgroundMusic_ = insertIt.first->second->CreateInstance();
	currentBackgroundMusic_->Play(true);
	currentBackgroundMusicType_ = type;
}

void framework::FAudio::setWalkSound(bool bPlay) {
}

void framework::FAudio::update() {
	backgroundAudioEngine_->Update();
	clipAudioEngine_->Update();
	ambientAudioEngine_->Update();
}

framework::FAudio::FAudio()
	: backgroundAudioEngine_(std::make_unique<DirectX::AudioEngine>())
	, clipAudioEngine_(std::make_unique<DirectX::AudioEngine>())
	, ambientAudioEngine_(std::make_unique<DirectX::AudioEngine>()) {

	backgroundAudioEngine_->SetMasterVolume(0.5f);
	clipAudioEngine_->SetMasterVolume(0.5f);
	ambientAudioEngine_->SetMasterVolume(0.5f);
	backgroundMusicPaths_.emplace(BackgroundMusicType::DECHOST, L"../Data/Sounds/dechost.wav");
	backgroundMusicPaths_.emplace(BackgroundMusicType::PAPA_2138, L"../Data/Sounds/papa2138.wav");
	backgroundMusicPaths_.emplace(BackgroundMusicType::BACKGROUND, L"../Data/Sounds/flavo_cardinal_background.wav");
	backgroundMusicPaths_.emplace(BackgroundMusicType::BOSS_FIGHT, L"../Data/Sounds/flavo_cyborg_boss_fight.wav");

	audioClip2DPaths_.emplace(AudioClip2DType::SHOOT, L"../Data/Sounds/shoot.wav");
	audioClip2DPaths_.emplace(AudioClip2DType::BULLET_HIT_REACTION, L"../Data/Sounds/hit_reaction.wav");
	audioClip2DPaths_.emplace(AudioClip2DType::PILL, L"../Data/Sounds/pill.wav");
	audioClip2DPaths_.emplace(AudioClip2DType::PILL_REACTION, L"../Data/Sounds/co.wav");

	for (const auto& audioClip2DPath : audioClip2DPaths_) {
		loadedSounds2D_.emplace(audioClip2DPath.first, std::make_unique<DirectX::SoundEffect>(clipAudioEngine_.get(), audioClip2DPath.second.c_str()));
	}

	shootClipInstance_ = loadedSounds2D_[AudioClip2DType::SHOOT]->CreateInstance();
	bulletHitReactionClipInstance_ = loadedSounds2D_[AudioClip2DType::BULLET_HIT_REACTION]->CreateInstance();
	pillClipInstance_ = loadedSounds2D_[AudioClip2DType::PILL]->CreateInstance();
	pillReactionClipInstance_ = loadedSounds2D_[AudioClip2DType::PILL_REACTION]->CreateInstance();
}