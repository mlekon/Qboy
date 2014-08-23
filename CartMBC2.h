#ifndef MBC2_H
#define MBC2_H

#include "Cartridge.h"

class CartMBC2 : public Cartridge
{
public:
	CartMBC2(std::string rom_file);

	void write(word addr, byte value);
	byte read(word addr);
};

#endif //MBC2_H