#pragma once
#include "GameObject.h"
#include "RenderObject.h"

namespace NCL {
	namespace  CSC8503 {
		class NetworkPlayer;
		class NetworkedGame;

		class BouncePad : public GameObject
		{
		public:
			static constexpr float FireForce = 40000;
			static constexpr float inverseMass = 1.0 / 10.0;

			static int CurrentAvailableProjectileID;

			BouncePad(NetworkPlayer* inOwner, NetworkedGame* inGame, Mesh* inMesh, Texture* inTex, Shader* inShader);
			BouncePad(Mesh* inMesh, Texture* inTex, Shader* inShader, const Vector3& inScale);
			~BouncePad();

		protected:
			NetworkedGame* Game;
			NetworkPlayer* Owner;

			Vector3 scale = Vector3(50,50,6);
		};

	}
}