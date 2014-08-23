#include "CartRom.h"

CartRom::CartRom(std::string rom_file) : Cartridge(rom_file)
{
	loadBanks(romBanks, rom_file);
}


/////////////////////////////////
//writing to a ROM-only cartridge has no effect
void CartRom::write(word addr, byte value)
{
}


/////////////////////////////////
//only addresses 0x0000 to 0x8000 are available
byte CartRom::read(word addr)
{
	if(addr <= 0x7fff)
		return romBanks[addr];
	else
		return 0;
}