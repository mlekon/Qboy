#include "Timer.h"

const float Timer::msPerClock = (1 / 4194.304);
const float Timer::clocksPerSec = 4194304;
const float Timer::divTicksPerClock = (16384 / clocksPerSec);
const float Timer::timerTicksPerClock[] = 
{
	4096   / clocksPerSec,
	262144 / clocksPerSec,
	65536  / clocksPerSec,
	16384  / clocksPerSec
};


Timer::Timer()
{
	timerThreshold =	0;
	divRegister =		0;
	timaRegister =		0;
}


Timer::~Timer()
{
}


/////////////////////////////////
//sets the necessary links to the components
//the timer needs to communicate with
void Timer::setComponents(MemoryManager* m, Z80* z, Gameboy* gb)
{
	this->gb = gb;
	mem = m;
	z80 = z;
}


/////////////////////////////////
//advance the timer by the given number of clocks
void Timer::timer(byte num_clocks)
{
	timerThreshold += num_clocks;

	//timer and div do not need to be updated more than
	//once every 128 cycles
	if(timerThreshold >= 128)
	{
		advanceDiv(timerThreshold);
		advanceTimer(timerThreshold);
		timerThreshold -= 128;
	}
}


void Timer::setDiv(byte b)
{
	divRegister = b;
}


void Timer::setTimer(byte b)
{
	timaRegister = b;
}


/////////////////////////////////
//advance the div register by num_clocks
//and update its value in gameboy memory
void Timer::advanceDiv(byte num_clocks)
{
	divRegister += (divTicksPerClock * num_clocks);
	if(divRegister > 0xff)
	{
		divRegister -= 0xff;
	}
	mem->writeRam(DIV, (byte)divRegister);
}


/////////////////////////////////
//advance the timer by num_clocks and updates the tima 
//register based on the timer rate, requests interrupt
//on overflow, and updates value in gameboy memory
void Timer::advanceTimer(byte num_clocks)
{
	byte tac = mem->readRam(TAC);

	//bit 2 is a flag indicating whether the timer is active
	if(tac & 0x4)
	{
		//get the timer speed
		byte timer_frequency_index = tac & 0x3;
		timaRegister += (timerTicksPerClock[timer_frequency_index] * num_clocks);
		if(timaRegister > 0xff)
		{
			//overflow. request interrupt
			timaRegister -= 0xff;
			mem->requestInterrupt((Interrupt)(mem->readRam(0xff0f) & 4));
			mem->writeRam(TIMA, mem->readRam(TMA));
		}
	}
}


void Timer::constructState(State* s)
{
}


void Timer::restoreState(State* s)
{
}


void Timer::reset()
{
	timerThreshold =	0;
	divRegister =		0;
	timaRegister =		0;
}