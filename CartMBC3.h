#ifndef MBC3_H
#define MBC3_H

#include "Cartridge.h"
#include <ctime>

class CartMBC3 : public Cartridge
{
private:
	bool rtc_mapped;
	bool rtc_latched;
	byte rtc_latch_buffer;
	byte rtc_reg;
	byte seconds;
	byte minutes;
	byte hours;
	byte day_low;
	byte day_high;
	byte secondsLatch;
	byte minutesLatch;
	byte hoursLatch;
	byte dayLowLatch;
	byte dayHighLatch;
	int ms_last_update;
public:
	CartMBC3(std::string rom_file);

	void write(word addr, byte value);
	byte read(word addr);
	
	byte read_rtc(byte);
	void update_clock();
};

#endif //MBC3_H