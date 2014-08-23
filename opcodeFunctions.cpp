#include "Z80.h"

/////////////////////////////////
// 8 bit loads 
void Z80::load8(Register rd, Register rs)
{
	reg[rd] = reg[rs];
}


/////////////////////////////////
//loads the given 8 bit register with the given
//8 bit immediate value
//rd = destination register index
//is = immediate source
void Z80::load8(Register r, byte is)
{
	programCounter++;
	reg[r] = is;
}


/////////////////////////////////
//load the given 16 bit register with the
//given 8 bit immediate values representing
//the high and low bytes
void Z80::load16(Register rd, byte sh, byte sl)
{
	programCounter += 2;
	*((word*)&reg[rd]) = (sh << 8) + sl;
}


/////////////////////////////////
//load the given 16 bit register with the
//given 16 bit immediate value
void Z80::load16(Register rd, word s)
{
	programCounter += 2;
	*((word*)&reg[rd]) = s;
}


/////////////////////////////////
//push the given 16 bit register onto the stack
void Z80::push(Register r)
{
	if(r == AF)
		reg[F] = ZF + NF + HF + CF;
	mem->pushStack16(reg[r+1], reg[r]);
}


/////////////////////////////////
//pop 16 bits from the stack and put it in the
//given 16 bit register
void Z80::pop(Register r)
{
	word t = mem->popStack16();
	*((word*)&reg[r]) = t;
	if(r == AF)
	{
		CF = t & SET_CF;
		HF = t & SET_HF;
		ZF = t & SET_ZF;
		NF = t & SET_NF;
	}

}


/////////////////////////////////
//add the value of the given register to A
//H, C, ZF, and N flags are set as needed
void Z80::add8(Register r)
{
	((reg[A] & 0xf) + (reg[r] & 0xf) > 0xf) ? HF = SET_HF : HF = RESET;
	(reg[A] + reg[r] > 0xff) ? CF = SET_CF : CF = RESET;
	reg[A] += reg[r];
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
}


/////////////////////////////////
//add the given 8 bit immediate value to A.
//H, C, ZF, and N flags are set as needed
void Z80::add8(byte b)
{
	programCounter++;
	int initial = reg[A];
	int param = b;
	((initial & 0xf) + (param & 0xf) > 0xf) ? HF = SET_HF : HF = RESET;
	(initial + param > 0xff) ? CF = SET_CF : CF = RESET;
	reg[A] += b;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
}


/////////////////////////////////
//add the value of the given register and the carry 
//flag to A. H, C, ZF, and N flags are set as needed
void Z80::addPlusCarry8(Register r)
{
	int carry = (CF ? 1 : 0);
	int initial = reg[A];
	int param = reg[r];
	((initial & 0xf) + (param & 0xf) + carry > 0xf) ? HF = SET_HF : HF = RESET;
	(initial + param + carry > 0xff) ? CF = SET_CF : CF = RESET;
	reg[A] += (reg[r] + carry);
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
}


/////////////////////////////////
//add the given 8 bit immediate value and the carry flag
// to A. H, C, ZF, and N flags are set as needed
void Z80::addPlusCarry8(byte b)
{
	programCounter++;
	int initial = reg[A];
	int param = b;
	int carry = (CF ? 1 : 0);
	((initial & 0xf) + (param & 0xf) + carry > 0xf) ? HF = SET_HF : HF = RESET;
	(initial + param + carry > 0xff) ? CF = SET_CF : CF = RESET;
	reg[A] += (b + carry);
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
}


/////////////////////////////////
//subtract the value of the given register to A
//H, C, ZF, and N flags are set as needed
void Z80::sub8(Register r)
{
	int initial = reg[A];
	int param = reg[r];
	((initial & 0xf) - (param & 0xf) < 0) ? HF = SET_HF : HF = RESET;
	(initial - param < 0) ? CF = SET_CF : CF = RESET;
	reg[A] -= reg[r];
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = SET_NF;
}


/////////////////////////////////
//subtract the given 8 bit immediate value
//to A. H, C, ZF, and N flags are set as needed
void Z80::sub8(byte b)
{
	programCounter++;
	int initial = reg[A];
	int param = b;
	((initial & 0xf) - (param & 0xf) < 0) ? HF = SET_HF : HF = RESET;
	(initial - param < 0) ? CF = SET_CF : CF = RESET;
	reg[A] -= b;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = SET_NF;
}


