#pragma once

#include <vector>

namespace NCL {
	namespace CSC8503 {
		class MaleGuard;

		typedef std::vector<MaleGuard*>::const_iterator AnimatedObjectIterator;

		class GameAnimation {
		public:
			GameAnimation();
			~GameAnimation();

			void AddAnimatedObject(MaleGuard* animatedObject);
			void RemoveAnimatedObject(MaleGuard* animatedObject, bool andDelete = false);

			void Clear();

		protected:
			std::vector<MaleGuard*> animatedObjects;

			int anmObjectsIDCounter;
			int anmObjectsStateCounter;
		};
	}
}