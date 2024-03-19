#include "Projectile.h"
#include "TutorialGame.h"
#include "NetworkPlayer.h"

using namespace NCL;
using namespace CSC8503;

int Projectile::CurrentAvailableProjectileID = 1000;

Projectile::Projectile(NetworkPlayer* Owner, TutorialGame* Game)
{
	this->Owner = Owner;
	this->Game = Game;
	this->settag("Projectile");
	timeLeft = LIFE_TIME_AMOUNT;
	SetRenderObject(new RenderObject(&GetTransform(), Game->GetSphereMesh(), nullptr, Game->GetPbrShader()));

	for (uint8_t i = (uint8_t)TextureType::ALBEDO; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		GetRenderObject()->SetTexture((TextureType)i, Game->goldTextureList[i]);
	}
}

Projectile::~Projectile()
{
}

void Projectile::OnCollisionBegin(GameObject* otherObject)
{
	//std::cout << transform.GetPosition().y<<"\n";
}
