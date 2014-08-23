#ifndef GB_H
#define GB_H

typedef unsigned char byte;
typedef unsigned short word;

void init_memory();
int proc_opcode(byte, byte, byte);
word getReg16(int);
void modReg16(int, int);
void movReg16(int, byte, byte);
void movReg16(int, word);
byte getZF();
void setZF();
void resetZF();
byte getN();
void setN();
void resetN();
byte getH();
void setH();
void resetH();
byte getC();
void setC();
void resetC();
void swap8(int);
void swap16(int);
void push_stack8(byte);
void push_stack16(byte, byte);
void push_stack16(word);
byte pop_stack8();
word pop_stack16();

enum {
	/* enumated register names map to reg array */
	A = 0,
	AF = 0,
	F = 1,
	B = 2,
	BC = 2,
	C = 3,
	D = 4,
	DE = 4,
	E = 5,
	H = 6,
	HL = 6,
	L = 7,
	SP = 8,
	PC = 10,
	FLAG = 12,

	/* interrupt handler addresses */
	VBLANK = 0x0040,
	LCDC_STATUS = 0x0048,
	TIMER_OVERFLOW = 0x0050,
	SERIAL_XFER = 0x0058,
	P10_P13_HI_LO = 0x0060,

	/* cartridge types */
	ROM = 0,
	ROM_MBC1 = 1,
	ROM_MBC1_RAM = 2,
	ROM_MBC1_RAM_BATT = 3,
	ROM_MBC2 = 5,
	ROM_MBC2_BATT = 6,
	ROM_RAM = 8,
	ROM_RAM_BATT = 9,
	ROM_MMMO1 = 0xb,
	ROM_MMMO1_SRAM = 0xc,
	ROM_MMMO1_SRAM_BATT = 0xd,
	ROM_MBC3_TIMER_BATT = 0xf,
	ROM_MBC3_TIMER_RAM_BATT = 0x10,
	ROM_MBC3 = 0x11,
	ROM_MBC3_RAM = 0x12,
	ROM_MBC3_RAM_BATT = 0x13,
	ROM_MBC5 = 0x19,
	ROM_MBC5_RAM = 0x1a,
	ROM_MBC5_RAM_BATT = 0x1b,
	ROM_MBC5_RUMBLE = 0x1c,
	ROM_MBC5_RUMBlE_SRAM = 0x1d,
	ROM_MBC5_RUMBLE_SRAM_BATT = 0x1e,
	Pocket_Camera = 0x1f,
	Bandai_TAMA5 = 0xfd,
	Hudson_HuC_3 = 0xfe,
	Hudson_HuC_1 = 0xff
};

extern typedef struct mem
{
	byte mem[0xffff+1]
	void write(word address, byte value)
	{
	}
}
/* entire memory map */
extern byte mem[0xffff+1];

/* register contents */
extern byte reg[13];

/* are interrupts enabled? */
extern bool interrupt_master_enable;
extern bool interrupt_enabled;
extern bool cpu_active;

/* flag register masks */
extern byte zero_flag;
extern byte subtract_flag;
extern byte half_carry_flag;
extern byte carry_flag;

/* memory region pointers */
extern word interrupt_enable_register;
extern word internal_ram;
extern word unused_high;
extern word io_ports;
extern word unused_low;
extern word oam;
extern word internal_ram_8k_echo;
extern word internal_ram_8k;
extern word switchable_ram;
extern word video_ram;
extern word switchable_rom;
extern word rom_bank_0;

/* values checked on startup */
const byte nintendo_logo[] = {
	0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03,
	0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08,
	0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e,
	0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
	0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9,
	0x33, 0x3e
};

#endif