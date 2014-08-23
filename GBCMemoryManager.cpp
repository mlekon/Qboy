#include "GBCMemoryManager.h"


GBCMemoryManager::GBCMemoryManager(void) : MemoryManager()
{
	currentVRamBank = 0;
	currentWorkRamBank = 1;
	memset(paletteRam, 0xff, 0x40);
	memset(vramBanks, 0, 2 * 0x2000);
	memset(workRamBanks, 0, 8 * 0x1000);
}


GBCMemoryManager::~GBCMemoryManager(void)
{
}


void GBCMemoryManager::setLCD(GBCLCD* lcd)
{
	this->lcd = lcd;
}


void GBCMemoryManager::setPaletteAccess(bool b)
{
	paletteAccess = b;
}


/////////////////////////////////
//writes to a special register in the high memory
//some special registers have special requirements
//which are taken care of here
void GBCMemoryManager::specialRegisters(word addr, byte val)
{
	word oamStartAddr = val << 8;
	switch(addr)
	{
	case 0xff00:
		//set input for to read from
		mem[addr] = val | 0xf;
		return;
	case DIV:
		//reset div register
		val = 0;
		time->setDiv(0);
	case 0xff0f:
		//manual interrupt request
		mem[addr] = val;
		z80->interrupts();
		return;
	case SCY:
	case SCX:
		break;
	case LY:
		//read only
		return;
	case 0xff46:		
		//direct memory access: copy OAM
		if(oamAccess)
		{
			for(int i = 0; i < 40*4; i++)
				write(0xfe00+i, read(oamStartAddr+i));
		}
		return;
	case VBK:
		currentVRamBank = val & 1;
		break;
	case HDMA5:
		if(val & 0x80)
		{
			word src = ((mem[HDMA1] << 8) | mem[HDMA2]) & 0xfff0;
			word dest = (mem[HDMA3] << 8) | mem[HDMA4];

			if(dest >= 0x8000 && dest <= 0x9ff0 && (src <= 0x7ff0 || (src>= 0xa000 && src <= 0xdff0)))
			{
				for(int i = 0; i < ((val & 0x7f) + 1) * 0x10; i++)
				{
					vramBanks[currentVRamBank][dest - 0x8000] = cart->read(src);
					dest++; src++;
				}
			}
			mem[HDMA5] = 0xff;
		}
		else
		{
			int test = 0;
		}
		break;
	case BCPS:
		break;
	case BCPD:
		if(paletteAccess)
		{
			paletteRam[mem[BCPS] & 0x3f] = val;
			if(mem[BCPS] & 0x80)
			{
				mem[BCPS] = (mem[BCPS] + 1) & 0x3f;
			}
		}
		return;
	case OCPD:
		if(paletteAccess)
		{
			paletteRam[mem[OCPS] & 0x3f] = val;
			if(mem[OCPS] & 0x80)
			{
				mem[OCPS] = (mem[OCPS] + 1) & 0x3f;
			}
		}
		return;
	case SVBK:
		currentWorkRamBank = (val & 7) ? val & 7 : 1;
		break;
	default:;
#ifdef AUDIO_ENABLE
		if(addr <= 0xff3f && addr >= 0xff10)
			val = audio->setSoundRegister(addr, val);
#endif
	}
	mem[addr] = val;
	return;
}


/////////////////////////////////
//copies the non-cartridge memory to the given state
void GBCMemoryManager::constructState(State* s)
{
	memcpy(s->videoRam, &mem[0x8000], VIDEO_RAM_SIZE);
	memcpy(s->upperRam, &mem[0xc000], UPPER_RAM_SIZE);
	s->vramAccess = this->vramAccess;
	s->oamAccess = this->oamAccess;
}


/////////////////////////////////
//restores the non-cartridge memory from the given state
void GBCMemoryManager::restoreState(State* s)
{
	memcpy(&mem[0x8000], s->videoRam, VIDEO_RAM_SIZE);
	memcpy(&mem[0xc000], s->upperRam, UPPER_RAM_SIZE);
	this->vramAccess = s->vramAccess;
	this->oamAccess = s->oamAccess;
}


