#ifndef GB_OP
#define GB_OP

#include "enum.h"
#include "Z80.h"
#include "Gameboy.h"
#include "Disassembly.h"

/////////////////////////////////
//calls the procOp function to execute the next instruction
//the return value represents the number of clocks elapsed
//from the instruction. it's used to update the timers
void Z80::procOpcode()
{
	lastOpClocks = procOp();
	cpuClocks += lastOpClocks;
	clockCounter += lastOpClocks;
}


/////////////////////////////////
//executes a basic CPU operation based on the opcode in
//parameter "op". up to two parameters may be given for an
//opcode, each one byte in length. for two-byte opcodes, the
//first parameter byte is used as the second half of the 
//total opcode. returns the number of parameters used by an
//opcode + 1, to indicate the necessary IP increment
int Z80::procOp()
{
	if(!cpuActive)
	{
		return 4;
	}

	byte op = mem->read(programCounter);
	byte p1, p2;

	if(opcodeOperands[op] == 1)
		p1 = mem->read(programCounter+1); //high
	else
	{
		p1 = mem->read(programCounter+1); //high
		p2 = mem->read(programCounter+2); //low
	}

//#ifdef DEBUGGER
//	if(gb->getBpm()->checkBreakpoint(programCounter, op))
//	{
//		//debug break
//	}
//#endif

#ifdef DEBUG
	this->current_op = op;

	oplog[oplog_next].opcode = op;
	oplog[oplog_next].p1 = p1;
	oplog[oplog_next].p2 = p2;
	memcpy(oplog[oplog_next].regs, reg, sizeof(byte) * REG_MAX);
	oplog_next = (oplog_next + 1)%1024;

	instructionsExecuted++;

	if(instructionsExecuted <= 0x20000)
	{
		logger.write((char*)&op,1);
		if(instructionsExecuted == 0x20000)
			logger.close();
	}
#endif

#ifdef DEBUGGER
	Breakpoint* bp;
	if(gb->getBpm()->executeBreakpoints > 0 && (bp = gb->getBpm()->breakpointMap[programCounter]) != NULL && !gb->debugIsBroken())
	{
		if(bp->type & BreakpointExecute)
		{
			gb->debugBreak();
			return 0;
		}
	}
#endif

	//normal program counter advancement
	//additional changes are made by various instructions
	programCounter++;
	
	//loop to allow debug breakpoints to
	//be re-processed with their original opcode
	do
	{
		switch(op)
		{
		case 0x3e:			//LD A, #	
			load8(A, p1);
			return 8;
		case 0x06:			//LD B, #
			load8(B, p1);
			return 8;
		case 0x0e:			//LD C, #
			load8(C, p1);
			return 8;
		case 0x16:			//LD D, #
			load8(D, p1);
			return 8;
		case 0x1e:			//LD E, #
			load8(E, p1);
			return 8;
		case 0x26:			//LD H, #
			load8(H, p1);
			return 8;
		case 0x2e:			//LD L, #
			load8(L, p1);
			return 8;

		case 0x7f:			//LD A, A
			load8(A, A);
			return 4;
		case 0x78:			//LD A, B
			load8(A, B);
			return 4;
		case 0x79:			//LD A, C		
			load8(A, C);
			return 4;
		case 0x7a:			//LD A, D
			load8(A, D);
			return 4;
		case 0x7b:			//LD A, E
			load8(A, E);
			return 4;
		case 0x7c:			//LD A, H
			load8(A, H);
			return 4;
		case 0x7d:			//LD A, L
			load8(A, L);
			return 4;
		case 0x7e:			//LD A, [HL]
			load8(A, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x40:			//LD B, B
			load8(B, B);
			return 4;
		case 0x41:			//LD B, C
			load8(B, C);
			return 4;
		case 0x42:			//LD B, D
			load8(B, D);
			return 4;
		case 0x43:			//LD B, E
			load8(B, E);
			return 4;
		case 0x44:			//LD B, H
			load8(B, H);
			return 4;
		case 0x45:			//LD B, L
			load8(B, L);
			return 4;
		case 0x46:			//LD B, [HL]
			load8(B, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x48:			//LD C, B
			load8(C, B);
			return 4;
		case 0x49:			//LD C, C
			load8(C, C);
			return 4;
		case 0x4a:			//LD C, D
			load8(C, D);
			return 4;
		case 0x4b:			//LD C, E
			load8(C, E);
			return 4;
		case 0x4c:			//LD C, H
			load8(C, H);
			return 4;
		case 0x4d:			//LD C, L
			load8(C, L);
			return 4;
		case 0x4e:			//LD C, [HL]
			load8(C, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x50:			//LD D, B
			load8(D, B);
			return 4;
		case 0x51:			//LD D, C
			load8(D, C);
			return 4;
		case 0x52:			//LD D, D
			load8(D, D);
			return 4;
		case 0x53:			//LD D, E
			load8(D, E);
			return 4;
		case 0x54:			//LD D, H
			load8(D, H);
			return 4;
		case 0x55:			//LD D, L
			load8(D, L);
			return 4;
		case 0x56:			//LD D, [HL]
			load8(D, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x58:			//LD E, B
			load8(E, B);
			return 4;
		case 0x59:			//LD E, C
			load8(E, C);
			return 4;
		case 0x5a:			//LD E, D
			load8(E, D);
			return 4;
		case 0x5b:			//LD E, E
			load8(E, E);
			return 4;
		case 0x5c:			//LD E, H
			load8(E, H);
			return 4;
		case 0x5d:			//LD E, L
			load8(E, L);
			return 4;
		case 0x5e:			//LD E, [HL]
			load8(E, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x60:			//LD H, B
			load8(H, B);
			return 4;
		case 0x61:			//LD H, C
			load8(H, C);
			return 4;
		case 0x62:			//LD H, D
			load8(H, D);
			return 4;		
		case 0x63:			//LD H, E
			load8(H, E);
			return 4;
		case 0x64:			//LD H, H
			load8(H, H);
			return 4;
		case 0x65:			//LD H, L
			load8(H, L);
			return 4;
		case 0x66:			//LD H, [HL]
			load8(H, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x68:			//LD L, B
			load8(L, B);
			return 4;
		case 0x69:			//LD, L, C
			load8(L, C);
			return 4;
		case 0x6a:			//LD L, D
			load8(L, D);
			return 4;	
		case 0x6b:			//LD L, E
			load8(L, E);
			return 4;
		case 0x6c:			//LD L, H
			load8(L, H);
			return 4;
		case 0x6d:			//LD L, L
			load8(L, L);
			return 4;
		case 0x6e:			//LD L, [HL]
			load8(L, mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0x70:			//LD [HL], B
			mem->write(getReg16(HL), reg[B]);
			return 8;
		case 0x71:			//LD [HL], C
			mem->write(getReg16(HL), reg[C]);
			return 8;
		case 0x72:			//LD [HL], D
			mem->write(getReg16(HL), reg[D]);
			return 8;
		case 0x73:			//LD [HL], E
			mem->write(getReg16(HL), reg[E]);
			return 8;
		case 0x74:			//LD [HL], H
			mem->write(getReg16(HL), reg[H]);
			return 8;
		case 0x75:			//LD [HL], L
			mem->write(getReg16(HL), reg[L]);
			return 8;
		case 0x36:			//LD [HL], #
			mem->write(getReg16(HL), p1);
			programCounter++;
			return 12;

		case 0x0a:			//LD A, [BC]
			load8(A, mem->read(getReg16(BC)));
			programCounter--;
			return 8;
		case 0x1a:			//LD A, [DE]
			load8(A, mem->read(getReg16(DE)));
			programCounter--;
			return 8;
		case 0xfa:			//LD A, [##]
			load8(A, mem->read((p2 << 8) + p1));
			programCounter++;
			return 16;
		case 0x02:			//LD [BC], A
			mem->write(getReg16(BC), reg[A]);
			return 8;
		case 0x12:			//LD [DE], A
			mem->write(getReg16(DE), reg[A]);
			return 8;
		case 0x77:			//LD [HL], A
			mem->write(getReg16(HL), reg[A]);
			return 8;
		case 0xea:			//LD [##], A
			mem->write((p2 << 8) + p1, reg[A]);
			programCounter += 2;
			return 16;

		/* load A into 8-bit registers */
		case 0x47:			//LD B, A
			load8(B, A);
			return 4;
		case 0x4f:			//LD C, A
			load8(C, A);
			return 4;
		case 0x57:			//LD D, A
			load8(D, A);
			return 4;
		case 0x5f:			//LD E, A
			load8(E, A);
			return 4;
		case 0x67:			//LD H, A
			load8(H, A);
			return 4;
		case 0x6f:			//LD L, A
			load8(L, A);
			return 4;

		case 0xf2:			//LD A, [0xff00 + C]
			reg[A] = mem->read(0xff00 + reg[C]);
			return 8;
		case 0xe2:			//LD [0xff00 + C], A
			mem->write(0xff00 + reg[C], reg[A]);
			return 8;
		case 0x3a:			//LD A, [HL-]
			reg[A] = mem->read(getReg16(HL));
			decrement16(HL);
			return 8;
		case 0x32:			//LD [HL-], A
			mem->write(getReg16(HL), reg[A]);
			decrement16(HL);
			return 8;
		case 0x2a:			//LD A, [HL+]
			load8(A, mem->read(getReg16(HL)));
			programCounter--;
			increment16(HL);
			return 8;
		case 0x22:			//LD [HL+], A
			mem->write(getReg16(HL), reg[A]);
			increment16(HL);
			return 8;
		case 0xe0:			//LDH [0xff00 + #], A
			mem->write(0xff00 + p1, reg[A]);
			programCounter++;
			return 12;
		case 0xf0:			//LDH A, [0xff00 + #]
			load8(A, mem->read(0xff00 + p1));
			return 12;

		/* 16 bit loads */
		case 0x01:			//LD BC, ##
			load16(BC, p2, p1);
			return 12;
		case 0x11:			//LD DE, ##
			load16(DE, p2, p1);
			return 12;
		case 0x21:			//LD HL, ##
			load16(HL, p2, p1);
			return 12;
		case 0x31:			//LD SP, ##
			load16(SP, p2, p1);
			return 12;
		case 0xf9:			//LD SP, HL
			load16(SP, getReg16(HL));
			programCounter -= 2;
			return 8;
		case 0xf8:			//LDHL HL, SP + #
			load16(HL, (word)getReg16(SP) + (char)p1);
			ZF = RESET;
			NF = RESET;
			programCounter--;
			return 12;
		case 0x08:			//LD [##], SP
			mem->write((p2 << 8) + p1, getReg16(SP));
			programCounter += 2;
			return 20;

		/* push the register to the stack */
		case 0xf5:			//PUSH AF
			push(AF);
			return 16;
		case 0xc5:			//PUSH BC
			push(BC);
			return 16;
		case 0xd5:			//PUSH DE
			push(DE);
			return 16;
		case 0xe5:			//PUSH HL
			push(HL);
			return 16;

		/* pop from the stack into the register */
		case 0xf1:			//POP AF
			pop(AF);
			return 12;
		case 0xc1:			//POP BC
			pop(BC);
			return 12;
		case 0xd1:			//POP DE
			pop(DE);
			return 12;
		case 0xe1:			//POP HL
			pop(HL);
			return 12;

		/* add to register A */
		case 0x87:			//ADD A, A
			add8(A);
			return 4;
		case 0x80:			//ADD A, B
			add8(B);
			return 4;
		case 0x81:			//ADD A, C
			add8(C);
			return 4;
		case 0x82:			//ADD A, D
			add8(D);
			return 4;
		case 0x83:			//ADD A, E
			add8(E);
			return 4;
		case 0x84:			//ADD A, H
			add8(H);
			return 4;
		case 0x85:			//ADD A, L
			add8(L);
			return 1;
		case 0x86:			//ADD A, [HL]
			add8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xc6:			//ADD A, #
			add8(p1);
			return 8;

		/* add to register A + carry flag */
		case 0x8f:			//ADC A, A
			addPlusCarry8(A);
			return 4;
		case 0x88:			//ADC A, B
			addPlusCarry8(B);
			return 4;
		case 0x89:			//ADC A, C
			addPlusCarry8(C);
			return 4;
		case 0x8a:			//ADC A, D
			addPlusCarry8(D);
			return 4;
		case 0x8b:			//ADC A, E
			addPlusCarry8(E);
			return 4;
		case 0x8c:			//ADC A, H
			addPlusCarry8(H);
			return 4;
		case 0x8d:			//ADC A, L
			addPlusCarry8(L);
			return 4;
		case 0x8e:			//ADC A, [HL]
			addPlusCarry8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xce:			//ADC A, #
			addPlusCarry8(p1);
			return 8;

		/* subtract from register A */
		case 0x97:			//SUB A, A
			sub8(A);
			return 4;
		case 0x90:			//SUB A, B
			sub8(B);
			return 4;
		case 0x91:			//SUB A, C
			sub8(C);
			return 4;
		case 0x92:			//SUB A, D
			sub8(D);
			return 4;
		case 0x93:			//SUB A, E
			sub8(E);
			return 4;
		case 0x94:			//SUB A, H
			sub8(H);
			return 4;
		case 0x95:			//SUB A, L
			sub8(L);
			return 4;
		case 0x96:			//SUB A, [HL]
			sub8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xd6:			//SUB A, #
			sub8(p1);
			return 8;

		/* subtract from register A and carry flag */
		case 0x9f:			//SBC A, A
			subPlusCarry8(A);
			return 4;
		case 0x98:			//SBC A, B
			subPlusCarry8(B);
			return 4;
		case 0x99:			//SBC A, C
			subPlusCarry8(C);
			return 4;
		case 0x9a:			//SBC A, D
			subPlusCarry8(D);
			return 4;
		case 0x9b:			//SBC A, E
			subPlusCarry8(E);
			return 4;
		case 0x9c:			//SBC A, H
			subPlusCarry8(H);
			return 4;
		case 0x9d:			//SBC A, L
			subPlusCarry8(L);
			return 4;
		case 0x9e:			//SBC A, [HL]
			subPlusCarry8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xde:			//SBC A, #
			subPlusCarry8(p1);
			return 8;

		/* logical AND with register A, result in A */
		case 0xa7:			//AND A, A
			and8(A);
			return 4;
		case 0xa0:			//AND A, B
			and8(B);
			return 4;
		case 0xa1:			//AND A, C
			and8(C);
			return 4;
		case 0xa2:			//AND A, D
			and8(D);
			return 4;
		case 0xa3:			//AND A, E
			and8(E);
			return 4;
		case 0xa4:			//AND A, H
			and8(H);
			return 4;
		case 0xa5:			//AND A, L
			and8(L);
			return 4;
		case 0xa6:			//AND A, [HL]
			and8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xe6:			//AND A, #
			and8(p1);
			return 8;

		/* logical OR with register A, result in A */
		case 0xb7:			//OR A, A
			or8(A);
			return 4;
		case 0xb0:			//OR A, B
			or8(B);
			return 4;
		case 0xb1:			//OR A, C
			or8(C);
			return 4;
		case 0xb2:			//OR A, D
			or8(D);
			return 4;
		case 0xb3:			//OR A, E
			or8(E);
			return 4;
		case 0xb4:			//OR A, H
			or8(H);
			return 4;
		case 0xb5:			//OR A, L
			or8(L);
			return 4;
		case 0xb6:			//OR A, [HL]
			or8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xf6:			//OR A, #
			or8(p1);
			return 8;

		/* logical XOR with register A, result in A */
		case 0xaf:			//XOR A, A
			xor8(A);
			return 4;
		case 0xa8:			//XOR A, B
			xor8(B);
			return 4;
		case 0xa9:			//XOR A, C
			xor8(C);
			return 4;
		case 0xaa:			//XOR A, D
			xor8(D);
			return 4;
		case 0xab:			//XOR A, E
			xor8(E);
			return 4;
		case 0xac:			//XOR A, H
			xor8(H);
			return 4;
		case 0xad:			//XOR A, L
			xor8(L);
			return 4;
		case 0xae:			//XOR A, [HL]
			xor8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xee:			//XOR A, #
			xor8(p1);
			return 8;

		/* compare A with n (A - n). results are thrown away */
		case 0xbf:			//CP A, A
			compare8(A);
			return 4;
		case 0xb8:			//CP A, B
			compare8(B);
			return 4;
		case 0xb9:			//CP A, C
			compare8(C);
			return 4;
		case 0xba:			//CP A, D
			compare8(D);
			return 4;
		case 0xbb:			//CP A, E
			compare8(E);
			return 4;
		case 0xbc:			//CP A, H
			compare8(H);
			return 4;
		case 0xbd:			//CP A, L
			compare8(L);
			return 4;
		case 0xbe:			//CP A, [HL]
			compare8(mem->read(getReg16(HL)));
			programCounter--;
			return 8;
		case 0xfe:			//CP A, #
			compare8(p1);
			return 8;

		/* increment registers */
		case 0x3c:			//INC A
			increment8(A);
			return 4;
		case 0x04:			//INC B
			increment8(B);
			return 4;
		case 0x0c:			//INC C
			increment8(C);
			return 4;
		case 0x14:			//INC D
			increment8(D);
			return 4;
		case 0x1c:			//INC E
			increment8(E);
			return 4;
		case 0x24:			//INC H
			increment8(H);
			return 4;
		case 0x2c:			//INC L
			increment8(L);
			return 4;
		case 0x34:			//INC [HL]
			increment8(mem->read(getReg16(HL)));
			return 12;

		/* decrement registers */
		case 0x3d:			//DEC A
			decrement8(A);
			return 4;
		case 0x05:			//DEC B
			decrement8(B);
			return 4;
		case 0x0d:			//DEC C
			decrement8(C);
			return 4;
		case 0x15:			//DEC D
			decrement8(D);
			return 4;
		case 0x1d:			//DEC E
			decrement8(E);
			return 4;
		case 0x25:			//DEC H
			decrement8(H);
			return 4;
		case 0x2d:			//DEC L
			decrement8(L);
			return 4;
		case 0x35:			//DEC [HL]
			decrement8(mem->read(getReg16(HL)));
			return 12;

		/* 16-bit arithmetic */

		/* add register to HL */
		case 0x09:			//ADD HL, BC
			add16(BC);
			return 8;
		case 0x19:			//ADD HL, DE
			add16(DE);
			return 8;
		case 0x29:			//ADD HL, HL
			add16(HL);
			return 8;
		case 0x39:			//ADD HL, SP
			add16(SP);
			return 8;

		/* add n to stack pointer (SP) */
		case 0xe8:			//ADD SP, #
			programCounter++;
			ZF = RESET;
			NF = RESET;
			((int)(getReg16(SP) & 0xfff) + (char)p1 > 0xfff) ? HF = SET_HF : HF = RESET;
			((int)(getReg16(SP)) + (char)p1 > 0xffff) ? CF = SET_CF : CF = RESET;
			modReg16(SP, (char)p1);
			return 16;

		/* increment 16 bit registers */
		case 0x03:			//INC BC
			increment16(BC);
			return 8;
		case 0x13:			//INC DE
			increment16(DE);
			return 8;
		case 0x23:			//INC HL
			increment16(HL);
			return 8;
		case 0x33:			//INC SP
			increment16(SP);
			return 8;

		/* decrement 16 bit registers */
		case 0x0b:			//DEC BC
			decrement16(BC);
			return 8;
		case 0x1b:			//DEC DE
			decrement16(DE);
			return 8;
		case 0x2b:			//DEC HL
			decrement16(HL);
			return 8;
		case 0x3b:			//DEC SP
			decrement16(SP);
			return 8;

		/* two byte opcodes */
		/* moved to procOp2.cpp */
		case 0xcb:
			return procOpcode_cb(p1, p2);

		/* decimal adjust register A */
		case 0x27:			//DAA
			{
				int h = reg[A] >> 4;
				int l = reg[A] & 0x0F;
				bool _FC = true;
				if (NF) 
				{
					if (CF) 
					{
						if (HF)
							reg[A] += 0x9A;
						else 
							reg[A] += 0xA0;
					}
					else 
					{
						_FC = false;
						if (HF)
							reg[A] += 0xFA;
						else
							reg[A] += 0x00;
					}
				}
				else if (CF)
				{
					if (HF || l > 9)
						reg[A] += 0x66;
					else
						reg[A] += 0x60;
				}
				else if (HF)
				{
					if (h > 9)
						reg[A] += 0x66;
					else 
					{
						reg[A] += 0x06;
						_FC = false;
					}
				}
				else if (l > 9)
				{
					if (h < 9)
					{
						_FC = false;
						reg[A] += 0x06;
					} 
					else 
						reg[A] += 0x66;
				}
				else if (h > 9)
					reg[A] += 0x60;
				else 
					_FC = false;

				(reg[A] == 0) ? ZF = SET_ZF : ZF = RESET;
				HF = RESET;
				(_FC) ? CF = SET_CF : CF = RESET;
			}
			return 4;

		/* compliment register A (flip all bits) */
		case 0x2f:			//CPL
			reg[A] = reg[A] ^ 0xff;
			NF = SET_NF;
			HF = SET_HF;
			return 4;

		/* compliment carry flag (flip it) */
		case 0x3f:			//CCF
			(CF) ? CF = RESET : CF = SET_CF;
			NF = RESET;
			HF = RESET;
			return 4;

		/* set carry flag */
		case 0x37:			//SCF
			CF = SET_CF;
			NF = RESET;
			HF = RESET;
			return 4;
	
		/* no operation */
		case 0x00:			//NOP
		case 0xD3:
		case 0xDB:
		case 0xDD:
		case 0xE3:
		case 0xE4:
		case 0xEB:
		case 0xEC:
		case 0xF4:
		case 0xFC:
			return 4;

		//debugger break point opcode.
		//unused by the gameboy, repurposed for the emulator
		case 0xFD:
			return 4;
			//{
			//	Breakpoint* bp = gb->getBpm()->getBreakpoint(programCounter-1);
			//	if(!bp || bp->opcode == BREAKPOINT_CODE)
			//	{
			//		//error: invalid breakpoint code
			//		return 4;
			//	}

			//	if(!gb->debugIsBroken())
			//	{
			//		if(gb->getBpm()->checkBreakpoint(programCounter-1, 0))
			//		{
			//			gb->debugBreak();
			//			programCounter--;
			//			return 0;
			//		}
			//		else
			//		{
			//			//condition failed; don't break
			//			//re-process with the original opcode
			//			op = bp->opcode;
			//			continue;
			//		}
			//	}
			//	else
			//	{
			//		//already broken; ignore breakpoints
			//		op = bp->opcode;
			//		continue;
			//	}
			//	return 4;
			//}

		/* halt */
		case 0x76:			//HALT
			/* power down cpu */
			cpuActive = false;
			interruptMasterEnable = true;
			interrupts();
			return 4;

		/* stop */
		case 0x10:			//STOP
			programCounter++;
			switch(p1)
			{
			case 0x00:
				/* halt cpu and display until button press */
				cpuActive = false;
				lcd->toggleActive();
			}
			return 4;

		/* disable interrupts */
		case 0xf3:			//DI
			/* interrupts diabled after this operation */
			interruptMasterEnable = false;
			procOpcode();
			return 4;

		/* enable interrupts */
		case 0xfb:			//EI
			/* interrupts enabled after this operation */
			procOpcode();
			interruptMasterEnable = true;
			interrupts();
			return 4;

		/* rotates and shifts */

		/* rotate A left 1 bit. old bit 7 to carry flag */
		case 0x07:			//RLCA
			rlc(A);
			return 4;
		/* rotate A left 1 bit through carry flag */
		case 0x17:			//RLA
			rl(A);
			return 4;
		/* rotate A right. old bit 0 to carry */
		case 0x0f:			//RRCA
			rrc(A);
			return 4;
		/* rotate A right through carry flag */
		case 0x1f:			//RRA
			rr(A);
			return 4;

		/* jumps */

		/* jump to address nn */
		case 0xc3:			//JP ##
			jump(1, p2, p1);
			return 12;

		/* jump if Z is reset */
		case 0xc2:			//JP NZ, ##
			jump(ZF == 0, p2, p1);
			return 12;

		/* jump if Z is set */
		case 0xca:			//JP Z, ##
			jump(ZF > 0, p2, p1);
			return 12;

		/* jump if C is reset */
		case 0xd2:			//JP NC, ##
			jump(!(CF > 0), p2, p1);
			return 12;

		/* jump if C is set */
		case 0xda:			//JP C, ##
			jump(CF > 0, p2, p1);
			return 12;

		/* jump to address in HL */
		case 0xe9:			//JP [HL]
			programCounter = getReg16(HL);
			//movReg16(PC, getReg16(HL));
			return 4;

		/* add n to current address and jump to it */
		case 0x18:			//JP PC + #
			programCounter++;
			programCounter += (char)p1;
			//modReg16(PC, (char)p1);
			return 8;

		/* add n to address if Z is reset */
		case 0x20:			//JR NZ, PC + #
			jumpAddN(!(ZF > 0), p1);
			return 8;

		/* add n to address if Z is set */
		case 0x28:			//JR Z, PC + #
			jumpAddN(ZF > 0, p1);
			return 8;

		/* add n to address if C is reset */
		case 0x30:			//JR NC, PC + #
			jumpAddN(!(CF > 0), p1);
			return 8;

		/* add n to address if C is set */
		case 0x38:			//JR NC, PC + #
			jumpAddN(CF > 0, p1);
			return 8;

		/* push address of next instr onto stack. jump to addr nn */
		case 0xcd:			//CALL ##
			call(1, p2, p1);
			return 12;

		/* call address n if Z is reset */
		case 0xc4:			//CALL NZ, ##
			call(!(ZF > 0), p2, p1);
			return 12;

		/* call address n if Z is set */
		case 0xcc:			//CALL Z, ##
			call(ZF > 0, p2, p1);
			return 12;

		/* call address n if C is reset */
		case 0xd4:			//CALL NC, ##
			call(!(CF > 0), p2, p1);
			return 12;

		/* call address n if C is set */
		case 0xdc:			//CALL C, ##
			call(CF > 0, p2, p1);
			return 12;

		/* restart: push addr to stack, jump to addr 0000+n */
		case 0xc7:			//RST 0x00
			restart(0);
			return 32;
		case 0xcf:			//RST 0x08
			restart(8);
			return 32;
		case 0xd7:			//RST 0x10
			restart(0x10);
			return 32;
		case 0xdf:			//RST 0x18
			restart(0x18);
			return 32;
		case 0xe7:			//RST 0x20
			restart(0x20);
			return 32;
		case 0xef:			//RST 0x28
			restart(0x28);
			return 32;
		case 0xf7:			//RST 0x30
			restart(0x30);
			return 32;
		case 0xff:			//RST 0x38
			restart(0x38);
			return 32;

		/* pop two bytes from stack & jump to that address */
		case 0xc9:			//RET
			ret(1);
			return 8;

		/* return if Z reset, Z set, C reset, C set */
		case 0xc0:			//RET NZ
			ret(!(ZF > 0));
			return 8;
		case 0xc8:			//RET Z
			ret(ZF > 0);
			return 8;
		case 0xd0:			//RET NC
			ret(!(CF > 0));
			return 8;
		case 0xd8:			//RET C
			ret(CF > 0);
			return 8;

		/* return and enable interrupts */
		case 0xd9:			//RETI
			ret(1);
			interruptMasterEnable = true;
			interrupts();
			return 8;
		default:
			std::cout << "Unknown opcde: " << op << "\n";
			return 0;
		};
	}
	while(true);
}

#endif