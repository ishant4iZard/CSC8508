#include "MaleGuard.h"
#include <TextureLoader.h>

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

MaleGuard::MaleGuard(const std::string& objectName) :GameObject(name) {
	maleGuardMaterial = new MeshMaterial("Male_Guard.mat");

	maleGuardName = objectName;
	name = objectName;

	anmNames[IDLE]			= "Idle1";
	anmNames[GUNFIRE1]		= "Gunfire1";
	anmNames[STEPFORWARD]	= "StepForward";
	anmNames[HAPPY]			= "Happy";

	for (int i = 0; i < NUM_ANMS; i++) {
		AnmName anm = static_cast<AnmName>(i);
		maleGuardAnimations[anm] = new MeshAnimation(anmNames[anm] + ".anm");
	}

	activeAnimation = maleGuardAnimations[IDLE];
	nextAnimation = activeAnimation;
}
