#include "CartMBC5Rumble.h"

CartMBC5Rumble::CartMBC5Rumble(std::string rom_file) : CartMBC5(rom_file)
{
	currentRomBank_msb = 0;
	motor_on = false;
	loadBanks(romBanks, rom_file);
}


/////////////////////////////////
//writes a value to the MBC5 cartridge ram
//or changes how the cartridge data is accessed
void CartMBC5Rumble::write(word address, byte value)
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
		//set the ram bank and turn the
		//rumble motor on or off
		currentRamBank = value & 0x7;
		motor_on = (value & 0x8) != 0;
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