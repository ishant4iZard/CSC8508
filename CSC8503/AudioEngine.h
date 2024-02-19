#pragma once

#include "Event.h"

#include <fmod_studio.hpp>
#include <fmod.hpp>

#include <map>

class AudioEngine {
public :
	AudioEngine() {
		result = FMOD::System_Create(&system);
		result = system->init(512, FMOD_INIT_NORMAL, nullptr);
	}
	
	~AudioEngine(){
		for (auto i : loopedSounds) {
			i.second->release();
		}
		for (auto i : oneShotSounds) {
			i.second->release();
		}
		system->release();
		channel->stop();
	}

	int CreateSound(std::string fileName, bool isLooped);
	void PlaySound(int indx, bool isLooped);
	float GetVolume() {	return volume; };
	void SetVolume(float val);

	void Update() {	system->update(); }

protected :
	FMOD::System* system;
	FMOD::Channel* channel;
	FMOD_RESULT result;

	std::map<int, FMOD::Sound*> loopedSounds;
	std::map<int, FMOD::Sound*> oneShotSounds;
	int soundCount = 0;
	float volume = 1;
};