/////////////////////////////////
//subtract the value of the given register and carry 
//flag to A. H, C, ZF, and N flags are set as needed
void Z80::subPlusCarry8(Register r)
{
	int initial = reg[A];
	int param = reg[r];
	int carry = (CF ? 1 : 0);
	((initial & 0xf) - (param & 0xf) - carry < 0) ? HF = SET_HF : HF = RESET;
	(initial - param - carry < 0) ? CF = SET_CF : CF = RESET;
	reg[A] = reg[A] - reg[r] - carry;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = SET_NF;
}


/////////////////////////////////
//subtract the 8 bit immediate value and carry flag to A.
//H, C, ZF, and N flags are set as needed
void Z80::subPlusCarry8(byte b)
{
	programCounter++;
	int initial = reg[A];
	int param = b;
	int carry = (CF ? 1 : 0);
	((initial & 0xf) - (param & 0xf) - carry < 0) ? HF = SET_HF : HF = RESET;
	(initial - param - carry < 0) ? CF = SET_CF : CF = RESET;
	reg[A] = reg[A] - b - carry;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = SET_NF;
}


/////////////////////////////////
//performs a bitwise AND on the given register and A
//H, C, ZF, and N flags are set as needed
void Z80::and8(Register r)
{
	reg[A] = reg[A] & reg[r];
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = SET_HF;
	CF = RESET;
}


/////////////////////////////////
//performs a bitwise AND on the given 8 bit immediate 
//value and A. H, C, ZF, and N flags are set as needed
void Z80::and8(byte b)
{
	programCounter++;
	reg[A] = reg[A] & b;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = SET_HF;
	CF = RESET;
}


/////////////////////////////////
//performs a bitwise OR on the given register and A
//H, C, ZF, and N flags are set as needed
void Z80::or8(Register r)
{
	reg[A] = reg[A] | reg[r];
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	CF = RESET;
}


/////////////////////////////////
//performs a bitwise OR on the given 8 bit immediate 
//value and A. H, C, ZF, and N flags are set as needed
void Z80::or8(byte b)
{
	programCounter++;
	reg[A] = reg[A] | b;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	CF = RESET;
}


/////////////////////////////////
//performs a bitwise XOR on the given register and A
//H, C, ZF, and N flags are set as needed
void Z80::xor8(Register r)
{
	reg[A] = reg[A] ^ reg[r];
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	CF = RESET;
}


