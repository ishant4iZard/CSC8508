#include "PS5MenuSystem.h"
#include "PS5_Game.h"
#include "PushdownMachine.h"
#include "UIBase.h"

using namespace NCL;
using namespace CSC8503;

PS5MenuSystem::PS5MenuSystem(PS5_Game* Game)
{
	MenuMachine = new PushdownMachine(new PS5MainMenu());
	MenuMachine->SetGame(Game);
}

PS5MenuSystem::~PS5MenuSystem()
{
	delete MenuMachine;
}

void PS5MenuSystem::Update(float dt)
{
	MenuMachine->Update(dt);
}

/** Main Menu Update */
PushdownState::PushdownResult PS5MainMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (game)
	{
		PS5_Game* Game = (PS5_Game*)game;

		ui->DrawStringText("Press Cross: Solo Game", Vector2(10, 23), UIBase::WHITE);

		ui->DrawStringText("Press Circle: Quit Game", Vector2(10, 38), UIBase::WHITE);

		if (Game->GetController()->GetNamedButton("Cross"))
		{
			Game->StartLevel();
			*newState = new PS5PlayingHUD();
			return PushdownResult::Push;
		}
		if (Game->GetController()->GetNamedButton("Circle"))
		{
			Game->CloseGame = true;
		}

	}

	return PushdownResult::NoChange;
}

void PS5MainMenu::ReceiveEvent(const EventType eventType)
{

}

/** PlaingHUD update */
PushdownState::PushdownResult PS5PlayingHUD::OnUpdate(float dt, PushdownState** newState)
{
	if (game)
	{
		PS5_Game* Game = (PS5_Game*)game;

		/** Game going HUD */
		if (!appState->GetIsGameOver())
		{
			if (!appState->GetIsGamePaused())
			{
				ui->DrawStringText("Time Left:  " + std::to_string((int)(Game->GetTimeLeft())), Vector2(3, 15), UIBase::WHITE);
				ui->DrawStringText("Your Score: " + std::to_string(Game->GetPlayerScore()), Vector2(3, 25), UIBase::WHITE);
				ui->DrawStringText("Bullets:    " + std::to_string(Game->GEtPlayerBulletsNum()), Vector2(3, 35), UIBase::WHITE);
				ui->DrawStringText("PowerUp State:  ", Vector2(3, 45), UIBase::WHITE);
				ui->DrawStringText(GetRoundPowerUpState(Game), Vector2(3, 50), UIBase::WHITE);

				ui->DrawStringText("Press Option:", Vector2(3, 60), UIBase::WHITE);
				ui->DrawStringText("Pause Game", Vector2(3, 65), UIBase::WHITE);

				if (Game->GetController()->GetNamedButton("Option"))
				{
					appState->SetIsGamePaused(true);
				}
			}

			if (appState->GetIsGamePaused())
			{
				ui->DrawStringText("Round Paused !", Vector2(40, 15), UIBase::WHITE);
				ui->DrawStringText("Press Circle: Unpause", Vector2(30, 45), UIBase::WHITE);
				ui->DrawStringText("Press Cross: End Round", Vector2(30, 55), UIBase::WHITE);

				if (Game->GetController()->GetNamedButton("Circle"))
				{
					appState->SetIsGamePaused(false);
				}

				if (Game->GetController()->GetNamedButton("Cross"))
				{
					appState->SetIsGameOver(true);
				}
			}
		}
		else
		{
			ui->DrawStringText("Round Over !", Vector2(40, 15), UIBase::WHITE);
			ui->DrawStringText("Your Score: " + std::to_string(Game->GetPlayerScore()), Vector2(25, 45), UIBase::WHITE);
			ui->DrawStringText("Press R1: Main Menu ", Vector2(25,55), UIBase::WHITE);

			if (Game->GetController()->GetNamedButton("R1"))
			{
				Game->EndLevel();
				return PushdownResult::Pop;
			}
		}
	}
	return PushdownResult::NoChange;
}

void PS5PlayingHUD::ReceiveEvent(const EventType eventType)
{

}

std::string PS5PlayingHUD::GetRoundPowerUpState(PS5_Game* Game)
{
	std::string state;
	switch (Game->GetActivatedPowerUpType())
	{
	case powerUpType::none:
		state = std::string("No powerup");
		break;

	case powerUpType::ice:
		state = std::string("ice");
		break;

	case powerUpType::sand:
		state = std::string("sand");
		break;

	case powerUpType::wind:
		state = std::string("wind");
		break;

	default:
		break;
	}
	return state;
}