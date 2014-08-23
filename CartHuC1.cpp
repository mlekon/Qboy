#include "CartHuC1.h"

CartHuC1::CartHuC1(std::string rom_file) : Cartridge(rom_file)
{
}

void CartHuC1::write(word address, byte value)
{
	int new_bank = 0;
	switch(address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		new_bank = value & 0x1f;
		if(new_bank == 0) new_bank = 1;
			currentRomBank = new_bank;
		break;
	case 4:
	case 5:
		if(memoryMode == MODE_4_32)
			currentRamBank = value & 3;
		else
			currentRomBank = (currentRomBank & 0x1f) | ((0x03 & value) << 5);
		break;
	case 6:
	case 7:
		memoryMode = !memoryMode;
		break;
	case 10:
	case 11:
		ramBanks[ramOffset+(currentRamBank*ramBankSize)+(address-0xa000)] = value;
		break;
	};
}

byte CartHuC1::read(word address)
{
	switch(address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return romBanks[address];
	case 4:
	case 5:
	case 6:
	case 7:
		return romBanks[(currentRomBank*ROM_BANK_SIZE) + (address - 0x4000)];
	case 10:
	case 11:
		return romBanks[ramOffset + (currentRamBank*ramBankSize) + (address - 0xa000)];
	};

	return 0;
}
