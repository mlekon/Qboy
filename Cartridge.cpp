#include "enum.h"
#include "Cartridge.h"
#include "CartMBC1.h"
#include "CartMBC2.h"
#include "CartMBC3.h"
#include "CartMBC5.h"
#include "CartMBC5Rumble.h"
#include "CartHuC1.h"
#include "CartRom.h"
#include "Gameboy.h"
#include <iostream>
#include <fstream>

using namespace std;

Cartridge::Cartridge()
{
}


Cartridge::Cartridge(std::string rom_file)
{
	ready = false;

	//get the name of the file minus the extension
	romName = rom_file;
	ramLock = false;
	ramWriteEnable = true;
	memoryMode = MODE_16_8;	

	romBanks = NULL;
	ramBanks = NULL;

	currentRomBank = 0;
	currentRamBank = 0;
	cartType = 0;
	numRomBanks = 2;
	numRamBanks = 0;
	ramWriteCount = 0;
	romSize = 0;
	ramSize = 0;

	//try to open the ram dump file, which is named the
	//same as the rom file with an extra '.sav' extension
	const char* rf = rom_file.append(".sav").c_str();
	ramFile = new fstream(rf, ios::binary | ios::in | ios::out);
	if(ramFile->fail())
	{
		//the ram file does exist; create and open it
		delete ramFile;
		ramFile = new fstream(rf, ios::binary | ios::out);
		ramFile->close();
		ramFile->open(rf, ios::in | ios::binary | ios::out);
	}
}


Cartridge::~Cartridge()
{
	//make sure the file is still open
	if(!ramFile->fail() && numRamBanks > 0)
	{
		ramFile->seekp(std::ios::beg);
		ramFile->write((char*)ramBanks, numRamBanks*ramBankSize);
		ramFile->close();
		delete ramFile;
	}

	delete[] romBanks;
	if(ramBanks != NULL)
		delete[] ramBanks;
}


/////////////////////////////////
//alloate memory for the ram and rom banks
void Cartridge::initMemory()
{
	romBanks = new byte[(ROM_BANK_SIZE*numRomBanks)];
	memset(romBanks, 0, (ROM_BANK_SIZE*numRomBanks));

	if((ramBankSize*numRamBanks) > 0)
	{
		ramBanks = new byte[(ramBankSize*numRamBanks)];
		memset(ramBanks, 0, (ramBankSize*numRamBanks));
	}
	else
		ramBanks = NULL;
}


/////////////////////////////////
//set pointers to the other gameboy systems
void Cartridge::setComponents(MemoryManager* m, Z80* z, Gameboy* gb)
{
	this->gb = gb;
	mem = m;
	z80 = z;
}


/////////////////////////////////
//generates the appropriate type of cartridge from the
//given file name
Cartridge* Cartridge::cartFactory(std::string rom_file, Gameboy* gb)
{
	//open the file and read the cartridge type byte
	std::ifstream is;
	is.open(rom_file.c_str(), ios::binary);
	is.seekg(0x0147);
	char b;
	is.read(&b,1);
	is.close();

	//create a new cartridge object
	switch(b)
	{
	case ROM:
	case ROM_RAM:
	case ROM_RAM_BATT:
	case ROM_MMMO1:
	case ROM_MMMO1_SRAM:
	case ROM_MMMO1_SRAM_BATT:
		return new CartRom(rom_file);
	case ROM_MBC1:
	case ROM_MBC1_RAM:
	case ROM_MBC1_RAM_BATT:
		return new CartMBC1(rom_file);
	case ROM_MBC2:
	case ROM_MBC2_BATT:
		return new CartMBC2(rom_file);
	case ROM_MBC3_RAM:
	case ROM_MBC3_RAM_BATT:
	case ROM_MBC3_TIMER_BATT:
	case ROM_MBC3_TIMER_RAM_BATT:
	case ROM_MBC3:
		return new CartMBC3(rom_file);
	case ROM_MBC5_RAM:
	case ROM_MBC5_RAM_BATT:
		return new CartMBC5(rom_file);
	case ROM_MBC5_RUMBLE:
	case ROM_MBC5_RUMBLE_SRAM:
	case ROM_MBC5_RUMBLE_SRAM_BATT:
		return new CartMBC5Rumble(rom_file);
	//case Hudson_HuC_3:
	//case Hudson_HuC_1:
	//	return new CartHuC1(rom_file);
	default:
		gb->addMessage(UNKNOWN_CARTRIDGE);
		return new CartRom(rom_file);
	
	}
}


