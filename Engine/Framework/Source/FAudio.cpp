#include "FAudio.h"
#include "Assertion.h"

framework::FAudio::~FAudio() {
	walkClipInstance_.reset();
	currentBackgroundMusic_.reset();
	shootClipInstance_.reset();
	zahidClipInstance.reset();

	walkClip_.reset();
	shootClip_.reset();
	zahidClip_.reset();

	backgroundAudioEngine_->Suspend();
	walkAudioEngine_->Suspend();
	ambientAudioEngine_->Suspend();
	clipAudioEngine_->Suspend();
	zahidAudioEngine_->Suspend();
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
	
	if (type == AudioClip2DType::ZAHID_LAUGH) {
		zahidClipInstance->Stop();
		//zahidClipInstance->SetVolume(3.0f);
		zahidClipInstance->Play();
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
	DirectX::SoundState state = walkClipInstance_->GetState();
	if (bPlay) {
		walkClipInstance_->Play(true);
	}
	else {
		walkClipInstance_->Stop();
	}
}

void framework::FAudio::update() {
	backgroundAudioEngine_->Update();
	clipAudioEngine_->Update();
	walkAudioEngine_->Update();
	ambientAudioEngine_->Update();
}

framework::FAudio::FAudio()
	: backgroundAudioEngine_(std::make_unique<DirectX::AudioEngine>()),
	  walkAudioEngine_(std::make_unique<DirectX::AudioEngine>()),
	  zahidAudioEngine_(std::make_unique<DirectX::AudioEngine>()),
	  clipAudioEngine_(std::make_unique<DirectX::AudioEngine>()),
	  ambientAudioEngine_(std::make_unique<DirectX::AudioEngine>()),
	  walkClip_(std::make_unique<DirectX::SoundEffect>(walkAudioEngine_.get(), L"../Data/Sounds/footsteps-metal_stairs-loop.wav")),
	  zahidClip_(std::make_unique<DirectX::SoundEffect>(zahidAudioEngine_.get(), L"../Data/Sounds/zahid_stutter.wav")),
	  shootClip_(std::make_unique<DirectX::SoundEffect>(clipAudioEngine_.get(), L"../Data/Sounds/sci-fi_weapon_blaster_laser_boom_zap_08.wav")) {
	backgroundAudioEngine_->SetMasterVolume(0.5f);
	clipAudioEngine_->SetMasterVolume(0.5f);
	walkAudioEngine_->SetMasterVolume(0.5f);
	zahidAudioEngine_->SetMasterVolume(2.5f);
	ambientAudioEngine_->SetMasterVolume(0.5f);
	backgroundMusicPaths_.emplace(BackgroundMusicType::DECHOST, L"../Data/Sounds/dechost.wav");
	backgroundMusicPaths_.emplace(BackgroundMusicType::PAPA_2138, L"../Data/Sounds/papa2138.wav");
	backgroundMusicPaths_.emplace(BackgroundMusicType::BACKGROUND, L"../Data/Sounds/flavo_cardinal_background.wav");
	backgroundMusicPaths_.emplace(BackgroundMusicType::BOSS_FIGHT, L"../Data/Sounds/flavo_cyborg_boss_fight.wav");

	audioClip2DPaths_.emplace(AudioClip2DType::ZAHID_LAUGH, L"../Data/Sounds/zahid_stutter.wav");

	walkClipInstance_ = walkClip_->CreateInstance();
	zahidClipInstance = zahidClip_->CreateInstance();
	shootClipInstance_ = shootClip_->CreateInstance();
}