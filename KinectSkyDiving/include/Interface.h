
/*
-----------------------------------------------------------------------------
Filename:    Interface.h
-----------------------------------------------------------------------------
This code is created by Dobromir Todorov
dobromir.todorov.todorov@gmail.com
-----------------------------------------------------------------------------
*/

#ifndef _Interface_h_
#define _Interface_h_

#include "Hikari.h"

class App;	// forward declaration

class Interface
{

public:
	Interface(App* main);
	virtual ~Interface(void);
	
	void setupHikari(void);
	void showMainMenu(void);

	Hikari::FlashValue onExit(Hikari::FlashControl* caller, const Hikari::Arguments& args);
	Hikari::FlashValue onPlay(Hikari::FlashControl* caller, const Hikari::Arguments& args);
	Hikari::FlashValue onOpen(Hikari::FlashControl* caller, const Hikari::Arguments& args);

	inline Hikari::HikariManager* getHikariManager(void) { return this->hikariMgr; }
	inline Hikari::FlashControl* getStartMenu(void) { return startMenu; }	
	inline Hikari::FlashControl* getGameDisplay(void) { return gameDisplay; }
	inline bool getArrowVisibility(void) { return isArrowVisible;}
	inline bool getGameOverStatus(void) { return isGameOver; }

	void updateArrow(Ogre::Real direction);
	void hideArrow();
	void showArrow();
	void updateScore(int n);
	void updateAltitude(Ogre::Real alt);
	void birdAttack(int numAtk);
	void gameOver(void);

private:
	App* main;
	Hikari::HikariManager* hikariMgr;
	Hikari::FlashControl* startMenu;
	Hikari::FlashControl* gameDisplay;

	bool isArrowVisible;
	int score;
	int numAttacked;
	bool isGameOver;

	//void onShot();
	//void onGoal();
	//void throwMeat();
	//void getMeat();
	//void addLife();
	//void getLife();
};

#endif

