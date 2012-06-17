

/*
-----------------------------------------------------------------------------
Filename:    Interface.cpp
-----------------------------------------------------------------------------
This code is created by Dobromir Todorov
dobromir.todorov.todorov@gmail.com
-----------------------------------------------------------------------------
*/

#include "Stdafx.h"
#include "Interface.h"
#include "App.h"

Interface::Interface(App* main) : 
	hikariMgr(0),
	score(0),
	numAttacked(0),
	isArrowVisible(true),
	isGameOver(false)
{
	this->main = main;
}

Interface::~Interface(void)
{
}

void Interface::setupHikari(void)
{
	using namespace Hikari;

	hikariMgr = HikariManager::GetPointer();

	if(!hikariMgr)
	{
		hikariMgr = new HikariManager("..\\..\\media\\UI");		
	}

	startMenu = hikariMgr->createFlashOverlay("MainMenu", 
		main->getCamera()->getViewport(),  
		main->getCamera()->getViewport()->getActualWidth(), 
		main->getCamera()->getViewport()->getActualHeight(), 
		Position(TopLeft));

	startMenu->load("UI.swf");
	startMenu->bind("START", FlashDelegate(this, &Interface::onPlay));
	startMenu->bind("EXIT", FlashDelegate(this, &Interface::onExit));
	startMenu->setDraggable(false);
	startMenu->setTransparent(true, true);
	//startMenu->hide();

	gameDisplay = hikariMgr->createFlashOverlay("inGameUI",
		main->getCamera()->getViewport(), 
		main->getCamera()->getViewport()->getActualWidth(), 
		main->getCamera()->getViewport()->getActualHeight(), 
		Position(TopLeft));
		
	gameDisplay->load("inGameUI.swf");
	gameDisplay->bind("OPEN", FlashDelegate(this, &Interface::onOpen));
	gameDisplay->setDraggable(false);
	gameDisplay->setTransparent(true, true);
	gameDisplay->hide();	
}

void Interface::showMainMenu(void)
{
	startMenu->show();
}

void Interface::updateArrow(Ogre::Real direction)
{
	gameDisplay->callFunction("target", Hikari::Args(static_cast<int>(direction)));
}

void Interface::hideArrow()
{
	isArrowVisible = false;
	gameDisplay->callFunction("hideArrow");
}

void Interface::showArrow()
{
	isArrowVisible = true;
	gameDisplay->callFunction("showArrow");
}

void Interface::updateAltitude(Ogre::Real alt)
{
	gameDisplay->callFunction("updateAltitude", Hikari::Args(alt));
}

void Interface::updateScore(int n)
{
	int dif = n - score;

	//if (dif != 0) std::cout << dif << "\n";

	if(dif == 100)
	{
		gameDisplay->callFunction("blue");
	}
	else if(dif == -100)
	{
		gameDisplay->callFunction("red");
	}

	score = n;
}

void Interface::birdAttack(int numAtk)
{
	int dif = numAtk - this->numAttacked;
	if(dif >= 1) gameDisplay->callFunction("bird");
	numAttacked = numAtk;
}

void Interface::gameOver(void)
{
	int birdScore = numAttacked * -10;
	int totalScore = score - birdScore;

	int numStar = 0;

	if(totalScore > 1000) numStar = 3;
	else if(totalScore > 500) numStar = 2;
	else if(totalScore > 200)numStar = 1;

	gameDisplay->callFunction("gameOver", Hikari::Args(numStar));

	isGameOver = true;
}

/*void Interface::onGoal()
{
	controls->callFunction("Goal");
}
void Interface::onShot()
{
	controls->callFunction("Shot");
}
void Interface::throwMeat()
{
	menu->callFunction("throwMeat");
}
void Interface::getMeat()
{
	menu->callFunction("getMeat");
}
void Interface::addLife()
{
	menu->callFunction("addLife");
}
void Interface::getLife()
{
	menu->callFunction("getLife");
}*/

Hikari::FlashValue Interface::onExit( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	main->shutdown();
	return FLASH_VOID;
}

Hikari::FlashValue Interface::onOpen( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	return FLASH_VOID;
}

Hikari::FlashValue Interface::onPlay( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	startMenu->hide();
	startMenu->stop();
	gameDisplay->show();
	main->startGame();
	main->getTrayManager()->hideCursor();
	return FLASH_VOID;
}

