#include "CartMBC3.h"

CartMBC3::CartMBC3(std::string rom_file) : Cartridge(rom_file)
{
	__int64 t;
	rtc_latched = false;
	rtc_mapped = false;
	rtc_latch_buffer = 0xff;
	ms_last_update = clock();
	loadBanks(romBanks, rom_file);

	ramFile->read((char*)&seconds, 1);
	ramFile->read((char*)&minutes, 1);
	ramFile->read((char*)&hours, 1);
	ramFile->read((char*)&day_low, 1);
	ramFile->read((char*)&day_high, 1);

	ramFile->read((char*)&secondsLatch, 4);
	ramFile->read((char*)&minutesLatch, 4);
	ramFile->read((char*)&hoursLatch, 4);
	ramFile->read((char*)&dayLowLatch, 4);
	ramFile->read((char*)&dayHighLatch, 4);

	ramFile->read((char*)&t, 8);

}

void CartMBC3::write(word address, byte value)
{
	update_clock();
	int new_bank = 0;
	switch(address >> 12)
	{
	case 0:
	case 1:
		ramWriteEnable = ((value & 0x0a) == 0x0a) ? true : false;
		break;
	case 2:
	case 3:
		new_bank = value & 0x7f;
		if(new_bank == 0) new_bank = 1;
			currentRomBank = new_bank;
		break;
	case 4:
	case 5:
		if(value >= 0x8 && value <= 0xc)
		{
			rtc_mapped = true;
			rtc_reg = value;
		}
		else
		{
			currentRamBank = value & 0x3;
		}
		break;
	case 6:
	case 7:
		if(rtc_latch_buffer == 0 && value == 1)
		{
			rtc_latched = true;
			rtc_latch_buffer = 0xff;
		}
		else
		{
			rtc_latch_buffer = value;
		}
		break;
	case 10:
	case 11:
		ramBanks[(currentRamBank*ramBankSize)+(address-0xa000)] = value;
		
		//prevent writes to the disk if the ram is locked
		if(!ramLock)
		{
			//push to the disk only every X ram writes
			if(ramWriteCount == RAM_WRITE_THRESHOLD)
			{
				__int64 t;
				time(&t);
				ramFile->seekp(std::ios::beg);
				ramFile->write((char*)ramBanks, numRamBanks*ramBankSize);
				ramFile->write((const char*)&seconds, 4);
				ramFile->write((const char*)&minutes, 4);
				ramFile->write((const char*)&hours, 4);
				ramFile->write((const char*)&day_low, 4);
				ramFile->write((const char*)&day_high, 4);

				//change to latches for each counter
				ramFile->write((const char*)&secondsLatch, 4);
				ramFile->write((const char*)&minutesLatch, 4);
				ramFile->write((const char*)&hoursLatch, 4);
				ramFile->write((const char*)&dayLowLatch, 4);
				ramFile->write((const char*)&dayHighLatch, 4);

				ramFile->write((const char*)&t, 8);
				ramWriteCount = 0;
			}
			else
				ramWriteCount++;
		}
		break;
	}
}

byte CartMBC3::read(word address)
{
	update_clock();
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
		if(currentRomBank <= numRomBanks)
			return romBanks[(currentRomBank*ROM_BANK_SIZE) + (address - 0x4000)];
	case 10:
	case 11:
		if(currentRamBank <= numRamBanks)
		{
			if(rtc_mapped)
				return read_rtc(rtc_reg);
			return romBanks[(currentRamBank*ramBankSize) + (address - 0xa000)];
		}
	}

	return 0;
}

byte CartMBC3::read_rtc(byte rtc_reg)
{
	byte ret_val = 0;
	switch(rtc_reg)
	{
		case 0x8:
			ret_val = seconds;
			break;
		case 0x9:
			ret_val = minutes;
			break;
		case 0xa:
			ret_val = hours;
			break;
		case 0xb:
			ret_val = day_low;
			break;
		case 0xc:
			ret_val = day_high;
			break;
	}
	return ret_val;
}

void CartMBC3::update_clock()
{
	if(rtc_latched)
		return;

	int elapsed_sec = (clock() - ms_last_update) / 1000;
	if(elapsed_sec > 0)
	{
		ms_last_update = clock();

		seconds += elapsed_sec;
		if(seconds >= 60)
		{
			minutes += (seconds / 60);
			seconds -= 60;
			if(minutes >= 60)
			{
				hours += (minutes / 60);
				minutes -= 60;
				if(hours >= 24)
				{
					day_low += (hours / 24);
					hours -= 24;
					if(day_low == 0)
					{
						if((day_high & 1) == 0)
						{
							day_high += 1;
						}
						else
						{
							day_high += (1 << 7);
							day_high -= 1;
						}
					}
				}
			}
		}
	}
}