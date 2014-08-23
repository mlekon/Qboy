#ifndef GBC_MEM_H
#define GBC_MEM_H

#include "MemoryManager.h"
#include "GBCLCD.h"

class GBCLCD;

class GBCMemoryManager : public MemoryManager
{
private:
	GBCLCD* lcd;

	bool paletteAccess;
	byte currentWorkRamBank;
	byte currentVRamBank;

	byte vramBanks[2][0x2000];
	byte workRamBanks[8][0x1000];
	byte paletteRam[0x40];
public:
	GBCMemoryManager(void);
	~GBCMemoryManager(void);

	void specialRegisters(word addr, byte val);
	void constructState(State* s);
	void restoreState(State* s);
	byte read(word addr);
	byte readRam(word addr);
	void writeRam(word addr, byte val);
	void write(word addr, byte val);
	byte readVRam(word addr, byte bank);
	byte readPaletteRam(byte offset);
	void setPaletteAccess(bool b);
	void setLCD(GBCLCD* lcd);
};

#endif //GBC_MEM_H