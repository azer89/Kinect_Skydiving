//FSM.cpp

#include "Stdafx.h"
#include "FSM.h"

//Constructor
FSM::FSM()
{
  currentState = NULL;
  delayState = "NONE";
}

//Destructor
FSM::~FSM()
{
  stateBank.clear();
}

//Update each tick
void FSM::Update(const float& dt)
{
  //Make sure a current state is loaded
  if (currentState == NULL) return;

  //Check if there are any delayed transition requests
  if (delayState != "NONE")
  {
    TransitionTo(delayState);
    delayState = "NONE";
  }

  //Update the current state, may trigger a transition.
  currentState->Update(dt);
}

//Called to transition to another state
//@param stateName the name of the state to transition to
void FSM::TransitionTo(std::string stateName)
{
  if (stateName == currentState->stateName)
	  return;

  //Find the named state
  FSMState *goToState = NULL;
  for(iter= stateBank.begin(); iter!= stateBank.end(); iter++)
    if ( (*iter)->stateName == stateName )
      goToState = iter->get();

  //Error, trying to transition to a non-existant state
  if (goToState == NULL)
  {
    //Print an error here, or assert if you want
    return;
  }

  currentState->DoEXIT();
  goToState->DoENTER();
  currentState = goToState;
}

//Transition to another state (delayed until beginning of next update)
//@param stateName the name of the state to transition to
void FSM::DelayTransitionTo(std::string stateName)
{
  delayState = stateName;
}

//Add a state to the bank, optionally make it the current state
//@param newState the new state to add to the state machine
//@param makeCurrent is this new state the current state?
void FSM::AddState(FSMState *newState, bool makeCurrent)
{
  //Add this state to the FSM
  std::tr1::shared_ptr<FSMState> newStatePtr(newState);
  stateBank.push_back(newStatePtr);
  //Make this the current state?
  if (makeCurrent)
  {
	  currentState = newState;
	  currentState->DoENTER();
  }
}

//What is the name of the current state?
std::string FSM::GetState()
{
  return currentState->stateName;
}