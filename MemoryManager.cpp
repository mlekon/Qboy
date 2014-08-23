#include "MemoryManager.h"
#include "Gameboy.h"

MemoryManager::MemoryManager()
{
	oamAccess = true;
	vramAccess = true;
	initMem();
}


/////////////////////////////////
//sets pointers to other gameboy components
//that the memory must interact with
void MemoryManager::setComponents(Z80* z, Cartridge* c, Timer* t, Audio* a, Gameboy* gb)
{
	this->gb = gb;
	z80 = z;
	cart = c;
	time = t;
	audio = a;
}


/////////////////////////////////
//sets initial memory values that have a
//specific value on startup
void MemoryManager::initMem()
{
	memset(mem, 0, 0x10000);
	mem[TIMA] = 0;		mem[TMA] = 0;		mem[TAC] = 0;		mem[NR10] = 0x80;
	mem[NR11] = 0xbf;	mem[NR12] = 0xf3;	mem[NR14] = 0xbf;	mem[NR21] = 0x3f;
	mem[NR22] = 0;		mem[NR24] = 0xbf;	mem[NR30] = 0x7f;	mem[NR31] = 0xff;
	mem[NR32] = 0x9f;	mem[NR33] = 0xbf;	mem[NR34] = 0xbf;	mem[NR41] = 0xff;	
	mem[NR42] = 0;		mem[NR43] = 0;		mem[NR44] = 0xbf;	mem[NR50] = 0x77;	
	mem[NR51] = 0xf3;	mem[NR52] = 0xf1;	mem[LCDC] = 0x91;	mem[SCY] = 0;		
	mem[SCX] = 0;		mem[LYC] = 0;		mem[BGP] = 0xfc;	mem[OBP0] = 0xff;
	mem[OBP1] = 0xff;	mem[WY] = 0;		mem[WX] = 0;		mem[IE] = 0;		
	mem[LY] = 0;	
}


/////////////////////////////////
//set access to video ram
void MemoryManager::setVramAccess(bool b)
{
	vramAccess = b;
}


/////////////////////////////////
//sets access to OAM
void MemoryManager::setOamAccess(bool b)
{
	oamAccess = b;
}


/////////////////////////////////
//push one byte onto the stack specified
//by the stack pointer of the cpu component
//the stack pointer is automatically adjusted
void MemoryManager::pushStack8(byte b)
{
	z80->modReg16(SP, -1);
	write(z80->getReg16(SP), b);
}


/////////////////////////////////
//push two bytes onto the stack specified
//by the stack pointer of the cpu component
//the stack pointer is automatically adjusted
void MemoryManager::pushStack16(byte high, byte low)
{
	z80->modReg16(SP, -1);
	writeRam(z80->getReg16(SP), high);
	z80->modReg16(SP, -1);
	writeRam(z80->getReg16(SP), low);
}


/////////////////////////////////
//push two bytes (one word) onto the stack specified
//by the stack pointer of the cpu component
//the stack pointer is automatically adjusted
void MemoryManager::pushStack16(word w)
{
	byte high = (w >> 8) & 0xff;
	byte low = w & 0xff;
	z80->modReg16(SP, -1);
	writeRam(z80->getReg16(SP), high);
	z80->modReg16(SP, -1);
	writeRam(z80->getReg16(SP), low);
}


/////////////////////////////////
//pops one byte from the stack and returns the value
//the stack pointer is automatically adjusted
byte MemoryManager::popStack8()
{
	byte r = read(z80->getReg16(SP));
	z80->modReg16(SP, 1);
	return r;
}


/////////////////////////////////
//pops two bytes from the stack and returns the value
//the stack pointer is automatically adjusted
word MemoryManager::popStack16()
{

	byte low = read(z80->getReg16(SP));
	z80->modReg16(SP, 1);
	byte high = read(z80->getReg16(SP));
	z80->modReg16(SP, 1);
	return ((word)high << 8) + low;
}


/////////////////////////////////
//read the byte at the given memory address
byte MemoryManager::read(word addr)
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
	case 4:
	case 6:
	case 7:
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15:
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
byte MemoryManager::readRam(word addr)
{
	return mem[addr];
}


