#ifndef MEM_MAN_H
#define MEM_MAN_H

class Z80;
class Cartridge;
class Timer;

#include "enum.h"
#include "Z80.h"
#include "Cartridge.h"
#include "Timer.h"
#include "Audio.h"
#include "IGameBoyComponent.h"
#include "BreakpointMap.h"

enum Interrupt;

enum MemoryRegister
{
	/* special registers */
	DIV = 0xff04,
	TIMA = 0xff05,
	TMA = 0xff06,
	TAC = 0xff07,
	NR10 = 0xff10,
	NR11 = 0xff11,
	NR12 = 0xff12,
	NR13 = 0xff13,
	NR14 = 0xff14,
	NR21 = 0xff16,
	NR22 = 0xff17,
	NR23 = 0xff18,
	NR24 = 0xff19,
	NR30 = 0xff1a,
	NR31 = 0xff1b,
	NR32 = 0xff1c,
	NR33 = 0xff1d,
	NR34 = 0xff1e,
	NR41 = 0xff20,
	NR42 = 0xff21,
	NR43 = 0xff22,
	NR44 = 0xff23,
	NR50 = 0xff24,
	NR51 = 0xff25,
	NR52 = 0xff26,
	LCDC = 0xff40,
	STAT = 0xff41,
	SCY = 0xff42,
	SCX = 0xff43,
	LY = 0xff44,
	LYC = 0xff45,
	BGP = 0xff47,
	OBP0 = 0xff48,
	OBP1 = 0xff49,
	WY = 0xff4a,
	WX = 0xff4b,
	VBK = 0xff4f,
	HDMA1 = 0xff51,
	HDMA2 = 0xff52,
	HDMA3 = 0xff53,
	HDMA4 = 0xff54,
	HDMA5 = 0xff55,
	BCPS = 0xff68,
	BCPD = 0xff69,
	OCPS = 0xff6a,
	OCPD = 0xff6b,
	SVBK = 0xff70,
	IE = 0xffff
};


class Audio;

class MemoryManager : IGameBoyComponent
{
protected:
	byte mem[0x10000];						//the entire gameboy address space

	byte port10;							//value of input port 10
	byte port20;							//value of input port 20

	bool vramAccess;						//is the video ram being accessed?
	bool oamAccess;							//is the OAM being accessed?
	bool inputReadThisFrame;

	Z80* z80;								//cpu component
	Cartridge* cart;						//cartridge component
	Timer* time;							//timer component
	Audio* audio;
public:
	MemoryManager();
	MemoryManager(Z80*);

	virtual void specialRegisters(word addr, byte val);
	byte* getMemArrayAddress(word gbAddr);

	virtual byte read(word address);
	virtual byte readRam(word);
	virtual void write(word address, byte value);
	virtual void writeRam(word, byte);
	void addBreakpoint(Breakpoint bp);
	Breakpoint removeBreakpoint(word address);

	void setInput10(byte);
	void setInput20(byte);
	void setInputPorts(byte, byte);

	void setCPU(Z80*);
	void setCart(Cartridge*);
	void setComponents(Z80*, Cartridge*, Timer*, Audio*, Gameboy*);
	void setVramAccess(bool b);
	void setOamAccess(bool b);
	void requestInterrupt(Interrupt b);
	void initMem();

	void pushStack8(byte);
	void pushStack16(byte h, byte l);
	void pushStack16(word);
	byte popStack8();
	word popStack16();

	void constructState(State*);
	void restoreState(State*);
	bool lagFrame();
	void reset();
};

#endif //MEM_MAN_H