/////////////////////////////////
//performs a bitwise XOR on the given 8 bit immediate 
//value and A. H, C, ZF, and N flags are set as needed
void Z80::xor8(byte b)
{
	programCounter++;
	reg[A] = reg[A] ^ b;
	(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	CF = RESET;
}


/////////////////////////////////
//compares the value in the given register with that of A
//H, C, ZF, and N flags are set as needed
void Z80::compare8(Register r)
{
	((reg[A] & 0xf) < (reg[r] & 0xf)) ? HF = SET_HF : HF = RESET;
	(reg[A] == reg[r]) ? ZF = SET_ZF : ZF = RESET;
	NF = SET_NF;
	(reg[A] < reg[r]) ? CF = SET_CF : CF = RESET;
}


/////////////////////////////////
//compares the value in the given 8 bit immediate value 
//with that of A. H, C, ZF, and N flags are set as needed
void Z80::compare8(byte b)
{
	programCounter++;
	((reg[A] & 0xf) < (b & 0xf)) ? HF = SET_HF : HF = RESET;
	(reg[A] == b) ? ZF = SET_ZF : ZF = RESET;
	NF = SET_NF;
	(reg[A] < b) ? CF = SET_CF : CF = RESET;
}


/////////////////////////////////
//increments the value in the given register
//H, C, ZF, and N flags are set as needed
void Z80::increment8(Register r)
{
	reg[r] += 1;
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	((reg[r] & 0xf) == 0) ? HF = SET_HF : HF = RESET;
}


/////////////////////////////////
//increments the given 8 bit immediate value and writes
//it to the address in HL.
//H, C, ZF, and N flags are set as needed
void Z80::increment8(byte b)
{
	b += 1;
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	((b & 0xf) == 0) ? HF = SET_HF : HF = RESET;
	mem->write(getReg16(HL),b);
}


/////////////////////////////////
//decrements the value in the given register
//H, C, ZF, and N flags are set as needed
void Z80::decrement8(Register r)
{
	NF = SET_NF;
	((reg[r] & 0xf) == 0) ? HF = SET_HF : HF = RESET;
	reg[r] -= 1;
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
}


/////////////////////////////////
//increments the given 8 bit immediate value and writes
//it to the addres in HL.
//H, C, ZF, and N flags are set as needed
void Z80::decrement8(byte b)
{
	NF = SET_NF;
	((b & 0xf) == 0) ? HF = SET_HF : HF = RESET;
	b -= 1;
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	mem->write(getReg16(HL),b);
}


/////////////////////////////////
//add the value of a 16 bit register to HL
void Z80::add16(Register r)
{
	int r16 = getReg16(r);
	int hl = getReg16(HL);
	((hl & 0xfff) + (r16 & 0xfff) > 0xfff) ? HF = SET_HF : HF = RESET;
	((hl + r16) > 0xffff) ? CF = SET_CF : CF = RESET;
	modReg16(HL, (word)r16);
	NF = RESET;
}


/////////////////////////////////
//increment a 16 bit register by 1
void Z80::increment16(Register r)
{
	*((word*)&reg[r]) += 1;
}


/////////////////////////////////
//decrement a 16 bit register by 1
void Z80::decrement16(Register r)
{
	*((word*)&reg[r]) -= 1;
}


/////////////////////////////////
//swap the upper and lower halves of the given
//8 bit reigster
void Z80::swap(Register r)
{
	swap8(r);
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET; HF = RESET; CF = RESET;
}


/////////////////////////////////
//swap the upper and lower halves of the given
//8-bit immediate value
void Z80::swap(byte b)
{
	b = ((b & 0xf) << 4) + (b >> 4);
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET; HF = RESET; CF = RESET;
	mem->write(getReg16(HL), b);
}

/* rotates and shifts */
/////////////////////////////////
//rotate register left, old bit 7 to carry flag
void Z80::rlc(Register r)
{
	((reg[r] & 0x80) != 0) ? CF = SET_CF : CF = RESET;
	reg[r] = (reg[r] << 1) | (((reg[r] & 0x80) != 0) ? 1 : 0); 
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//rotate immediate left, old bit 7 to carry flag
void Z80::rlc(byte b)
{
	((b & 0x80) != 0) ? CF = SET_CF : CF = RESET;
	b = (b << 1) | (((b & 0x80) != 0) ? 1 : 0); 
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//rotate register left through carry flag
void Z80::rl(Register r)
{
	byte carry = CF ? 1 : 0;
	((reg[r] & 0x80) != 0) ? CF = SET_CF : CF = RESET;	
	reg[r] = (reg[r] << 1) | carry;
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//rotate immediate left through carry flag
void Z80::rl(byte b)
{
	byte carry = CF ? 1 : 0;
	(b & 0x80) ? CF = SET_CF : CF = RESET;	
	b = ((b << 1) & 0xfe) | carry;
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//rotate register right, old bit 0 to carry flag
void Z80::rrc(Register r)
{
	CF = (reg[r] & 1) * SET_CF;
//	((reg[r] & 1) == 1) ? CF = SET_CF : CF = RESET;
	reg[r] = (reg[r] >> 1) | ((reg[r] & 1) << 7);
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//rotate immediate right, old bit 0 to carry flag
void Z80::rrc(byte b)
{
	CF = (b & 1) * SET_CF;
//	((b & 1) == 1) ? CF = SET_CF : CF = RESET;
	b = (b >> 1) | ((b & 1) << 7);
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//rotate register right through carry flag
void Z80::rr(Register r)
{
	byte carry = CF ? (1 << 7) : 0;
	CF = (reg[r] & 1) ? SET_CF : RESET;
	reg[r] = (reg[r] >> 1) | carry;
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//rotate immediate right through carry flag
void Z80::rr(byte b)
{
	byte carry = CF ? (1 << 7) : 0;
	CF = (b & 1) * SET_CF;
//	((b & 1) == 1) ? CF = SET_CF : CF = RESET;
	b = (b >> 1) | carry;
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//shift register left into carry, bit 0 to 0
void Z80::sla(Register r)
{
	((reg[r] & 0x80) != 0) ? CF = SET_CF : CF = RESET;
	reg[r] = (reg[r] << 1) & 0xfe;
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//shift immediate left into carry, bit 0 to 0
void Z80::sla(byte b)
{
	((b & 0x80) != 0) ? CF = SET_CF : CF = RESET;
	b = (b << 1) & 0xfe;
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//shift register right into carry, bit 7 to 0
void Z80::srl(Register r)
{
	CF = (reg[r] & 1) * SET_CF;
//	((reg[r] & 1) == 1) ? CF = SET_CF : CF = RESET;
	reg[r] = (reg[r] >> 1) & 0x7f;
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//shift immediate right into carry, bit 7 to 0
void Z80::srl(byte b)
{
	CF = (b & 1) * SET_CF;
//	((b & 1) == 1) ? CF = SET_CF : CF = RESET;
	b = (b >> 1) & 0x7f;
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//rotate register right into carry, bit 7 doesn't change
void Z80::sra(Register r)
{
	CF = (reg[r] & 1) * SET_CF;
//	((reg[r] & 1) == 1) ? CF = SET_CF : CF = RESET;
	reg[r] = ((reg[r] >> 1) & 0x7f) | (reg[r] & 0x80);
	(reg[r] == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
}


/////////////////////////////////
//rotate immediate right into carry, bit 7 doesn't change
void Z80::sra(byte b)
{
	CF = (b & 1) * SET_CF;
//	((b & 1) == 1) ? CF = SET_CF : CF = RESET;
	b = ((b >> 1) & 0x7f) | (b & 0x80);
	(b == 0) ? ZF = SET_ZF : ZF = RESET;
	NF = RESET;
	HF = RESET;
	mem->write(getReg16(HL), b);
}


/////////////////////////////////
//set the zero flag equal to the value of bit b
//of register r
void Z80::testBit(Register r, byte b)
{
	(reg[r] & (1 << b)) ? ZF = RESET : ZF = SET_ZF;
	NF = RESET;
	HF = SET_HF;
}


/////////////////////////////////
//set the value of bit b of register r to 1
void Z80::setBit(Register r, byte b)
{
	reg[r] = reg[r] | (1 << b);
}


/////////////////////////////////
//set the value of bit b of register r to 0
void Z80::resetBit(Register r, byte b)
{
	reg[r] = reg[r] & (0xff - (1 << b));
}


/////////////////////////////////
//jump to the address of sh, sl if the condition c is true
void Z80::jump(bool c, byte sh, byte sl)
{
	if(c)
		programCounter = (sh << 8) + sl;
	else
		programCounter += 2;
}


/////////////////////////////////
//add immediate value n to the PC and jump
//if the condition c is true
void Z80::jumpAddN(bool c, byte n)
{
	programCounter++;
	if(c)
		programCounter += (char)n;
}


/////////////////////////////////
//jump to the address of sh, sl if the condition c is true
//pushes PC onto the stack before jumping
void Z80::call(bool c, byte sh, byte sl)
{
	programCounter += 2;	
	if(c)
	{
		mem->pushStack16(programCounter);
		programCounter = (sh << 8) + sl;
		//mem->pushStack16(getReg16(PC));
		//movReg16(PC, sh, sl);
	}
}


/////////////////////////////////
//jump to the specified restart address
//pushes the PC to the stack before jumping
void Z80::restart(byte addr)
{
	mem->pushStack16(programCounter);
	programCounter = addr;
	//mem->pushStack16(getReg16(PC));
	//movReg16(PC, addr);
}


/////////////////////////////////
//returns from a function if the condition c is true
//pops 2 bytes from the stack into PC
void Z80::ret(bool c)
{
	if(c)
	{
		programCounter = mem->popStack16();
		//movReg16(PC, mem->popStack16());
	}
}