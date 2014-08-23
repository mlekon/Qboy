#include "CartMBC2.h"

CartMBC2::CartMBC2(std::string rom_file) : Cartridge(rom_file)
{
	loadCart(rom_file);
	numRamBanks = 1;
	initMemory();
	std::ifstream ifs;
	ifs.open(romName.c_str(), std::ios::binary);
	ifs.seekg(0, std::ios::end);
	int length = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	ifs.read((char*)romBanks, length);
	ifs.close();

	//char* file_type = ".sav";
	if(numRamBanks > 0)
	{
		ramFile->seekg(0, std::ios::end);
		length = ramFile->tellg();
		ramFile->seekg(0, std::ios::beg);
		ramOffset = numRomBanks * ROM_BANK_SIZE;
		if(length > 0 && length <= numRamBanks * ramBankSize)
			ramFile->read((char*)ramBanks, length);
	}
}


/////////////////////////////////
//writes a value to the MBC2 cartridge ram
//or changes how the cartridge data is accessed
void CartMBC2::write(word address, byte value)
{
	int new_bank;

	switch(address >> 12)
	{
	case 0:
	case 1:
		//enable or disable ram writing
		ramWriteEnable = (!((address >> 8) & 1)) ? !ramWriteEnable : ramWriteEnable;
		break;
	case 2:
	case 3:
		//change the ram bank mapped to 0x4000 to 0x7fff
		new_bank = value & 0xf;
		if(new_bank == 0) new_bank = 1;
		currentRomBank = ((address >> 8) & 1) ? new_bank : currentRomBank;
		break;
	case 10:
		//write 4 bits to the cartridge ram
		if(address <= 0xa1ff)
			ramBanks[ramOffset+(address-0xa000)] = value & 0xf;
	
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
}


/////////////////////////////////
//returns the value at the specified address
//of the cartridge in the current bank
byte CartMBC2::read(word address)
{
	switch(address >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		//rom bank 0 is always mapped to 0x0000 to 0x4000
		return romBanks[address];
	case 4:
	case 5:
	case 6:
	case 7:
		//read from currentRomBank
		return romBanks[(currentRomBank*ROM_BANK_SIZE) + (address - 0x4000)];
	case 10:
		//MBC2 ram uses half-byte (4-bit) values
		if(address <= 0xa1ff)
			return ramBanks[(currentRamBank*ramBankSize) + (address - 0xa000)] & 0xf;
	}

	return 0;
}
