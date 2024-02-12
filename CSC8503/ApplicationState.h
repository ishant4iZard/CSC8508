#pragma once

class ApplicationState {
public :
	static ApplicationState*	GetInstance();
	static void					Destory();

	void SetIsServer		(const bool &val) { isServer = val; }
	void SetIsClient		(const bool& val) { isClient = val; }
	void SetIsGamePaused	(const bool& val) { isGamePaused = val; }
	void SetIsGameOver		(const bool &val) { isGameOver = val; }
	void SetHasWonGame		(const bool &val) { hasWonGame = val; }

	bool GetIsServer()		const { return isServer; }
	bool GetIsClient()		const { return isClient; }
	bool GetIsGamePaused()	const { return isGamePaused; }
	bool GetIsGameOver()	const { return isGameOver; }
	bool GetHasWonGame()	const { return hasWonGame; }

protected :
	static ApplicationState* appState;

	bool isServer		= false;
	bool isClient		= false;
	bool isGamePaused	= false;
	bool isGameOver		= false;
	bool hasWonGame		= false;
};