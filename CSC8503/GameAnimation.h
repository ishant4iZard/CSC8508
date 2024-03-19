#pragma once

#include <vector>

namespace NCL {
	namespace CSC8503 {
		class MaleGuard;
		class MaxGuard;
		class GameObject;

		/*typedef std::vector<MaleGuard*>::const_iterator AnimatedObjectIterator;
		typedef std::vector<MaxGuard*>::const_iterator AnimatedObjectIterator;*/

		class GameAnimation {
		public:
			GameAnimation();
			~GameAnimation();

			void AddAnimatedObject(MaleGuard* animatedObject);
			void AddAnimatedObject(MaxGuard* animatedObject);
			void AddAnimatedObject(GameObject* animatedObject);
			void RemoveAnimatedObject(MaleGuard* animatedObject, bool andDelete = false);
			void RemoveAnimatedObject(MaxGuard* animatedObject, bool andDelete = false);

			void Clear();

		protected:
			std::vector<GameObject*> animatedObjects;

			int anmObjectsIDCounter;
			int anmObjectsStateCounter;
		};
	}
}