/////////////////////////////////
//loads cartridge data other than game information from the rom file,
//such as the title and number of rom banks
void Cartridge::loadCart(string rom_file)
{
	//read the first bank (present in all cartridge types)
	char buffer[0x400];
	ifstream is;
	is.open(rom_file.c_str(), ios::binary);
	is.read(buffer, 0x400);
	is.close();

	//copy cartridge info such as the title
	memcpy(title, &buffer[0x134], 16);
	region = buffer[0x14a];
	cartType = buffer[0x147];
	licensee = (buffer[0x144] << 8) + buffer[0x145];
	maskVersionNum = buffer[0x14c];

	byte type = buffer[0x143];
	if(type == 0x80 || type == 0xc0)
		gbType = GBC;
	else
		gbType = GB;

	//set number of rom banks
	switch(buffer[0x148])
	{
	case 0:
		numRomBanks = 2;
		break;
	case 1:
		numRomBanks = 4;
		break;
	case 2:
		numRomBanks = 8;
		break;
	case 3:
		numRomBanks = 16;
		break;
	case 4:
		numRomBanks = 32;
		break;
	case 5:
		numRomBanks = 64;
		break;
	case 6:
		numRomBanks = 128;
		break;
	case 7:
		numRomBanks = 255;
		break;
	case 0x52:
		numRomBanks = 72;
		break;
	case 0x53:
		numRomBanks = 80;
		break;
	case 0x54:
		numRomBanks = 96;
		break;
	};

	//set number of ram banks
	switch(buffer[0x149])
	{
	case 0:
		numRamBanks = 0;
		break;
	case 1:
	case 2:
		numRamBanks = 1;
		break;
	case 3:
		numRamBanks = 4;
		break;
	case 4:
		numRamBanks = 16;
		break;
	default:
		numRamBanks = 0;
	}

	romSize = numRomBanks * ROM_BANK_SIZE;
	ramSize = numRamBanks * ramBankSize;
}


/////////////////////////////////
//loads data into the rom banks from the rom file
void Cartridge::loadBanks(byte* dest, std::string romName)
{
	loadCart(romName);
	initMemory();

	std::ifstream ifs;
	ifs.open(romName.c_str(), ios::binary);

	//get the file length
	ifs.seekg(0, ios::end);
	int length = ifs.tellg();
	ifs.seekg(0, ios::beg);
	ifs.read((char*)romBanks, length);
	ifs.close();

	//load data from the save file into the ram banks
	if(numRamBanks > 0)
	{
		ramFile->seekg(0, ios::end);
		length = ramFile->tellg();
		ramFile->seekg(0, ios::beg);

		if(length > 0 && length <= numRamBanks * ramBankSize)
			ramFile->read((char*)ramBanks, length);
	}
	else
	{
		ramBanks = NULL;
	}

	ready = true;
}


/////////////////////////////////
//dummy function
void Cartridge::write(word addr, byte val)
{
}


/////////////////////////////////
//dummy function
byte Cartridge::read(word addr)
{
	return 0;
}


/////////////////////////////////
//writes the status of the cartridge to the given state
void Cartridge::constructState(State* s)
{
	s->memoryMode = this->memoryMode;
	s->currentRamBank = this->currentRamBank;
	s->currentRomBank = this->currentRomBank;
	s->ramWriteCount = this->ramWriteCount;
}


/////////////////////////////////
//loads the cartridge attributes from the given state
void Cartridge::restoreState(State* s)
{
	this->memoryMode = s->memoryMode;
	this->currentRamBank = s->currentRamBank;
	this->currentRomBank = s->currentRomBank;
	this->ramWriteCount = s->ramWriteCount;
}


/////////////////////////////////
//prevent the ram from being accessed
//used when rewinding
void Cartridge::lockRam()
{
	ramLock = true;
}


/////////////////////////////////
//allows the ram to be accessed
//used when done rewinding
void Cartridge::unlockRam()
{
	ramLock = false;
}


