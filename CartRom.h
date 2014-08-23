#ifndef ROM_H
#define ROM_H

#include "Cartridge.h"

class CartRom : public Cartridge
{
private:
	byte* rom;
public:
	CartRom(std::string rom_file);

	void write(word addr, byte value);
	byte read(word addr);
};
#endif //ROM_H