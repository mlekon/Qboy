#ifndef GB_OP
#define GB_OP

#include "enum.h"
#include "Z80.h"

/* 
TODO:	separate similar instructions into their own functions
using the lower half of the opcode byte to identify them
*/

void Z80::proc_opcode()
{
	cpu_clocks += proc_op();
}

/* executes a basic CPU operation based on the opcode in
parameter "op". up to two parameters may be given for an
opcode, each one byte in length. for two-byte opcodes, the
first parameter byte is used as the second half of the 
total opcode. returns the number of parameters used by an
opcode + 1, to indicate the necessary IP increment */
int Z80::proc_op()
{
	if(!cpu_active)
	{
		return 4;
	}

	// c9 1a 28 fa 6b 29 ea fa d7 e1 5f af 57 19 2a 66 6f e9
	byte op = mem->read(getReg16(PC));
	byte p1 = mem->read(getReg16(PC)+1); //high
	byte p2 = mem->read(getReg16(PC)+2); //low

	byte ta = reg[A];

#if LOG_OP == 1
		char a = op;
		logger.write(&a, 1);
#endif

	incPC();
	
	//instructions_executed++;
	//if(instructions_executed == 0x2b3b9) {
	//	int x = 1;
	//}

	switch(op)
	{
	/* LD instructions */
	case 0x06:
		load8(B, p1);
		return 8;
	case 0x0e:
		load8(C, p1);
		return 8;
	case 0x16:
		load8(D, p1);
		return 8;
	case 0x1e:
		load8(E, p1);
		return 8;
	case 0x26:
		load8(H, p1);
		return 8;
	case 0x2e:
		load8(L, p1);
		return 8;

	case 0x7f:
		load8(A, A);
		return 4;
	case 0x78:
		load8(A, B);
		return 4;
	case 0x79:
		load8(A, C);
		return 4;
	case 0x7a:
		load8(A, D);
		return 4;
	case 0x7b:
		load8(A, E);
		return 4;
	case 0x7c:
		load8(A, H);
		return 4;
	case 0x7d:
		load8(A, L);
		return 4;
	case 0x7e:
		load8(A, mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0x40:
		load8(B, B);
		return 4;
	case 0x41:
		load8(B, C);
		return 4;
	case 0x42:
		load8(B, D);
		return 4;
	case 0x43:
		load8(B, E);
		return 4;
	case 0x44:
		load8(B, H);
		return 4;
	case 0x45:
		load8(B, L);
		return 4;
	case 0x46:
		load8(B, mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0x48:
		load8(C, B);
		return 4;
	case 0x49:
		load8(C, C);
		return 4;
	case 0x4a:
		load8(C, D);
		return 4;
	case 0x4b:
		load8(C, E);
		return 4;
	case 0x4c:
		load8(C, H);
		return 4;
	case 0x4d:
		load8(C, L);
		return 4;
	case 0x4e:
		load8(C, mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0x50:
		load8(D, B);
		return 4;
	case 0x51:
		load8(D, C);
		return 4;
	case 0x52:
		load8(D, D);
		return 4;
	case 0x53:
		load8(D, E);
		return 4;
	case 0x54:
		load8(D, H);
		return 4;
	case 0x56:
		load8(D, mem->read(getReg16(HL)));
		decPC();
		return 4;
	case 0x55:
		load8(D, L);
		return 8;
	case 0x58:
		load8(E, B);
		return 4;
	case 0x59:
		load8(E, C);
		return 4;
	case 0x5a:
		load8(E, D);
		return 4;
	case 0x5b:
		load8(E, E);
		return 4;
	case 0x5c:
		load8(E, H);
		return 4;
	case 0x5d:
		load8(E, L);
		return 4;
	case 0x5e:
		load8(E, mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0x60:
		load8(H, B);
		return 4;
	case 0x61:
		load8(H, C);
		return 4;
	case 0x62:
		load8(H, D);
		return 4;
	case 0x63:
		load8(H, E);
		return 4;
	case 0x64:
		load8(H, H);
		return 4;
	case 0x65:
		load8(H, L);
		return 4;
	case 0x66:
		load8(H, mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0x68:
		load8(L, B);
		return 4;
	case 0x69:
		load8(L, C);
		return 4;
	case 0x6a:
		load8(L, D);
		return 4;
	case 0x6b:
		load8(L, E);
		return 4;
	case 0x6c:
		load8(L, H);
		return 4;
	case 0x6d:
		load8(L, L);
		return 4;
	case 0x6e:
		load8(L, mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0x70:
		mem->write(getReg16(HL), reg[B]);
		return 8;
	case 0x71:
		mem->write(getReg16(HL), reg[C]);
		return 8;
	case 0x72:
		mem->write(getReg16(HL), reg[D]);
		return 8;
	case 0x73:
		mem->write(getReg16(HL), reg[E]);
		return 8;
	case 0x74:
		mem->write(getReg16(HL), reg[H]);
		return 8;
	case 0x75:
		mem->write(getReg16(HL), reg[L]);
		return 8;
	case 0x36:
		mem->write(getReg16(HL), p1);
		incPC();
		return 12;

	case 0x0a:
		load8(A, mem->read(getReg16(BC)));
		decPC();
		return 8;
	case 0x1a:
		load8(A, mem->read(getReg16(DE)));
		decPC();
		return 8;
	case 0xfa:
		load8(A, mem->read((p2 << 8) + p1));
		incPC();
		return 16;
	case 0x3e:
		load8(A, p1);
		return 8;
	case 0x02:
		mem->write(getReg16(BC), reg[A]);
		return 8;
	case 0x12:
		mem->write(getReg16(DE), reg[A]);
		return 8;
	case 0x77:
		mem->write(getReg16(HL), reg[A]);
		return 8;
	case 0xea:
		mem->write((p2 << 8) + p1, reg[A]);
		incPC(2);
		return 16;

	//i'm a goddamn moron opcodes
	case 0x47:
		load8(B, A);
		return 4;
	case 0x4f:
		load8(C, A);
		return 4;
	case 0x57:
		load8(D, A);
		return 4;
	case 0x5f:
		load8(E, A);
		return 4;
	case 0x67:
		load8(H, A);
		return 4;
	case 0x6f:
		load8(L, A);
		return 4;

	case 0xf2:
		load8(A, mem->read(0xff00 + reg[C]));
		decPC();
		return 8;
	case 0xe2:
		mem->write(0xff00 + reg[C], reg[A]);
		return 8;
	case 0x3a:
		load8(A, mem->read(getReg16(HL)));
		decPC();
		decrement16(HL);
		return 8;
	case 0x32:
		mem->write(getReg16(HL), reg[A]);
		decrement16(HL);
		return 8;
	case 0x2a:
		load8(A, mem->read(getReg16(HL)));
		decPC();
		increment16(HL);
		return 8;
	case 0x22:
		mem->write(getReg16(HL), reg[A]);
		increment16(HL);
		return 8;
	case 0xe0:
		mem->write(0xff00 + p1, reg[A]);
		incPC();
		return 12;
	case 0xf0:		//LDH A,(n)
		load8(A, mem->read(0xff00 + p1));
		return 12;

	case 0x01:
		load16(BC, p2, p1);
		return 12;
	case 0x11:
		load16(DE, p2, p1);
		return 12;
	case 0x21:
		load16(HL, p2, p1);
		return 12;
	case 0x31:		//LD n, nn
		load16(SP, p2, p1);
		return 12;

	case 0xf9:		//LD SP, HL
		load16(SP, getReg16(HL));
		decPC(2);
		return 8;

	case 0xf8:		//LDHL SP, n
		load16(HL, (short)getReg16(SP) + (char)p1);
		resetZF();
		resetN();
		((int)((getReg16(SP)) + ((char)p1)) > 0xffff ||
		((int)((getReg16(SP)) + ((char)p1)) < 0)) ? setC() : resetC();
		(((int)(getReg16(SP) & 0xff) + (char)p1) < p1) ? setH() : resetH();
		decPC();
		return 12;

	case 0x08:		//LD (nn),SP
		mem->write((p2 << 8) + p1, getReg16(SP));
		incPC(2);
		return 20;

	/* push to stack */
	case 0xf5:
		push(AF);
		return 16;
	case 0xc5:
		push(BC);
		return 16;
	case 0xd5:
		push(DE);
		return 16;
	case 0xe5:
		push(HL);
		return 16;

	/* pop from stack */
	case 0xf1:
		pop(AF);
		return 12;
	case 0xc1:
		pop(BC);
		return 12;
	case 0xd1:
		pop(DE);
		return 12;
	case 0xe1:
		pop(HL);
		return 12;

	/* ADD A, n */
	case 0x87:
		add8(A);
		return 4;

	/* add to reg[A] */
	case 0x80:
		add8(B);
		return 4;
	case 0x81:
		add8(C);
		return 4;
	case 0x82:
		add8(D);
		return 4;
	case 0x83:
		add8(E);
		return 4;
	case 0x84:
		add8(H);
		return 4;
	case 0x85:
		add8(L);
		return 1;
	case 0x86:
		add8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xc6:
		add8(p1);
		return 8;

	/* add to reg[A], add carry flag */
	case 0x8f:
		add_plus_carry8(A);
		return 4;
	case 0x88:
		add_plus_carry8(B);
		return 4;
	case 0x89:
		add_plus_carry8(C);
		return 4;
	case 0x8a:
		add_plus_carry8(D);
		return 4;
	case 0x8b:
		add_plus_carry8(E);
		return 4;
	case 0x8c:
		add_plus_carry8(H);
		return 4;
	case 0x8d:
		add_plus_carry8(L);
		return 4;
	case 0x8e:
		add_plus_carry8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xce:
		add_plus_carry8(p1);
		return 8;

	/* subtract from reg[A] */
	case 0x97:
		sub8(A);
		return 4;
	case 0x90:
		sub8(B);
		return 4;
	case 0x91:
		sub8(C);
		return 4;
	case 0x92:
		sub8(D);
		return 4;
	case 0x93:
		sub8(E);
		return 4;
	case 0x94:
		sub8(H);
		return 4;
	case 0x95:
		sub8(L);
		return 4;
	case 0x96:
		sub8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xd6:
		sub8(p1);
		return 8;

	/* subtract from reg[A], add carry flag */
	case 0x9f:
		sub_plus_carry8(A);
		return 4;
	case 0x98:
		sub_plus_carry8(B);
		return 4;
	case 0x99:
		sub_plus_carry8(C);
		return 4;
	case 0x9a:
		sub_plus_carry8(D);
		return 4;
	case 0x9b:
		sub_plus_carry8(E);
		return 4;
	case 0x9c:
		sub_plus_carry8(H);
		return 4;
	case 0x9d:
		sub_plus_carry8(L);
		return 4;
	case 0x9e:
		sub_plus_carry8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xde:
		sub_plus_carry8(p1);
		return 8;

	/* logical AND with register A, result in A */
	case 0xa7:
		and8(A);
		return 4;
	case 0xa0:
		and8(B);
		return 4;
	case 0xa1:
		and8(C);
		return 4;
	case 0xa2:
		and8(D);
		return 4;
	case 0xa3:
		and8(E);
		return 4;
	case 0xa4:
		and8(H);
		return 4;
	case 0xa5:
		and8(L);
		return 4;
	case 0xa6:
		and8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xe6:
		and8(p1);
		return 8;

	/* logical OR with register A, result in A */
	case 0xb7:
		or8(A);
		return 4;
	case 0xb0:
		or8(B);
		return 4;
	case 0xb1:
		or8(C);
		return 4;
	case 0xb2:
		or8(D);
		return 4;
	case 0xb3:
		or8(E);
		return 4;
	case 0xb4:
		or8(H);
		return 4;
	case 0xb5:
		or8(L);
		return 4;
	case 0xb6:
		or8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xf6:
		or8(p1);
		return 8;

	/* logical XOR with register A, result in A */
	case 0xaf:
		xor8(A);
		return 4;
	case 0xa8:
		xor8(B);
		return 4;
	case 0xa9:
		xor8(C);
		return 4;
	case 0xaa:
		xor8(D);
		return 4;
	case 0xab:
		xor8(E);
		return 4;
	case 0xac:
		xor8(H);
		return 4;
	case 0xad:
		xor8(L);
		return 4;
	case 0xae:
		xor8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xee:
		xor8(p1);
		return 8;

	/* compare A with n (A - n). results are thrown away */
	case 0xbf:
		compare8(A);
		return 4;
	case 0xb8:
		compare8(B);
		return 4;
	case 0xb9:
		compare8(C);
		return 4;
	case 0xba:
		compare8(D);
		return 4;
	case 0xbb:
		compare8(E);
		return 4;
	case 0xbc:
		compare8(H);
		return 4;
	case 0xbd:
		compare8(L);
		return 4;
	case 0xbe:
		compare8(mem->read(getReg16(HL)));
		decPC();
		return 8;
	case 0xfe:
		compare8(p1);
		return 8;

	/* increment registers */
	case 0x3c:
		increment8(A);
		return 4;
	case 0x04:
		increment8(B);
		return 4;
	case 0x0c:
		increment8(C);
		return 4;
	case 0x14:
		increment8(D);
		return 4;
	case 0x1c:
		increment8(E);
		return 4;
	case 0x24:
		increment8(H);
		return 4;
	case 0x2c:
		increment8(L);
		return 4;
	case 0x34:
		increment8(mem->read(getReg16(HL)));
		return 12;

	/* decrement registers */
	case 0x3d:
		decrement8(A);
		return 4;
	case 0x05:
		decrement8(B);
		return 4;
	case 0x0d:
		decrement8(C);
		return 4;
	case 0x15:
		decrement8(D);
		return 4;
	case 0x1d:
		decrement8(E);
		return 4;
	case 0x25:
		decrement8(H);
		return 4;
	case 0x2d:
		decrement8(L);
		return 4;
	case 0x35:
		decrement8(mem->read(getReg16(HL)));
		return 12;

	/* 16-bit arithmetic */

	/* add register to HL */
	case 0x09:
		add16(BC);
		return 8;
	case 0x19:
		add16(DE);
		return 8;
	case 0x29:
		add16(HL);
		return 8;
	case 0x39:
		add16(SP);
		return 8;

	/* add n to stack pointer (SP) */
	case 0xe8:
		incPC();
		resetZF();
		resetN();
		((getReg16(SP) & 0xff) + (char)p1 > 0xff) ? setH() : resetH();
		((int)(getReg16(SP) + (char)p1) > 0xffff) ? setC() : resetC();
		modReg16(SP, (char)p1);
		return 16;

	/* increment 16 bit registers */
	case 0x03:
		increment16(BC);
		return 8;
	case 0x13:
		increment16(DE);
		return 8;
	case 0x23:
		increment16(HL);
		return 8;
	case 0x33:
		increment16(SP);
		return 8;

	/* decrement 16 bit registers */
	case 0x0b:
		decrement16(BC);
		return 8;
	case 0x1b:
		decrement16(DE);
		return 8;
	case 0x2b:
		decrement16(HL);
		return 8;
	case 0x3b:
		decrement16(SP);
		return 8;

	/* two byte opcodes */
	/* moved to proc_op2.cpp for organizational purposes */
	case 0xcb:
		return proc_opcode_cb(p1, p2);

	/* decimal adjust register A */
	case 0x27:
		/* what the hell do i do here? */
		if(reg[A] == 0) setZF(); else resetZF();
		resetH();
		return 4;

	/* compliment register A (flip all bits) */
	case 0x2f:
		reg[A] = reg[A] ^ 0xff;
		setN();
		setH();
		return 4;

	/* compliment carry flag (flip it) */
	case 0x3f:
		(getC()) ? resetC() : setC();
		resetN();
		resetH();
		return 4;

	/* set carry flag */
	case 0x37:
		setC();
		resetN();
		resetH();
		return 4;
	
	/* no operation */
	case 0x00:
	case 0xD3:
    case 0xDB:
    case 0xDD:
    case 0xE3:
    case 0xE4:
    case 0xEB:
    case 0xEC:
    case 0xF4:
    case 0xFC:
    case 0xFD:
		return 4;

	/* halt */
	case 0x76:
		/* power down cpu */
		cpu_active = false;
		IME(true);
		return 4;

	/* stop */
	case 0x10:
		incPC();
		switch(p1)
		{
		case 0x00:
			/* halt cpu and display until button press */
			cpu_active = false;
			lcd->toggle_active();
		default:;
		}
		return 4;

	/* disable interrupts */
	case 0xf3:
		/* interrupts diabled after this operation */
		IME(false);
		return 4;

	/* enable interrupts */
	case 0xfb:
		/* interrupts enabled after this operation */
		IME(true);
		return 4;

	/* rotates and shifts */

	/* rotate A left 1 bit. old bit 7 to carry flag */
	case 0x07:
		rlc(A);
		return 4;
	/* rotate A left 1 bit through carry flag */
	case 0x17:
		rl(A);
		return 4;
	/* rotate A right. old bit 0 to carry */
	case 0x0f:
		rrc(A);
		return 4;
	/* rotate A right through carry flag */
	case 0x1f:
		rr(A);
		return 4;

	/* jumps */

	/* jump to address nn */
	case 0xc3:
		incPC(2);
		movReg16(PC, p1, p2);
		return 12;

	/* jump if Z is reset */
	case 0xc2:
		jump(!getZF(), p1, p2);
		return 12;

	/* jump if Z is set */
	case 0xca:
		jump(getZF(), p1, p2);
		return 12;

	/* jump if C is reset */
	case 0xd2:
		jump(!getC(), p1, p2);
		return 12;

	/* jump if C is set */
	case 0xda:
		jump(getC(), p1, p2);
		return 12;

	/* jump to address in HL */
	case 0xe9:
		movReg16(PC, getReg16(HL));
		return 4;

	/* add n to current address and jump to it */
	case 0x18:
		incPC();
		modReg16(PC, (char)p1);
		return 8;

	/* add n to address if Z is reset */
	case 0x20:
		jump_add_n(!getZF(), p1);
		return 8;

	/* add n to address if Z is set */
	case 0x28:
		jump_add_n(getZF(), p1);
		return 8;

	/* add n to address if C is reset */
	case 0x30:
		jump_add_n(!getC(), p1);
		return 8;

	/* add n to address if C is set */
	case 0x38:
		jump_add_n(getC(), p1);
		return 8;

	/* push address of next instr onto stack. jump to addr nn */
	case 0xcd:
		call(1, p2, p1);
		return 12;

	/* call address n if Z is reset */
	case 0xc4:
		call(!getZF(), p2, p1);
		return 12;

	/* call address n if Z is set */
	case 0xcc:
		call(getZF(), p2, p1);
		return 12;

	/* call address n if C is reset */
	case 0xd4:
		call(!getC(), p2, p1);
		return 12;

	/* call address n if C is set */
	case 0xdc:
		call(getC(), p2, p1);
		return 12;

	/* restart: push addr to stack, jump to addr 0000+n */
	case 0xc7:
		restart(0);
		return 32;
	case 0xcf:
		restart(8);
		return 32;
	case 0xd7:
		restart(0x10);
		return 32;
	case 0xdf:
		restart(0x18);
		return 32;
	case 0xe7:
		restart(0x20);
		return 32;
	case 0xef:
		restart(0x28);
		return 32;
	case 0xf7:
		restart(0x30);
		return 32;
	case 0xff:
		restart(0x38);
		return 32;

	/* pop two bytes from stack & jump to that address */
	case 0xc9:
		ret(1);
		return 8;

	/* return if Z reset, Z set, C reset, C set */
	case 0xc0:
		ret(!getZF());
		return 8;
	case 0xc8:
		ret(getZF());
		return 8;
	case 0xd0:
		ret(!getC());
		return 8;
	case 0xd8:
		ret(getC());
		return 8;

	/* return and enable interrupts */
	case 0xd9:
		movReg16(PC, mem->pop_stack16());
		IME(true);
		return 8;
	default:
		return 0;
	};
}

#endif