#include "ApplicationState.h"

ApplicationState* ApplicationState::appState = nullptr;

 ApplicationState* ApplicationState::GetInstance() {
	if (appState == nullptr)
		appState = new ApplicationState();

	return appState;
}

void ApplicationState::Destory() {
	if (appState == nullptr)
		return;

	delete appState;
}