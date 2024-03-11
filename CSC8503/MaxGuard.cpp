#include "MaxGuard.h"
#include <TextureLoader.h>

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

MaxGuard::MaxGuard(const std::string& objectName) :GameObject(name) {
	maxGuardMaterial = new MeshMaterial("Rig_Maximilian.mat");

	maxGuardName = objectName;
	name = objectName;

	anmNames[IDLE_MAX] = "Idle_Max";
	anmNames[SINGLESHOT] = "SingleShot";
	anmNames[WALK] = "Walk";
	anmNames[RUN] = "Run";

	for (int i = 0; i < NUM_ANMS2nd; i++) {
		AnmName2nd anm = static_cast<AnmName2nd>(i);
		maxGuardAnimations[anm] = new MeshAnimation(anmNames[anm] + ".anm");
	}

	activeAnimation = maxGuardAnimations[IDLE_MAX];
	nextAnimation = activeAnimation;
}