/////////////////////////////////
//read the byte at the given memory address
byte GBCMemoryManager::read(word addr)
{
	byte val = 0;

	//use only the three highest bits
	switch(addr >> 13)
	{
	case 0: 
	case 1: 
	case 2: 
	case 3:
	case 5:
		//rom or ram mapped address
		val = cart->read(addr);
		break;
	case 8:
	case 9:
		val = vramBanks[currentVRamBank][addr - 0x8000];
		break;
	case 12:
		val = workRamBanks[0][addr - 0xc000];
		break;
	case 13:
		val = workRamBanks[currentWorkRamBank][addr - 0xd000];
		break;
	case 15:
		if(addr == BCPD)
		{
			if(paletteAccess)
			{
				val = paletteRam[mem[BCPS] & 0x3f];
			}
		}
		else if(addr == OCPD)
		{
			if(paletteAccess)
			{
				val = paletteRam[mem[OCPS] & 0x3f];
			}
		}
		break;
	default:
		//return the specified input port value
		if(addr == 0xff00)
		{
			inputReadThisFrame = true;
			val = (mem[addr] & 0x20) ? (0x20 + (port20 & 0xf)) : (0x10 + (port10 & 0xf));
		}
		else
		{
			val = mem[addr];
		}
	};

	return val;
}


/////////////////////////////////
//slightly faster version of read()
//used only when the address is constant or
//will never be in rom or ram mapped space
byte GBCMemoryManager::readRam(word addr)
{
	if(addr >= 0x8000 && addr <= 0x9fff)
		return vramBanks[currentVRamBank][addr - 0x8000];
	else if(addr >= 0xc000 && addr <= 0xcfff)
		return workRamBanks[0][addr - 0xc000];
	else if(addr >= 0xd000 && addr <= 0xdfff)
		return workRamBanks[currentWorkRamBank][addr - 0xd000];
	return mem[addr];
}


/////////////////////////////////
//slightly faster version of write()
//used only when writing to the gameboy's internal ram
void GBCMemoryManager::writeRam(word addr, byte val)
{
	if(addr >= 0xe000 && addr <= 0xfe00)
		mem[addr - 0x2000] = val;
	else if(addr >= 0xc000 && addr <= 0xcfff)
		workRamBanks[0][addr - 0xc000] = val;
	else if(addr >= 0xd000 && addr <= 0xdfff)
		workRamBanks[currentWorkRamBank][addr - 0xd000] = val;
	else if(addr >= 0x8000 && addr <= 0x9fff)
		vramBanks[currentVRamBank][addr - 0x8000] = val;
	else
		mem[addr] = val;
}


/////////////////////////////////
//writes an 8 bit value to the given address
//of the memory. uses the cartridge for addresses
//that fall within the cartridge-mapped memory
void GBCMemoryManager::write(word addr, byte val)
{
	//use only the highest hex digit of the address
	switch(addr >> 12)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 10:
	case 11:
		//cartridge mapped address
		cart->write(addr, val);
		break;
	case 8:
	case 9:
		//stop if video ram access is denied
		if(vramAccess)
		{
			vramBanks[currentVRamBank][addr - 0x8000] = val;
		}
		break;
	case 12:
		workRamBanks[0][addr - 0xc000] = val;
		break;
	case 13:
		workRamBanks[currentWorkRamBank][addr - 0xd000] = val;
		break;

		//mem[addr] = val;

		////copy value to mirrored address
		//if(addr <= 0xde00)
		//	mem[addr+0x2000] = val;
		//break;
	case 15:
		if(addr <= 0xfe00)
		{
			//copy value to mirrored address
			mem[addr] = val;
			mem[addr-0x2000] = val;
		}
		else if(addr >= 0xfe00 && addr <= 0xfe9f && oamAccess)
			//oam, access must be allowed to write
			mem[addr] = val;
		else
			//a memory-based register is being written to
			specialRegisters(addr, val);
	}
}


/////////////////////////////////
//reads a byte from the specified video ram bank
//used when the vram bank is known
byte GBCMemoryManager::readVRam(word addr, byte bank)
{
	return vramBanks[bank & 1][addr - 0x8000];
}


byte GBCMemoryManager::readPaletteRam(byte offset)
{
	return paletteRam[offset & 0x3f];
}