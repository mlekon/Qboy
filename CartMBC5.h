#ifndef MBC5_H
#define MBC5_H

#include "Cartridge.h"

class CartMBC5 : public Cartridge
{
protected:
	byte currentRomBank_msb;			//bit 8 of the rom bank
	word _currentRomBank;				//bits 0-7 of the rom bank
public:
	CartMBC5(std::string rom_file);

	void write(word addr, byte value);
	byte read(word addr);
};

#endif //MBC5_H