/////////////////////////////////
//slightly faster version of write()
//used only when writing to the gameboy's internal ram
void MemoryManager::writeRam(word addr, byte val)
{
	if(addr >= 0xe000 && addr <= 0xfe00)
		mem[addr - 0x2000] = val;
	if(addr >= 0xc000 && addr <= 0xde00)
		mem[addr + 0x2000] = val;
	mem[addr] = val;
}


/////////////////////////////////
//writes an 8 bit value to the given address
//of the memory. uses the cartridge for addresses
//that fall within the cartridge-mapped memory
void MemoryManager::write(word addr, byte val)
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
		if(!vramAccess)
			return;
	case 12:
	case 13:
		mem[addr] = val;

		//copy value to mirrored address
		if(addr <= 0xde00)
			mem[addr+0x2000] = val;
		break;
	case 14:
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
//set the ports indicating the pressed controls
void MemoryManager::setInputPorts(byte p10, byte p20) 
{
	port10 = p10;
	port20 = p20;
}


/////////////////////////////////
//request an interrupt corresponding to the given value
//the interrupt is processed immediately
void MemoryManager::requestInterrupt(Interrupt b)
{
	mem[0xff0f] = b;
	z80->interrupts();
}


/////////////////////////////////
//writes to a special register in the high memory
//some special registers have special requirements
//which are taken care of here
void MemoryManager::specialRegisters(word addr, byte val)
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
void MemoryManager::constructState(State* s)
{
	memcpy(s->videoRam, &mem[0x8000], VIDEO_RAM_SIZE);
	memcpy(s->upperRam, &mem[0xc000], UPPER_RAM_SIZE);
	s->vramAccess = this->vramAccess;
	s->oamAccess = this->oamAccess;
}


/////////////////////////////////
//restores the non-cartridge memory from the given state
void MemoryManager::restoreState(State* s)
{
	memcpy(&mem[0x8000], s->videoRam, VIDEO_RAM_SIZE);
	memcpy(&mem[0xc000], s->upperRam, UPPER_RAM_SIZE);
	this->vramAccess = s->vramAccess;
	this->oamAccess = s->oamAccess;
}


/////////////////////////////////
//set the input port value for the A, B, start & select
void MemoryManager::setInput10(byte b)
{
	port10 = b;
}


/////////////////////////////////
//set the inpuy poty value for the directional pad
void MemoryManager::setInput20(byte b)
{
	port20 = b;
}


/////////////////////////////////
//gets the pointer to a specific part of the memory array
byte* MemoryManager::getMemArrayAddress(word gbAddr)
{
	return &mem[gbAddr];
}


/////////////////////////////////
//check whether the input was read by the
//client code since the last vblank
bool MemoryManager::lagFrame()
{
	bool ret = !inputReadThisFrame;
	inputReadThisFrame = false;
	return ret;
}


/////////////////////////////////
//restore the inital state of the memory
void MemoryManager::reset()
{
	initMem();
	oamAccess = false;
	vramAccess = true;
	inputReadThisFrame = false;
}


void MemoryManager::addBreakpoint(Breakpoint bp)
{
	//if(gb->getBpm()->getBreakpoint(bp.address) == NULL)
	//{
	//	if(bp.address < 0x8000)
	//	{
	//		bp = cart->addBreakpoint(bp);
	//	}
	//	else
	//	{
	//		bp.opcode = mem[bp.address];
	//		bp.source = -1;
	//		mem[bp.address] = BREAKPOINT_CODE;
	//	}
	//
	//	gb->getBpm()->addBreakpoint(bp.address, bp);
	//}
}


Breakpoint MemoryManager::removeBreakpoint(word address)
{
	Breakpoint bp;	// = gb->getBpm()->removeBreakpoint(address);
	//if(address < 0x8000)
	//{
	//	cart->removeBreakpoint(bp);
	//}
	//else
	//{
	//	mem[address] = bp.opcode;
	//}

	return bp;
}