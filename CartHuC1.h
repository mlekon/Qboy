#ifndef HuC1_H
#define HuC1_H

#include "Cartridge.h"

class CartHuC1 : public Cartridge
{
public:
	CartHuC1(std::string rom_file);

	void write(word addr, byte value);
	byte read(word addr);
};

#endif //HuC1_H
