
void initDisassembly(wchar_t** disassemblyStrings, char* opcodeOperands, char* clocks)
{
	for(int i = 0; i < 256; i++)
	{
		disassemblyStrings[i] = L"";
		opcodeOperands[i] = 1;
		clocks[i] = 4;
	}
	wchar_t** d = disassemblyStrings;
	char* o = opcodeOperands;
	char* c = clocks;

	d[0x00] = L"NOP";						o[0x00] = 0;			c[0x00] = 4;
	d[0x06] = L"LD B, %.2X";				o[0x06] = 1;
	d[0x0E] = L"LD C, %.2X";				o[0x0E] = 1;
	d[0x16] = L"LD D, %.2X";				o[0x16] = 1;
	d[0x1E] = L"LD E, %.2X";				o[0x1E] = 1;
	d[0x26] = L"LD H, %.2X";				o[0x26] = 1;
	d[0x2E] = L"LD L, %.2X";				o[0x2E] = 1;

	d[0x7F] = L"LD A, A";					
	d[0x78] = L"LD A, B";					
	d[0x79] = L"LD A, C";					
	d[0x7A] = L"LD A, D";					
	d[0x7B] = L"LD A, E";					
	d[0x7C] = L"LD A, H";					
	d[0x7D] = L"LD A, L";					
	d[0x7E] = L"LD A, (HL)";				

	d[0x40] = L"LD B, B";					
	d[0x41] = L"LD B, C";					
	d[0x42] = L"LD B, D";					
	d[0x43] = L"LD B, E";					
	d[0x44] = L"LD B, H";					
	d[0x45] = L"LD B, L";					
	d[0x46] = L"LD B, (HL)";				

	d[0x48] = L"LD C, B";					
	d[0x49] = L"LD C, C";					
	d[0x4A] = L"LD C, D";					
	d[0x4B] = L"LD C, E";					
	d[0x4C] = L"LD C, H";					
	d[0x4D] = L"LD C, L";					
	d[0x4E] = L"LD C, (HL)";				

	d[0x50] = L"LD D, B";					
	d[0x51] = L"LD D, C";					
	d[0x52] = L"LD D, D";					
	d[0x53] = L"LD D, E";					
	d[0x54] = L"LD D, H";					
	d[0x55] = L"LD D, L";					
	d[0x56] = L"LD D, (HL)";				

	d[0x58] = L"LD E, B";					
	d[0x59] = L"LD E, C";					
	d[0x5A] = L"LD E, D";					
	d[0x5B] = L"LD E, E";					
	d[0x5C] = L"LD E, H";					
	d[0x5D] = L"LD E, L";					
	d[0x5E] = L"LD E, (HL)";				

	d[0x60] = L"LD H, B";					
	d[0x61] = L"LD H, C";					
	d[0x62] = L"LD H, D";					
	d[0x63] = L"LD H, E";					
	d[0x64] = L"LD H, H";					
	d[0x65] = L"LD H, L";					
	d[0x66] = L"LD H, (HL)";				

	d[0x68] = L"LD L, B";					
	d[0x69] = L"LD L, C";					
	d[0x6A] = L"LD L, D";					
	d[0x6B] = L"LD L, E";					
	d[0x6C] = L"LD L, H";					
	d[0x6D] = L"LD L, L";					
	d[0x6E] = L"LD L, (HL)";				

	d[0x70] = L"LD (HL), B";				
	d[0x71] = L"LD (HL), C";				
	d[0x72] = L"LD (HL), D";				
	d[0x73] = L"LD (HL), E";				
	d[0x74] = L"LD (HL), H";				
	d[0x75] = L"LD (HL), L";				
	d[0x76] = L"LD (HL), %.2X";				o[0x76] = 1;

	d[0x0A] = L"LD A, (BC)";				
	d[0x1A] = L"LD A, (DE)";				
	d[0xFA] = L"LD A, %.2X%.2X";			o[0xFA] = 2;
	d[0x3E] = L"LD A, %.2X";				o[0x3E] = 1;

	d[0x47] = L"LD B, A";					
	d[0x4F] = L"LD C, A";					
	d[0x57] = L"LD D, A";					
	d[0x5F] = L"LD E, A";					
	d[0x67] = L"LD H, A";					
	d[0x6F] = L"LD L, A";					
	d[0x02] = L"LD (BC), A";				
	d[0x12] = L"LD (DE), A";				
	d[0x77] = L"LD (HL), A";				
	d[0xEA] = L"LD (%.2X%.2X), A";			o[0xEA] = 2;

	d[0xF2] = L"LD A, (FF00 + C)";			
	d[0xE2] = L"LD (FF00 + C), A";			

	d[0x3A] = L"LDD A, (HL)";				
	d[0x32] = L"LDD (HL), A";				
	d[0x2A] = L"LDI A, (HL)";				
	d[0x22] = L"LDI (HL), A";				

	d[0xE0] = L"LD (FF00 + %.2X), A";		o[0xE0] = 1;
	d[0xF0] = L"LD A, (FF00 + %.2X)";		o[0xF0] = 1;

	d[0x01] = L"LD BC, %.2X%.2X";			o[0x01] = 2;
	d[0x11] = L"LD DE, %.2X%.2X";			o[0x11] = 2;
	d[0x21] = L"LD HL, %.2X%.2X";			o[0x21] = 2;
	d[0x31] = L"LD SP, %.2X%.2X";			o[0x31] = 2;
	d[0xF9] = L"LD SP, HL";					

	d[0xF8] = L"LDHL SP, %.2X";				o[0xF8] = 1;
	d[0x08] = L"LD (%.2X%.2X), SP";			o[0x08] = 2;

	d[0xF5] = L"PUSH AF";					
	d[0xC5] = L"PUSH BC";					
	d[0xD5] = L"PUSH DE";					
	d[0xE5] = L"PUSH HL";					

	d[0xF1] = L"POP AF";					
	d[0xC1] = L"POP BC";					
	d[0xD1] = L"POP DE";					
	d[0xE1] = L"POP HL";					

	d[0x87] = L"ADD A, A";					
	d[0x80] = L"ADD A, B";					
	d[0x81] = L"ADD A, C";					
	d[0x82] = L"ADD A, D";					
	d[0x83] = L"ADD A, E";					
	d[0x84] = L"ADD A, H";					
	d[0x85] = L"ADD A, L";					
	d[0x86] = L"ADD A, (HL)";				
	d[0xC6] = L"ADD A, %.2X";				o[0xC6] = 1;

	d[0x8F] = L"ADC A, A";					
	d[0x88] = L"ADC A, B";					
	d[0x89] = L"ADC A, C";					
	d[0x8A] = L"ADC A, D";					
	d[0x8B] = L"ADC A, E";					
	d[0x8C] = L"ADC A, H";					
	d[0x8D] = L"ADC A, L";					
	d[0x8E] = L"ADC A, (HL)";				
	d[0xCE] = L"ADC A, %.2X";				o[0xCE] = 1;

	d[0x97] = L"SUB A, A";					
	d[0x90] = L"SUB A, B";					
	d[0x91] = L"SUB A, C";					
	d[0x92] = L"SUB A, D";					
	d[0x93] = L"SUB A, E";					
	d[0x94] = L"SUB A, H";					
	d[0x95] = L"SUB A, L";					
	d[0x96] = L"SUB A, (HL)";				
	d[0xD6] = L"SUB A, %.2X";				o[0xD6] = 1;

	d[0x9F] = L"SBC A, A";					
	d[0x98] = L"SBC A, B";					
	d[0x99] = L"SBC A, C";					
	d[0x9A] = L"SBC A, D";					
	d[0x9B] = L"SBC A, E";					
	d[0x9C] = L"SBC A, H";					
	d[0x9D] = L"SBC A, L";					
	d[0x9E] = L"SBC A, (HL)";				
	d[0xDE] = L"SBC A, %.2X";				o[0xDE] = 1;

	d[0xA7] = L"AND A";						
	d[0xA0] = L"AND B";						
	d[0xA1] = L"AND C";						
	d[0xA2] = L"AND D";						
	d[0xA3] = L"AND E";						
	d[0xA4] = L"AND H";						
	d[0xA5] = L"AND L";						
	d[0xA6] = L"AND (HL)";					
	d[0xE6] = L"AND %.2X";					o[0xE6] = 1;

	d[0xB7] = L"OR A";
	d[0xB0] = L"OR B";
	d[0xB1] = L"OR C";
	d[0xB2] = L"OR D";
	d[0xB3] = L"OR E";
	d[0xB4] = L"OR H";
	d[0xB5] = L"OR L";
	d[0xB6] = L"OR (HL)";
	d[0xF6] = L"OR %.2X";					o[0xf6] = 1;

	d[0xAF] = L"XOR A";
	d[0xA8] = L"XOR B";
	d[0xA9] = L"XOR C";
	d[0xAA] = L"XOR D";
	d[0xAB] = L"XOR E";
	d[0xAC] = L"XOR H";
	d[0xAD] = L"XOR L";
	d[0xAE] = L"XOR (HL)";
	d[0xEE] = L"XOR %.2X";					o[0xee] = 1;

	d[0xBF] = L"CP A";
	d[0xB8] = L"CP B";
	d[0xB9] = L"CP C";
	d[0xBA] = L"CP D";
	d[0xBB] = L"CP E";
	d[0xBC] = L"CP H";
	d[0xBD] = L"CP L";
	d[0xBE] = L"CP (HL)";
	d[0xFE] = L"CP %.2X";					o[0xfe] = 1;

	d[0x3C] = L"INC A";
	d[0x04] = L"INC B";
	d[0x0C] = L"INC C";
	d[0x14] = L"INC D";
	d[0x1C] = L"INC E";
	d[0x24] = L"INC H";
	d[0x2C] = L"INC L";
	d[0x34] = L"INC (HL)";

	d[0x3D] = L"DEC A";
	d[0x05] = L"DEC B";
	d[0x0D] = L"DEC C";
	d[0x15] = L"DEC D";
	d[0x1D] = L"DEC E";
	d[0x25] = L"DEC H";
	d[0x2D] = L"DEC L";
	d[0x35] = L"DEC (HL)";

	d[0x09] = L"ADD HL, BC";
	d[0x19] = L"ADD HL, DE";
	d[0x29] = L"ADD HL, HL";
	d[0x39] = L"ADD HL, SP";

	d[0xE8] = L"ADD SP, %.2X";				o[0xe8] = 1;

	d[0x03] = L"INC BC";
	d[0x13] = L"INC DE";
	d[0x23] = L"INC HL";
	d[0x33] = L"INC SP";

	d[0x0B] = L"DEC BC";
	d[0x1B] = L"DEC DE";
	d[0x2B] = L"DEC HL";
	d[0x3B] = L"DEC SP";

	d[0x27] = L"DAA";

	d[0x2f] = L"CPL";
	d[0x3f] = L"CCF";
	d[0x37] = L"SCF";
	d[0x00] = L"NOP";
	d[0x76] = L"HALT";

	d[0xf3] = L"DI";
	d[0xfb] = L"EI";

	d[0x07] = L"RLCA";
	d[0x17] = L"RLA";
	d[0x0f] = L"RRCA";
	d[0x1f] = L"RRA";

	d[0xc3] = L"JP %.2X%.2X";				o[0xc3] = 2;

	d[0xc2] = L"JP NZ, %.2X%.2X";			o[0xc2] = 2;
	d[0xca] = L"JP Z, %.2X%.2X";			o[0xca] = 2;
	d[0xd2] = L"JP NC, %.2X%.2X";			o[0xd2] = 2;
	d[0xda] = L"JP C, %.2X%.2X";			o[0xda] = 2;
	d[0xe9] = L"JP (HL)";					o[0xe9] = 0;
	d[0x18] = L"JR PC + %.2X";				o[0x18] = 1;
	d[0x20] = L"JR NZ, %.2X";				o[0x20] = 1;
	d[0x28] = L"JR Z, %.2X";				o[0x28] = 1;
	d[0x30] = L"JR NC, %.2X";				o[0x30] = 1;
	d[0x38] = L"JR C, %.2X";				o[0x38] = 1;

	d[0xcd] = L"CALL %.2X%.2X";				o[0xcd] = 2;
	d[0xc4] = L"CALL NZ, %.2X%.2X";			o[0xc4] = 2;
	d[0xcc] = L"CALL Z, %.2X%.2X";			o[0xcc] = 2;
	d[0xd4] = L"CALL NC, %.2X%.2X";			o[0xd4] = 2;
	d[0xdc] = L"CALL C, %.2X%.2X";			o[0xdc] = 2;

	d[0xc7] = L"RST 0x00";
	d[0xcf] = L"RST 0x08";
	d[0xd7] = L"RST 0x10";
	d[0xdf] = L"RST 0x18";
	d[0xe7] = L"RST 0x20";
	d[0xef] = L"RST 0x28";
	d[0xf7] = L"RST 0x30";
	d[0xff] = L"RST 0x38";

	d[0xc9] = L"RET";
	d[0xc0] = L"RET NZ";
	d[0xc8] = L"RET Z";
	d[0xd0] = L"RET NC";
	d[0xd8] = L"RET C";

	d[0xd9] = L"RETI";

	d[0xcb] = L"CB ...";					o[0xcb] = 2;

	for(int i = 0; i < 256; i++)
	{
		if(o[i] == -1)
			o[i] = 0;
	}
}

void initDisassemblyCB(wchar_t** disassemblyStrings, char* opcodeOperands, char* clocks)
{
	
	//CB swap
	//CB rlc
	//CB rl
	//CB rrc
	//CB rr
	//CB sla
	//CB sra
	//CB srl
	//CB bit
	//CB set
	//CB res
	//CB test
}