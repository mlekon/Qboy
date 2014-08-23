#ifndef STATE_H
#define STATE_H

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include "Movie.h"

struct StateComponent
{
};

typedef unsigned char byte;
typedef unsigned short word;

class State
{
public:
	byte upperRam[0x4000];
	byte videoRam[0x2000];
	byte registers[10];
	byte ZF;							//flags from the F register
	byte NF;
	byte HF;
	byte CF;
	word programCounter;
	bool cpuHalted;
	bool interruptMasterEnable;
	bool oamAccess;						//is oam ram accessible?
	bool vramAccess;					//is vram accessible?
	bool memoryMode;					//memory mode of MBC1 carts
	int lcdHorizontalPhase;				//clocks since last scanline was drawn
	int lcdMode;						//mode the display was in
	int currentRamBank;
	int currentRomBank;
	int ramWriteCount;					//ram writes before the write-to-disk threshhold
	int frame;
	double clocks;						//total number of cpu clocks

	//Movie mov;

	State();
	State(bool);
	bool save_to_disk();
	bool save_to_disk(std::string, int);
	bool save_to_disk(int);
	bool load_from_disk(std::string, int);
};

#endif		//STATE_H