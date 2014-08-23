#ifndef TIMER_H
#define TIMER_H

#include "enum.h"
#include "Z80.h"
#include "MemoryManager.h"
#include "allegro.h"
#include "IGameBoyComponent.h"

class Timer : IGameBoyComponent
{
private:
	float divRegister;					//emulator floating point value of div
	float timaRegister;					//emulator floating point value of tima
	byte timerThreshold;			//clocks elapsed since the last div/tima update

	MemoryManager* mem;				//pointer to the memory component
	Z80* z80;						//pointer to the cpu component
public:
	static const float msPerClock;
	static const float clocksPerSec;
	static const float divTicksPerClock;
	static const float timerTicksPerClock[];

	Timer();
	~Timer();
	void timer(byte);
	void setDiv(byte);
	void setTimer(byte);
	void setComponents(MemoryManager*, Z80*, Gameboy*);
	void advanceTimer(byte);
	void advanceDiv(byte);

	void constructState(State*);
	void restoreState(State*);
	void reset();
};


#endif	//TIMER_H