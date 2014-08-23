#include "CartMBC5.h"

CartMBC5::CartMBC5(std::string rom_file) : Cartridge(rom_file)
{
	currentRomBank_msb = 0;
	loadBanks(romBanks, rom_file);
}


/////////////////////////////////
//writes a value to the MBC5 cartridge ram
//or changes how the cartridge data is accessed
void CartMBC5::write(word address, byte value)
{
	switch(address >> 12)
	{
	case 2:
		//set bits 0-7 of the rom bank
		currentRomBank = (currentRomBank & 0x100) + value;
		currentRomBank %= numRomBanks;
		break;
	case 3:
		//set bit 8 of the rom bank
		currentRomBank |= ((value & 1) << 7);
		currentRomBank %= numRomBanks;
		break;
	case 4:
	case 5:
		//set the ram bank
		currentRamBank = value & 0xf;
		break;
	case 10:
	case 11:
		ramBanks[(currentRamBank*ramBankSize)+(address-0xa000)] = value;
		
		//prevent writes to the disk if the ram is locked
		if(!ramLock)
		{
			//push to the disk only every X ram writes
			if(ramWriteCount == RAM_WRITE_THRESHOLD)
			{
				ramFile->seekp(std::ios::beg);
				ramFile->write((char*)ramBanks, numRamBanks*ramBankSize);
				ramWriteCount = 0;
			}
			else
				ramWriteCount++;
		}
		break;
	}
}


/////////////////////////////////
//returns the value at the specified address
//of the cartridge in the current bank
byte CartMBC5::read(word address)
{
	switch(address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		//read from bank 0
		return romBanks[address];
	case 4:
	case 5:
	case 6:
	case 7:
		//read from currentRomBank
		return romBanks[(currentRomBank*ROM_BANK_SIZE) + (address - 0x4000)];
	case 10:
	case 11:
	case 12:
		//read from currentRamBank
		return ramBanks[(currentRamBank*ramBankSize) + (address - 0xa000)];
	default:
		return 0;
	}
}
