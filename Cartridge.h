#ifndef CART_H
#define CART_H

#include "enum.h"
#include "MemoryManager.h"
#include "Z80.h"
#include <iostream>
#include <fstream>
#include <string>
#include "IGameBoyComponent.h"

enum CartridgeType
{
	/* cartridge types */
	ROM = 0,
	ROM_MBC1 = 1,
	ROM_MBC1_RAM = 2,
	ROM_MBC1_RAM_BATT = 3,
	ROM_MBC2 = 5,
	ROM_MBC2_BATT = 6,
	ROM_RAM = 8,
	ROM_RAM_BATT = 9,
	ROM_MMMO1 = 0xb,
	ROM_MMMO1_SRAM = 0xc,
	ROM_MMMO1_SRAM_BATT = 0xd,
	ROM_MBC3_TIMER_BATT = 0xf,
	ROM_MBC3_TIMER_RAM_BATT = 0x10,
	ROM_MBC3 = 0x11,
	ROM_MBC3_RAM = 0x12,
	ROM_MBC3_RAM_BATT = 0x13,
	ROM_MBC5 = 0x19,
	ROM_MBC5_RAM = 0x1a,
	ROM_MBC5_RAM_BATT = 0x1b,
	ROM_MBC5_RUMBLE = 0x1c,
	ROM_MBC5_RUMBLE_SRAM = 0x1d,
	ROM_MBC5_RUMBLE_SRAM_BATT = 0x1e,
	Pocket_Camera = 0x1f,
	Bandai_TAMA5 = 0xfd,
	Hudson_HuC_3 = 0xfe,
	Hudson_HuC_1 = 0xff
};

enum GameboyType
{
	GBC,
	GB
};


class Cartridge : public IGameBoyComponent
{
protected:
	static const word ramBankSize = 0x4000;				//bytes in a single memory bank

	static const int MODE_16_8 = 0;							//16 Mbit/8 Kbit MBC memory mode
	static const int MODE_4_32 = 1;							//4 Mbit/32 Kbit MBC memory mode
	static const int RAM_WRITE_THRESHOLD = 256;				//number of ram writes to perform before writing them
															//them to the save file on the disk

	bool ramLock;											//emulator mechanism: when true, prevents ram writing
	bool ramWriteEnable;									//cartridge mechanism: when true, allows ram writing
	bool memoryMode;										//indicates the memory mode MBC cartridges
	bool ready;

	unsigned int romSize;									//cartirdge rom size in bytes
	unsigned int ramSize;									//cartridge ram size in bytes

	int ramWriteCount;									//number of ram writes since last disk write
	int ramOffset;

	GameboyType gbType;

	MemoryManager* mem;										//memory component
	Z80* z80;												//cpu component
	
	word currentRomBank;									//rom bank to use in read()
	word numRomBanks;										//highest rom bank
	byte currentRamBank;									//ram bank to use in read()
	byte numRamBanks;										//highest ram bank
	byte cartType;											//code for the cartridge type
	byte region;											//cartridge region code
	byte licensee;											//cartridge licensee name
	byte maskVersionNum;
	byte title[16];											//game title
	byte checksum[2];										//cartridge data checksum

	byte* ramBanks;										//ram banks are contiguous within the array
	byte* romBanks;										//rom banks are contiguous within the array

	std::fstream* ramFile;									//stream to the file containing rom data
public:
	static Cartridge* cartFactory(std::string, Gameboy*);
	static void loadBanks(byte**, std::string);
	std::string romName;									//name of the file containing rom data

	Cartridge();
	Cartridge(std::string rom_file);
	virtual ~Cartridge();

	void setComponents(MemoryManager*, Z80*, Gameboy*);
	void initMemory();
	void loadCart(std::string);
	void lockRam();
	void unlockRam();
	void loadBanks(byte*, std::string);
	byte getCurrentROMBank();
	byte getCurrentRAMBank();
	word getChecksum();
	Breakpoint addBreakpoint(Breakpoint bp);
	void removeBreakpoint(Breakpoint bp);

	virtual void write(word addr, byte b);
	virtual byte read(word addr);
	byte readFromROMBank(word bank, word addr);
	byte readFromRAMBank(word bank, word addr);
	bool isReady();
	RomInfo getRomInfo();
	GameboyType getGameboyType();

	void constructState(State*);
	void restoreState(State*);
	void reset();
};

#endif //CART_H
