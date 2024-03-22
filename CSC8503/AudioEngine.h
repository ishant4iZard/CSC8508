#pragma once

#include "Event.h"

#include <fmod_studio.hpp>
#include <fmod.hpp>

#include <map>
#include <stdlib.h>

#ifndef _WIN32
#include <assert.h>
#endif // !_WIN32

class AudioEngine {
public :
	AudioEngine() {
#ifndef _WIN32
		// Does not work :-(
		SceKernelModule fmodModuleId = sceKernelLoadStartModule("./libfmodL.prx", 0, 0, 0, NULL, NULL);
		assert(fmodModuleId < 0 && "Failed to load module");
		
		fmodModuleId = sceKernelLoadStartModule("./libfmod.prx", 0, 0, 0, NULL, NULL);
		assert(fmodModuleId < 0 && "Failed to load module");		
		
		fmodModuleId = sceKernelLoadStartModule("./libfmodstudio.prx", 0, 0, 0, NULL, NULL);
		assert(fmodModuleId < 0 && "Failed to load module");

		fmodModuleId = sceKernelLoadStartModule("./libfmodstudioL.prx", 0, 0, 0, NULL, NULL);
		assert(fmodModuleId < 0 && "Failed to load module");
#endif

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

	void PauseAll();
	void UnPauseAll();

	void RemoveMusic(int indx) {
		FMOD::Sound* soundToRemove = loopedSounds[indx];

		loopedSounds.erase(indx);

		if (soundToRemove != nullptr) {
			soundToRemove->release();
		}
	}

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