#include "CartMBC1.h"
#include <assert.h>

CartMBC1::CartMBC1(std::string rom_file) : Cartridge(rom_file)
{
	loadBanks(romBanks, rom_file);
	currentRomBank = 1;
}


/////////////////////////////////
//writes a value to the MBC1 cartridge ram
//or changes how the cartridge data is accessed
void CartMBC1::write(word address, byte value)
{
	int new_bank = 0;

	//only the last hex digit is needed to 
	//determine which action to take
	switch(address >> 12)
	{
	case 0:
	case 1:
		//disable/enable ram writing
		ramWriteEnable = ((value & 0x0a) == 0x0a) ? false : true;
		break;
	case 2:
	case 3:
		//change the lower 5 bits of the current rom bank
		new_bank = value & 0x1f;
		if(new_bank == 0 || new_bank == 0x20 ||new_bank == 0x40 || new_bank == 0x60) 
			new_bank += 1;
		currentRomBank = (currentRomBank & 0x60) + new_bank;

		//bank 0 cannot be mapped to 0x4000-0x7fff
		if(currentRomBank == 0)
			currentRomBank = 1;

		if(currentRomBank >= numRomBanks)
			currentRomBank = numRomBanks;
		currentRomBank %= numRomBanks;
		break;
	case 4:
	case 5:
		if(memoryMode == MODE_4_32)
			//change the ram bank
			currentRamBank = value & 3;
		else
		{
			//change the upper 2 bits of the current rom bank
			new_bank = ((value << 5) & 0x60);
			if(new_bank == 0 || new_bank == 0x20 ||new_bank == 0x40 || new_bank == 0x60) 
				new_bank += 1;
			currentRomBank = (new_bank | (currentRomBank & 0x1f));
		}

		if(currentRomBank >= numRomBanks)
			currentRomBank = numRomBanks;
		currentRomBank %= numRomBanks;
		break;
	case 6:
	case 7:
		//change the memory mode
		memoryMode = value & 1;
		break;
	case 10:
	case 11:
		//write to the cartridge ram
		if(numRamBanks > 0 && currentRamBank <= numRamBanks)
		{
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
		}
		break;
	}
}


/////////////////////////////////
//returns the value at the specified address
//of the cartridge in the current bank
byte CartMBC1::read(word address)
{
	switch(address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		//bank 0 is always mapped to 0x0000 to 0x3ffff
		return romBanks[address];
	case 4:
	case 5:
	case 6:
	case 7:
		//0x4000 to 0x7fff are mapped to bank currentRomBank 
		return romBanks[(currentRomBank*ROM_BANK_SIZE) + (address - 0x4000)];
	case 10:
	case 11:
		//read from ram bank currentRamBank
		if(ramBanks != NULL)
			return ramBanks[(currentRamBank*ramBankSize) + (address - 0xa000)];
	default:
		return 0;
	}
}