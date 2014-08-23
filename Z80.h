#ifndef Z80_H
#define Z80_H

class MemoryManager;
class LCD;

#include "enum.h"
#include "Cartridge.h"
#include "MemoryManager.h"
#include "LCD.h"
#include <fstream>
#include "IGameBoyComponent.h"


/*****
time per frame: 0.0167504187 s
vblank duration (seconds): 0.0011 s
vblank duration (cycles): 4620 cycles
time per scan line: 459
cpu speed (machine cycles): 4.19 MHz
time per machine cycle: 0.0000002380952375 s
cycles out of vblank per frame: 65728 cycles
dma duration: 1120 bits over 672 cycles
------timer------

bit 2:
0: timer off
1: timer on

bits 1+0:
frequency		overflows per second	clocks between overflows
0: 4096			16						262500
1: 262144		1024					4100
2: 65536		256						16404
3: 16384		64						65624



----------------
*****/

enum Register
{
	/* enumerated register names map to reg array */
	A = 1,
	AF = 0,
	F = 0,
	B = 3,
	BC = 2,
	C = 2,
	D = 5,
	DE = 4,
	E = 4,
	H = 7,
	HL = 6,
	L = 6,
	SP = 8,
	REG_MAX = 10,
	PC = 11
};

enum ALU_Flag
{
	SET_ZF = 0x80,
	SET_NF = 0x40,
	SET_HF = 0x20,
	SET_CF = 0x10,
	RESET = 0,
};

enum InterruptHandler
{
	/* interrupt handler addresses */
	VBLANK_HANLDER = 0x0040,
	LCDC_STATUS_HANLDER = 0x0048,
	TIMER_OVERFLOW_HANLDER = 0x0050,
	SERIAL_XFER_HANLDER = 0x0058,
	P10_P13_HI_LO_HANLDER = 0x0060
};

enum Interrupt
{
	/* interrupt bit value for IF reg */
	VBLANK = 1,
	LCDC_STATUS = 2,
	TIMER_OVERFLOW = 4,
	SERIAL_XFER = 8,
	P10_P13_HI_LO = 16
};

enum InterruptPriority
{
	/* interrupt priorities */
	VBLANK_PRIORITY = 1,
	LCDC_STATUS_PRIORITY = 2,
	TIMER_OVERFLOW_PRIORITY = 3,
	SERIAL_XFER_PRIORITY = 4,
	P10_P13_HI_LO_PRIORITY = 5
};


class Z80 : IGameBoyComponent
{
private:
	byte reg[REG_MAX];					//registers: indexed with enumerated names
	word programCounter;
	byte ZF;
	byte NF;
	byte HF;
	byte CF;
	byte current_op;					//opcode executed this iteration
	byte p1;							//opcode parameter byte 1
	byte p2;							//opcode parameter byte 2

	bool interruptMasterEnable;			//IME flag: all interrupts on/off
	bool delayedInterrupt;

	MemoryManager* mem;					//pointer to the memory manager
	Cartridge* cart;					//pointer to the current cartridge
	LCD* lcd;							//pointer to the LCD display


	int oplog_next;						//next index in the debug buffer
	double opFreq[256];					//opcode frequency when debugging
	std::ofstream logger;				//opcode logging file
	struct op_record {					//opcode and register log
		byte opcode;
		byte p1;
		byte p2;
		byte regs[REG_MAX];
	} oplog[1024];

public:
	double cpuClocks;					//total number of clock cycles elapsed
	bool cpuActive;						//has the cpu been halted or stopped?
	int instructionsExecuted;			//number of instructions: used for debugging
	int clockCounter;					//clock counter for the audio system
	byte lastOpClocks;					//number of clocks taken by the previous instruciton
	
	Z80();
	Z80(MemoryManager*);
	~Z80();
	void setComponents(MemoryManager*, Cartridge*, LCD*, Gameboy*);
	void printDebugInfo();

	void procOpcode();
	int procOp();
	int procOpcode_cb(byte op, byte p1);
	void reset();
	word readReg(Register r);

	/* hardware interrupts */
	void interrupts();
	void interrupt(InterruptHandler i);
	word getReg16(Register r);
	void modReg16(Register r, int v);
	void movReg16(Register r, byte h, byte l);
	void movReg16(Register r, word v);
	void swap8(Register r);
	void swap16(Register r);
	void incPC();
	void incPC(int v);
	void decPC();
	void decPC(int v);

	/* 8 bit loads */
	void load8(Register rd, Register rs);
	void load8(Register rd, byte is);

	/* 16 bit loads */
	void load16(Register rd, byte sh, byte sl);
	void load16(Register rd, word s);

	/* PUSH */
	void push(Register r);

	/* POP */
	void pop(Register r);

	/* 8 bit ALU */
	void add8(Register r);
	void add8(byte b);
	void addPlusCarry8(Register r);
	void addPlusCarry8(byte b);
	void sub8(Register r);
	void sub8(byte b);
	void subPlusCarry8(Register r);
	void subPlusCarry8(byte b);
	void and8(Register r);
	void and8(byte b);
	void or8(Register r);
	void or8(byte b);
	void xor8(Register r);
	void xor8(byte b);
	void compare8(Register r);
	void compare8(byte b);
	void increment8(Register r);
	void increment8(byte b);
	void decrement8(Register r);
	void decrement8(byte b);

	/* 16 bit ALU */
	void add16(Register r);
	void increment16(Register r);
	void decrement16(Register r);

	/* swap */
	void swap(Register r);
	void swap(byte b);

	/* rotates and shifts */
	void rlc(Register r);
	void rl(Register r);
	void rrc(Register r);
	void rr(Register r);
	void sla(Register r);
	void srl(Register r);
	void sra(Register r);
	void rlc(byte b);
	void rl(byte b);
	void rrc(byte b);
	void rr(byte b);
	void sla(byte b);
	void srl(byte b);
	void sra(byte b);

	/* bit manipulations */
	void testBit(Register r, byte bit);
	void setBit(Register r, byte bit);
	void resetBit(Register r, byte bit);

	/* jumps */
	void jump(bool c, byte sh, byte sl);
	void jump(bool c, word s);
	void jumpAddN(bool c, byte n);

	/* calls */
	void call(bool c, byte sh, byte sl);
	void call(bool c, word s);

	/* restarts */
	void restart(byte addr);

	/* returns */
	void ret(bool c);

	void resetClockCounter();
	int getClockCounter();

	RegisterInfo getRegisterInfo();
	void constructState(State*);
	void restoreState(State*);
};

#endif //Z80_H