/////////////////////////////////
//resets the component when loading
//a new ROM file
void Cartridge::reset()
{
	ramWriteCount = 0;
	ramLock = false;
	ramWriteEnable = true;
	memoryMode = MODE_16_8;
}


/////////////////////////////////
//raw read from the specified rom bank.
//addr being the offset within that bank
byte Cartridge::readFromROMBank(word bank, word addr)
{
	if(bank >= 0 && bank < numRomBanks && addr < ROM_BANK_SIZE)
		return romBanks[(bank*ROM_BANK_SIZE) + addr];
	else
		return 0;
}


/////////////////////////////////
//raw read from the specified ram bank.
//addr being the offset within that bank
byte Cartridge::readFromRAMBank(word bank, word addr)
{
	if(bank >= 0 && bank < numRamBanks && addr < ramBankSize)
		return ramBanks[(bank*ramBankSize) + addr];
	else
		return 0;
}


/////////////////////////////////
//get the ram bank mapped to the system address space
byte Cartridge::getCurrentRAMBank()
{
	return currentRamBank;
}


/////////////////////////////////
//get the rom bank mapped to the system address space
byte Cartridge::getCurrentROMBank()
{
	return currentRomBank;
}


/////////////////////////////////
//Gets info about the current rom
RomInfo Cartridge::getRomInfo()
{
	RomInfo ri;
	ri.cartType = cartType;
	ri.checksum = (checksum[0] << 8) + checksum[1];
	ri.licensee = licensee;
	ri.maskVersionNum = maskVersionNum;
	ri.numRamBanks = numRamBanks;
	ri.numRomBanks = numRomBanks;
	ri.region = region;
	ri.totalRamSize = ramSize;
	ri.totalRomSize = romSize;

	//strcpy_s(ri.romName, 16, (char*)&title); 
	return ri;
}

/////////////////////////////////
//sums all data in the cartridge
//to check against the value in low cart memory
word Cartridge::getChecksum()
{
	return 0;
}


/////////////////////////////////
//is the cartridge loaded and ready to be used?
bool Cartridge::isReady()
{
	return ready;
}


/////////////////////////////////
//replaces the opcode at the given address with the
//breakpoint code. the previous value is stored in the
//breakpoint structure along with the bank at the time
Breakpoint Cartridge::addBreakpoint(Breakpoint bp)
{
	switch(bp.address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		//bank 0 is always mapped to 0x0000 to 0x3ffff
		bp.opcode = romBanks[bp.address];
		romBanks[bp.address] = BREAKPOINT_CODE;
		bp.source = 0;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		//0x4000 to 0x7fff are mapped to bank currentRomBank 
		bp.opcode = romBanks[(currentRomBank*ROM_BANK_SIZE) + (bp.address - 0x4000)];
		romBanks[(currentRomBank*ROM_BANK_SIZE) + (bp.address - 0x4000)] = BREAKPOINT_CODE;
		bp.source = currentRomBank;
		break;
	case 10:
	case 11:
		//read from ram bank currentRamBank
		if(ramBanks != NULL)
		{
			bp.opcode = ramBanks[(currentRamBank*ramBankSize) + (bp.address - 0xa000)];
			ramBanks[(currentRamBank*ramBankSize) + (bp.address - 0xa000)] = BREAKPOINT_CODE;
			bp.source = currentRamBank;
			break;
		}
	};
	return bp;
}


/////////////////////////////////
//restores the original opcode to the address
//that was set as a breakpoint
void Cartridge::removeBreakpoint(Breakpoint bp)
{
	switch(bp.address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		//bank 0 is always mapped to 0x0000 to 0x3ffff
		romBanks[bp.address] = bp.opcode;
		return;
	case 4:
	case 5:
	case 6:
	case 7:
		//0x4000 to 0x7fff are mapped to bank currentRomBank 
		romBanks[(bp.source*ROM_BANK_SIZE) + (bp.address - 0x4000)] = bp.opcode;
		return;
	case 10:
	case 11:
		//read from ram bank currentRamBank
		if(ramBanks != NULL)
		{
			ramBanks[(bp.source*ramBankSize) + (bp.address - 0xa000)] = bp.opcode;
		}
	};
}


/////////////////////////////////
//returns the type of gameboy hardware the cartridge needs
GameboyType Cartridge::getGameboyType()
{
	return gbType;
}