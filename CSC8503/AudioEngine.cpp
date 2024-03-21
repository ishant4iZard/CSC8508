#include "AudioEngine.h"

int AudioEngine::CreateSound(std::string fileName, bool isLooped) {
	FMOD::Sound* sound;
	result = system->createSound(
		fileName.c_str(), 
		(isLooped ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF), 
		nullptr, 
		&sound
	);

	if (result == FMOD_ERR_FILE_NOTFOUND)
	{
		std::cerr << "Error: File '" << fileName << "' does not exist\n";
		return -1;
	}

	if (isLooped)
		loopedSounds[soundCount] = sound;
	else
		oneShotSounds[soundCount] = sound;
	
	soundCount++;
	return soundCount - 1;
}

void AudioEngine::PlaySound(int indx, bool isLooped) {
	if (loopedSounds.find(indx) == loopedSounds.end() && oneShotSounds.find(indx) == oneShotSounds.end()) 
		return;

	FMOD::Sound* sound = isLooped ? loopedSounds[indx] : oneShotSounds[indx];

	system->playSound( 
		sound,
		nullptr, 
		true, 
		&channel);
	channel->setVolume(volume);
	channel->setPaused(false);
}

void AudioEngine::SetVolume(float volume) {
	if (volume < 0 || volume > 1)
		return;

	this->volume = volume;

	channel->setPaused(true);
	channel->setVolume(volume);
	channel->setPaused(false);
}

void AudioEngine::Pause()
{
	channel->setPaused(true);
}

void AudioEngine::UnPause()
{
	channel->setPaused(false);
}
