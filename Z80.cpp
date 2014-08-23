#include "Z80.h"
#include "allegro.h"
#include <fstream>


Z80::Z80()
{
	reset();
}


Z80::~Z80()
{
#ifdef DEBUG
	logger.close();
#endif
}


/////////////////////////////////
//reset all registers and control variables
//so that it behaves as if the system experienced a hard reset
void Z80::reset()
{
	instructionsExecuted = 0;
	cpuClocks = 0;
	clockCounter = 0;
	current_op = 0;
	p1 = p2 = 0;
	delayedInterrupt = false;
	lastOpClocks = 0;
	reg[A] = 0x11;
	reg[F] = 0xb0;
	reg[B] = 0;
	reg[C] = 0x13;
	reg[D] = 0;
	reg[E] = 0xd8;
	reg[H] = 1;
	reg[L] = 0x4d;
	movReg16(SP, 0xfffe);
	programCounter = 0x0100;
	ZF = NF = HF = CF = RESET;
	interruptMasterEnable = true;
	cpuActive = true;

#ifdef DEBUG
	memset(opFreq, 0, sizeof(opFreq));
	logger.open("opcode_log.txt");
	oplog_next = 0;
#endif
}


/////////////////////////////////
//sets the necessary links to the components
//the CPU needs to communicate with
void Z80::setComponents(MemoryManager* m, Cartridge* c, LCD* l, Gameboy* gb)
{
	this->gb = gb;
	lcd = l;
	mem = m;
	cart = c;
}


/////////////////////////////////
//gets the value of the specified register
word Z80::readReg(Register r)
{
	switch(r)
	{
	case AF:
	case BC:
	case DE:
	case HL:
	case SP:
		return getReg16(r);
	case PC:
		return programCounter;
	default:
		return reg[r];
	}
}


/////////////////////////////////
//begin executing an interrupt handler
//after pushing the PC onto the stack
void Z80::interrupt(InterruptHandler i)
{
	mem->pushStack16(programCounter);
	programCounter = i;
}


/////////////////////////////////
//returns the value of a 16 bit register
word Z80::getReg16(Register r)
{
	return *((word*)&reg[r]);
}


/////////////////////////////////
//changes the values of a 16 bit register
//relative to the 'v' parameter
void Z80::modReg16(Register r, int v)
{
	*((word*)&reg[r]) += v;
}


/////////////////////////////////
//write a value to a 16-bit register
void Z80::movReg16(Register r, byte l, byte h)
{
	*((word*)&reg[r]) = ((word)h << 8) + l;
}


/////////////////////////////////
//copies the 16-bit immediate value
//to a 16-bit register
void Z80::movReg16(Register r, word s)
{
	*((word*)&reg[r]) = s;
}


/////////////////////////////////
//swaps the upper and lower 4 bits of
//the given 8 bit register
void Z80::swap8(Register r)
{
	reg[r] = ((reg[r] & 0xf) << 4) + (reg[r] >> 4);
}


/////////////////////////////////
//swaps the upper and lower 4 bits of
//the given 16 bit register
void Z80::swap16(Register r)
{
	byte temp = reg[r];
	reg[r] = reg[r+1];
	reg[r+1] = temp;
}


/////////////////////////////////
//saves values of the CPU into the given state
//such that the state of the CPU can be recreated
void Z80::constructState(State* s)
{
	reg[F] = CF + HF + ZF + NF;
	memcpy(s->registers, this->reg, sizeof(reg));
	s->programCounter = programCounter;
	s->CF = CF; s->ZF = ZF; s->HF = HF; s->NF = NF;
	s->interruptMasterEnable = interruptMasterEnable;
	s->cpuHalted = this->cpuActive;
	s->clocks = this->cpuClocks;
}


/////////////////////////////////
//recreate the CPU state given by the state parameter
void Z80::restoreState(State* s)
{
	memcpy(this->reg, s->registers, sizeof(reg));
	programCounter = s->programCounter;
	CF = s->CF; HF = s->HF; ZF = s->ZF; NF = s->NF;
	reg[F] = CF + HF + ZF + NF;
	interruptMasterEnable = s->interruptMasterEnable;
	this->cpuActive = true;
	this->cpuClocks = s->clocks;
}


void Z80::printDebugInfo()
{
	//GBOPCODE i = opcodes[p1];
	//printf("Op: 0x%x\tp1 %x, p2 %x\n", current_op, p1, p2);
	//std::cout << "Registers: \n";
	//for(int i = 0; i < REG_MAX; i++)
	//{
	//	printf("%x\t", reg[i]);
	//}
	//std::cout << "\n";
}


/////////////////////////////////
//test for interrupt requests and calls the appropriate
//interrupt handler routine
void Z80::interrupts()
{
	//interrupt may only occur if the master enable switch is on
	//if 0xff0f is 0, then no interrupts have been requested
	//if 0xffff is 0, then no interrupts have been enabled
	if(interruptMasterEnable && mem->readRam(0xff0f) > 0 && mem->readRam(0xffff) > 0)
	{
		//loop through the 5 interrupts in decending priority
		for(int i = 0; i < 5; i++)
		{
			if(mem->readRam(0xffff) & (1 << i) && mem->readRam(0xff0f) & (1 << i))
			{
				//interrupt is enabled and requested. call the handler
				interrupt((InterruptHandler)(0x40 + (8 * i)));
				mem->write(0xff0f, mem->readRam(0xff0f) ^ (1 << i));		//reset interrupt flag bit
				cpuActive = true;
				interruptMasterEnable = false;

				//only the highest priority interrupt can execute
				break;
			}
		}
	}
}


/////////////////////////////////
//sets the audio counter to 0
void Z80::resetClockCounter()
{
	clockCounter = 0;
}


/////////////////////////////////
//returns the audio counter
int Z80::getClockCounter()
{
	return clockCounter;
}


/////////////////////////////////
//gets a dump of the current cpu registers
RegisterInfo Z80::getRegisterInfo()
{
	RegisterInfo ri;
	memcpy(ri.reg, reg, REG_MAX);
	ri.PC = programCounter;
	ri.CF = CF != 0;
	ri.HF = HF != 0;
	ri.NF = NF != 0;
	ri.ZF = ZF != 0;

	return ri;
}