#ifndef MBC1_H
#define MBC1_H

#include "Cartridge.h"

class CartMBC1 : public Cartridge
{
public:
	CartMBC1(std::string rom_file);

	void write(word addr, byte value);
	byte read(word addr);
};

#endif //MBC1_H
