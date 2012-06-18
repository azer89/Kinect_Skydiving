//FSM.h
#ifndef FSM_H
#define FSM_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

class FSM;

//An individual state (must belong to a FSM)
//This is an abstract class and must be sub-classed
class FSMState
{
public:
  FSMState(){};
  FSMState(FSM *fsm){};
  virtual void Update(const float& dt) = 0;
  virtual void DoENTER(){};
  virtual void DoEXIT(){};

  std::string stateName;  //used to switch between states
  FSM *fsm;
};

//A vector of shared pointers housing all the states in the machine
typedef std::vector< std::tr1::shared_ptr<FSMState> > StateBank;

//---------------------------------------
//A Simple Finite State Machine
class FSM
{
public:
  FSM();
  ~FSM();
  void Update(const float& dt);

  void transitionTo(std::string stateName);
  void DelayTransitionTo(std::string stateName);
  void AddState(FSMState *newState, bool makeCurrent);
  std::string GetState();

public:
  FSMState *currentState;
  std::string delayState;

   //Bank to house list of states
  StateBank stateBank;
  std::vector< std::tr1::shared_ptr<FSMState> >::iterator iter;
};

#endif