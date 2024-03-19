#pragma once
#include "PushdownState.h"
#include "ApplicationState.h"
#include "Event.h"
#include "UIPlaystation.h"
#include "PS5Controller.h"

namespace NCL {
	namespace  CSC8503 {
		class PushdownMachine;
		class PS5_Game;

		class PS5MenuSystem
		{
		public:
			PS5MenuSystem(PS5_Game* Game);
			~PS5MenuSystem();

			void Update(float dt);

		private:
			PushdownMachine* MenuMachine;
		};

		/** Main Menu */
		class PS5MainMenu : public PushdownState, public EventListener
		{
		public:
			PS5MainMenu()
			{
				ui = UIPlaystation::GetInstance();
				appState = ApplicationState::GetInstance();
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			UIPlaystation* ui;
			ApplicationState* appState;
		};

		/** PlayingHUD */
		class PS5PlayingHUD : public PushdownState, public EventListener
		{
		public:
			PS5PlayingHUD()
			{
				ui = UIPlaystation::GetInstance();
				appState = ApplicationState::GetInstance();
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			UIPlaystation* ui;
			ApplicationState* appState;

			std::string GetRoundPowerUpState(PS5_Game* Game);
		};
	}
}
