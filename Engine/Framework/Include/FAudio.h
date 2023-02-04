#pragma once

#include "Global.h"
#include <unordered_map>
#include "Audio.h"

namespace framework
{
	enum class BackgroundMusicType
	{
		NONE = -1,
		DECHOST,
		PAPA_2138,
		BACKGROUND,
		BOSS_FIGHT
	};

	enum class AudioClip2DType
	{
		SHOOT,
		ZAHID_LAUGH
	};

	class FAudio
	{
	public:
		~FAudio();
		static FAudio& getInstance();
		void playOnce2D(AudioClip2DType type);
		void playBackgroundMusic(BackgroundMusicType type);
		void setWalkSound(bool bPlay);
		void update();

	private:
		FAudio();
		std::unique_ptr<DirectX::AudioEngine> backgroundAudioEngine_;
		std::unique_ptr<DirectX::AudioEngine> walkAudioEngine_;
		std::unique_ptr<DirectX::AudioEngine> zahidAudioEngine_;
		std::unique_ptr<DirectX::AudioEngine> clipAudioEngine_;
		std::unique_ptr<DirectX::AudioEngine> ambientAudioEngine_;

		std::unique_ptr<DirectX::SoundEffectInstance> currentBackgroundMusic_;
		std::unique_ptr<DirectX::SoundEffectInstance> walkClipInstance_;
		std::unique_ptr<DirectX::SoundEffectInstance> shootClipInstance_;
		std::unique_ptr<DirectX::SoundEffectInstance> zahidClipInstance;

		std::unique_ptr<DirectX::SoundEffect> walkClip_;
		std::unique_ptr<DirectX::SoundEffect> zahidClip_;
		std::unique_ptr<DirectX::SoundEffect> shootClip_;

		std::unordered_map<AudioClip2DType, std::wstring> audioClip2DPaths_;
		std::unordered_map<AudioClip2DType, std::unique_ptr<DirectX::SoundEffect>> loadedSounds2D_;

		std::unordered_map<BackgroundMusicType, std::wstring> backgroundMusicPaths_;
		std::unordered_map<BackgroundMusicType, std::unique_ptr<DirectX::SoundEffect>> loadedBackgroundMusic_;

		BackgroundMusicType currentBackgroundMusicType_ = BackgroundMusicType::NONE;
	};
}
