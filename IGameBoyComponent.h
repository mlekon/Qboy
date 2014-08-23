#ifndef IGB_COMP_H
#define IGB_COMP_H

#include <string>

class Gameboy;
class State;

class IGameBoyComponent
{
protected:
	Gameboy* gb;
public:
	virtual void constructState(State*) = 0;
	virtual void restoreState(State*) = 0;
	virtual void reset()=0;

};

#endif