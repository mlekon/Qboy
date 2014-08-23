#ifndef GB_OP_CB
#define GB_OP_CB

#include "enum.h"
#include "Z80.h"

/////////////////////////////////
//executes 2-byte opcode instructions
//byte 1 is 0xCB
int Z80::procOpcode_cb(byte op, byte p2)
{
	programCounter++;

	switch(op)
	{
	/* swap upper and lower nibbles of a register */
	case 0x37:			//SWAP A
		swap(A);
		return 8;
	case 0x30:
		swap(B);
		return 8;
	case 0x31:
		swap(C);
		return 8;
	case 0x32:
		swap(D);
		return 8;
	case 0x33:
		swap(E);
		return 8;
	case 0x34:
		swap(H);
		return 8;
	case 0x35:
		swap(L);
		return 8;
	case 0x36:			//SWAP [HL]
		swap(mem->read(getReg16(HL)));
		return 16;

	/* rotate registers left. old bit 7 to carry */
	case 0x07:			//RLC A
		rlc(A);
		return 8;
	case 0x00:
		rlc(B);
		return 8;
	case 0x01:
		rlc(C);
		return 8;
	case 0x02:
		rlc(D);
		return 8;
	case 0x03:
		rlc(E);
		return 8;
	case 0x04:
		rlc(H);
		return 8;
	case 0x05:
		rlc(L);
		return 8;
	case 0x06:			//RLC [HL]
		rlc(mem->read(getReg16(HL)));
		return 16;

	/* rotate register left through carry */
	case 0x17:			//RL A
		rl(A);
		return 8;
	case 0x10:
		rl(B);
		return 8;
	case 0x11:
		rl(C);
		return 8;;
	case 0x12:
		rl(D);
		return 8;
	case 0x13:
		rl(E);
		return 8;
	case 0x14:
		rl(H);
		return 8;
	case 0x15:
		rl(L);
		return 8;
	case 0x16:			//RL [HL]
		rl(mem->read(getReg16(HL)));
		return 16;

	/* rotate register right. old bit 0 to carry flag */
	case 0x0f:			//RRC A
		rrc(A);
		return 8;
	case 0x08:
		rrc(B);
		return 8;
	case 0x09:
		rrc(C);
		return 8;
	case 0x0A:
		rrc(D);
		return 8;
	case 0x0b:
		rrc(E);
		return 8;
	case 0x0c:
		rrc(H);
		return 8;
	case 0x0d:
		rrc(L);
		return 8;
	case 0x0e:			//RRC [HL]
		rrc(mem->read(getReg16(HL)));
		return 16;

	/* rotate register right through carry flag */
	case 0x1f:
		rr(A);			//RR A
		return 8;
	case 0x18:
		rr(B);
		return 8;
	case 0x19:
		rr(C);
		return 8;
	case 0x1a:
		rr(D);
		return 8;
	case 0x1b:
		rr(E);
		return 8;
	case 0x1c:
		rr(H);
		return 8;
	case 0x1d:
		rr(L);
		return 8;
	case 0x1e:			//RR [HL]
		rr(mem->read(getReg16(HL)));
		return 16;

	/* shift registers left into carry. lsb = 0 */
	case 0x27:			//SLA A
		sla(A);
		return 8;
	case 0x20:
		sla(B);
		return 8;
	case 0x21:
		sla(C);
		return 8;
	case 0x22:
		sla(D);
		return 8;
	case 0x23:
		sla(E);
		return 8;
	case 0x24:
		sla(H);
		return 8;
	case 0x25:
		sla(L);
		return 8;
	case 0x26:			//SLA [HL]
		sla(mem->read(getReg16(HL)));
		return 16;
	
	/* signed right shift into carry. msb doesn't change */
	case 0x2f:			//SRA A
		sra(A);
		return 8;
	case 0x28:
		sra(B);
		return 8;
	case 0x29:
		sra(C);
		return 8;
	case 0x2a:
		sra(D);
		return 8;;
	case 0x2b:
		sra(E);
		return 8;
	case 0x2c:
		sra(H);
		return 8;
	case 0x2d:
		sra(L);
		return 8;
	case 0x2e:			//SRA [HL]
		sra(mem->read(getReg16(HL)));
		return 16;

	/* unsigned right shift into carry. msb = 0 */
	case 0x3f:			//SRL A
		srl(A);
		return 8; 
	case 0x38:
		srl(B);
		return 8; 
	case 0x39:
		srl(C);
		return 8; 
	case 0x3a:
		srl(D);
		return 8; 
	case 0x3b:
		srl(E);
		return 8; 
	case 0x3c:
		srl(H);
		return 8; 
	case 0x3d:
		srl(L);
		return 8; 
	case 0x3e:			//SRL [HL]
		srl(mem->read(getReg16(HL)));
		return 16;

	/* test bit b in register- ZF is set if bit is set */
	case 0x47:			//BIT 0, A
		testBit(A, 0);
		return 8;
	case 0x40:
		testBit(B, 0);
		return 8;
	case 0x41:
		testBit(C, 0);
		return 8;
	case 0x42:
		testBit(D, 0);
		return 8;
	case 0x43:
		testBit(E, 0);
		return 8;
	case 0x44:
		testBit(H, 0);
		return 8;
	case 0x45:			//BIT 0, L
		testBit(L, 0);
		return 8;
	case 0x46:			//BIT 0, [HL]
		(mem->read(getReg16(HL)) & (1 << 0)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x4f:			//BIT 1, A
		testBit(A, 1);
		return 8;
	case 0x48:
		testBit(B, 1);
		return 8;
	case 0x49:
		testBit(C, 1);
		return 8;
	case 0x4a:
		testBit(D, 1);
		return 8;
	case 0x4b:
		testBit(E, 1);
		return 8;
	case 0x4c:
		testBit(H, 1);
		return 8;
	case 0x4d:			//BIT 1, L
		testBit(L, 1);
		return 8;
	case 0x4e:			//BIT 1, [HL]
		(mem->read(getReg16(HL)) & (1 << 1)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x57:			//BIT 2, A
		testBit(A, 2);
		return 8;
	case 0x50:
		testBit(B, 2);
		return 8;
	case 0x51:
		testBit(C, 2);
		return 8;
	case 0x52:
		testBit(D, 2);
		return 8;
	case 0x53:
		testBit(E, 2);
		return 8;
	case 0x54:
		testBit(H, 2);
		return 8;
	case 0x55:			//BIT 2, L
		testBit(L, 2);
		return 8;
	case 0x56:			//BIT 2, [HL]
		(mem->read(getReg16(HL)) & (1 << 2)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x5f:			//BIT 3, A
		testBit(A, 3);
		return 8;
	case 0x58:
		testBit(B, 3);
		return 8;
	case 0x59:
		testBit(C, 3);
		return 8;
	case 0x5a:
		testBit(D, 3);
		return 8;
	case 0x5b:
		testBit(E, 3);
		return 8;
	case 0x5c:
		testBit(H, 3);
		return 8;
	case 0x5d:			//BIT 3, L
		testBit(L, 3);
		return 8;
	case 0x5e:			//BIT 3, [HL]
		(mem->read(getReg16(HL)) & (1 << 3)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x67:			//BIT 4, A
		testBit(A, 4);
		return 8;
	case 0x60:
		testBit(B, 4);
		return 8;
	case 0x61:
		testBit(C, 4);
		return 8;
	case 0x62:
		testBit(D, 4);
		return 8;
	case 0x63:
		testBit(E, 4);
		return 8;
	case 0x64:
		testBit(H, 4);
		return 8;
	case 0x65:			//BIT 4, L
		testBit(L, 4);
		return 8;
	case 0x66:			//BIT 4, [HL]
		(mem->read(getReg16(HL)) & (1 << 4)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x6f:			//BIT 5, A
		testBit(A, 5);
		return 8;
	case 0x68:
		testBit(B, 5);
		return 8;
	case 0x69:
		testBit(C, 5);
		return 8;
	case 0x6a:
		testBit(D, 5);
		return 8;
	case 0x6b:
		testBit(E, 5);
		return 8;
	case 0x6c:
		testBit(H, 5);
		return 8;
	case 0x6d:			//BIT 5, L
		testBit(L, 5);
		return 8;
	case 0x6e:			//BIT 5, [HL]
		(mem->read(getReg16(HL)) & (1 << 5)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x77:			//BIT 6, A
		testBit(A, 6);
		return 8;
	case 0x70:
		testBit(B, 6);
		return 8;
	case 0x71:
		testBit(C, 6);
		return 8;
	case 0x72:
		testBit(D, 6);
		return 8;
	case 0x73:
		testBit(E, 6);
		return 8;
	case 0x74:
		testBit(H, 6);
		return 8;
	case 0x75:			//BIT 6, L
		testBit(L, 6);
		return 8;
	case 0x76:			//BIT 6, [HL]
		(mem->read(getReg16(HL)) & (1 << 6)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	case 0x7f:			//BIT 7, A
		testBit(A, 7);
		return 8;
	case 0x78:
		testBit(B, 7);
		return 8;
	case 0x79:
		testBit(C, 7);
		return 8;
	case 0x7a:
		testBit(D, 7);
		return 8;
	case 0x7b:
		testBit(E, 7);
		return 8;
	case 0x7c:
		testBit(H, 7);
		return 8;
	case 0x7d:			//BIT 7, L
		testBit(L, 7);
		return 8;
	case 0x7e:			//BIT 7, [HL]
		(mem->read(getReg16(HL)) & (1 << 7)) ? ZF = RESET : ZF = SET_ZF;
		NF = RESET;
		HF = SET_HF;
		return 16;

	/* set bit b in register r */
	case 0xc7:
		setBit(A, 0);
		return 8;
	case 0xc0:
		setBit(B, 0);
		return 8;
	case 0xc1:
		setBit(C, 0);
		return 8;
	case 0xc2:
		setBit(D, 0);
		return 8;
	case 0xc3:
		setBit(E, 0);
		return 8;
	case 0xc4:
		setBit(H, 0);
		return 8;
	case 0xc5:
		setBit(L, 0);
		return 8;
	case 0xc6:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 0));
		return 16;

	case 0xcf:
		setBit(A, 1);
		return 8;
	case 0xc8:
		setBit(B, 1);
		return 8;
	case 0xc9:
		setBit(C, 1);
		return 8;
	case 0xca:
		setBit(D, 1);
		return 8;
	case 0xcb:
		setBit(E, 1);
		return 8;
	case 0xcc:
		setBit(H, 1);
		return 8;
	case 0xcd:
		setBit(L, 1);
		return 8;
	case 0xce:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 1));
		return 16;

	case 0xd7:
		setBit(A, 2);
		return 8;
	case 0xd0:
		setBit(B, 2);
		return 8;
	case 0xd1:
		setBit(C, 2);
		return 8;
	case 0xd2:
		setBit(D, 2);
		return 8;
	case 0xd3:
		setBit(E, 2);
		return 8;
	case 0xd4:
		setBit(H, 2);
		return 8;
	case 0xd5:
		setBit(L, 2);
		return 8;
	case 0xd6:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 2));
		return 16;

	case 0xdf:
		setBit(A, 3);
		return 8;
	case 0xd8:
		setBit(B, 3);
		return 8;
	case 0xd9:
		setBit(C, 3);
		return 8;
	case 0xda:
		setBit(D, 3);
		return 8;
	case 0xdb:
		setBit(E, 3);
		return 8;
	case 0xdc:
		setBit(H, 3);
		return 8;
	case 0xdd:
		setBit(L, 3);
		return 8;
	case 0xde:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 3));
		return 16;

	case 0xe7:
		setBit(A, 4);
		return 8;
	case 0xe0:
		setBit(B, 4);
		return 8;
	case 0xe1:
		setBit(C, 4);
		return 8;
	case 0xe2:
		setBit(D, 4);
		return 8;
	case 0xe3:
		setBit(E, 4);
		return 8;
	case 0xe4:
		setBit(H, 4);
		return 8;
	case 0xe5:
		setBit(L, 4);
		return 8;
	case 0xe6:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 4));
		return 16;

	case 0xef:
		setBit(A, 5);
		return 8;
	case 0xe8:
		setBit(B, 5);
		return 8;
	case 0xe9:
		setBit(C, 5);
		return 8;
	case 0xea:
		setBit(D, 5);
		return 8;
	case 0xeb:
		setBit(E, 5);
		return 8;
	case 0xec:
		setBit(H, 5);
		return 8;
	case 0xed:
		setBit(L, 5);
		return 8;
	case 0xee:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 5));
		return 16;

	case 0xf7:
		setBit(A, 6);
		return 8;
	case 0xf0:
		setBit(B, 6);
		return 8;
	case 0xf1:
		setBit(C, 6);
		return 8;
	case 0xf2:
		setBit(D, 6);
		return 8;
	case 0xf3:
		setBit(E, 6);
		return 8;
	case 0xf4:
		setBit(H, 6);
		return 8;
	case 0xf5:
		setBit(L, 6);
		return 8;
	case 0xf6:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 6));
		return 16;

	case 0xff:
		setBit(A, 7);
		return 8;
	case 0xf8:
		setBit(B, 7);
		return 8;
	case 0xf9:
		setBit(C, 7);
		return 8;
	case 0xfa:
		setBit(D, 7);
		return 8;
	case 0xfb:
		setBit(E, 7);
		return 8;
	case 0xfc:
		setBit(H, 7);
		return 8;
	case 0xfd:
		setBit(L, 7);
		return 8;
	case 0xfe:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) | (1 << 7));
		return 16;

	/* reset bit b in register r */
	case 0x87:
		resetBit(A, 0);
		return 8;
	case 0x80:
		resetBit(B, 0);
		return 8;
	case 0x81:
		resetBit(C, 0);
		return 8;
	case 0x82:
		resetBit(D, 0);
		return 8;
	case 0x83:
		resetBit(E, 0);
		return 8;
	case 0x84:
		resetBit(H, 0);
		return 8;
	case 0x85:
		resetBit(L, 0);
		return 8;
	case 0x86:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 0)));
		return 16;

	case 0x8f:
		resetBit(A, 1);
		return 8;
	case 0x88:
		resetBit(B, 1);
		return 8;
	case 0x89:
		resetBit(C, 1);
		return 8;
	case 0x8a:
		resetBit(D, 1);
		return 8;
	case 0x8b:
		resetBit(E, 1);
		return 8;
	case 0x8c:
		resetBit(H, 1);
		return 8;
	case 0x8d:
		resetBit(L, 1);
		return 8;
	case 0x8e:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 1)));
		return 16;

	case 0x97:
		resetBit(A, 2);
		return 8;
	case 0x90:
		resetBit(B, 2);
		return 8;
	case 0x91:
		resetBit(C, 2);
		return 8;
	case 0x92:
		resetBit(D, 2);
		return 8;
	case 0x93:
		resetBit(E, 2);
		return 8;
	case 0x94:
		resetBit(H, 2);
		return 8;
	case 0x95:
		resetBit(L, 2);
		return 8;
	case 0x96:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 2)));
		return 16;

	case 0x9f:
		resetBit(A, 3);
		return 8;
	case 0x98:
		resetBit(B, 3);
		return 8;
	case 0x99:
		resetBit(C, 3);
		return 8;
	case 0x9a:
		resetBit(D, 3);
		return 8;
	case 0x9b:
		resetBit(E, 3);
		return 8;
	case 0x9c:
		resetBit(H, 3);
		return 8;
	case 0x9d:
		resetBit(L, 3);
		return 8;
	case 0x9e:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 3)));
		return 16;

	case 0xa7:
		resetBit(A, 4);
		return 8;
	case 0xa0:
		resetBit(B, 4);
		return 8;
	case 0xa1:
		resetBit(C, 4);
		return 8;
	case 0xa2:
		resetBit(D, 4);
		return 8;
	case 0xa3:
		resetBit(E, 4);
		return 8;
	case 0xa4:
		resetBit(H, 4);
		return 8;
	case 0xa5:
		resetBit(L, 4);
		return 8;
	case 0xa6:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 4)));
		return 16;

	case 0xaf:
		resetBit(A, 5);
		return 8;
	case 0xa8:
		resetBit(B, 5);
		return 8;
	case 0xa9:
		resetBit(C, 5);
		return 8;
	case 0xaa:
		resetBit(D, 5);
		return 8;
	case 0xab:
		resetBit(E, 5);
		return 8;
	case 0xac:
		resetBit(H, 5);
		return 8;
	case 0xad:
		resetBit(L, 5);
		return 8;
	case 0xae:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 5)));
		return 16;

	case 0xb7:
		resetBit(A, 6);
		return 8;
	case 0xb0:
		resetBit(B, 6);
		return 8;
	case 0xb1:
		resetBit(C, 6);
		return 8;
	case 0xb2:
		resetBit(D, 6);
		return 8;
	case 0xb3:
		resetBit(E, 6);
		return 8;
	case 0xb4:
		resetBit(H, 6);
		return 8;
	case 0xb5:
		resetBit(L, 6);
		return 8;
	case 0xb6:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 6)));
		return 16;

	case 0xbf:
		resetBit(A, 7);
		return 8;
	case 0xb8:
		resetBit(B, 7);
		return 8;
	case 0xb9:
		resetBit(C, 7);
		return 8;
	case 0xba:
		resetBit(D, 7);
		return 8;
	case 0xbb:
		resetBit(E, 7);
		return 8;
	case 0xbc:
		resetBit(H, 7);
		return 8;
	case 0xbd:
		resetBit(L, 7);
		return 8;
	case 0xbe:
		mem->write(getReg16(HL), mem->read(getReg16(HL)) & (0xff - (1 << 7)));
		return 16;
	default:
		std::cout << "Unknown CB opcode: " << op << "\n";
		return 0;
	}
}

#endif //GB_OP_CB