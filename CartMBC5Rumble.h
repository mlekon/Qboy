#ifndef RUMBLE_H
#define RUMBLE_H

#include "CartMBC5.h"

class CartMBC5Rumble : public CartMBC5
{
private:
	bool motor_on;

public:
	CartMBC5Rumble(std::string rom_file);

	void write(word addr, byte value);
};

#endif //RUMBLE_H