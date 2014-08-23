#ifndef ENUM
#define ENUM

#include "State.h"

#define LOG_OP				0
#define SHOW_ALL_VID_RAM	0
#define AUDIO_ENABLE		1
#define BREAKPOINT_CODE		0xFD
#define LOGGED_OPS			500000
//#define DEBUG

class Gameboy;

typedef unsigned char byte;
typedef unsigned short word;
typedef wchar_t StrBuffer[256];
static const int clocksPerSecond = 4194304;

struct Instruction
{
	byte opcode;
	byte p1;
	byte p2;
	byte numP;
	word address;
};

enum InstructionType;

enum BreakpointType
{
	BreakpointExecute = 1,
	BreakpointRead = 2,
	BreakpointWrite = 4
};

enum BreakpointCondition
{
	NoCondition,
	GreaterThan,
	LessThan,
	GreaterThanEqual,
	LessThanEqual,
	Equal,
	NotEqual
};

enum MovieFormat
{
	QBoy =	(1 << 0),
	VBA =	(1 << 1)
};


struct InstructionInfo
{
	InstructionType type;
	const wchar_t* disassembly;
	bool twoByteOpcode;
	byte numOperands;
	byte clockCycles;
	word opcode;

	InstructionInfo(InstructionType t, wchar_t* d, bool tbo, byte nop, byte cl, word op)
	{
		type = t; disassembly = d; twoByteOpcode = tbo;
		numOperands = nop; clockCycles = cl; opcode = op;
	}
};

struct GBOPCODE {
	byte mask;
	byte hexCode;
	char* assembly;
};

struct RomInfo
{
	char romName[24];
	int totalRamSize;
	int totalRomSize;
	int numRamBanks;
	int numRomBanks;
	int checksum;
	int cartType;
	int region;
	int licensee;
	int maskVersionNum;
};

struct RegisterInfo
{
	byte reg[10];
	word PC;
	bool CF;
	bool ZF;
	bool NF;
	bool HF;
};

struct Breakpoint
{
	byte opcode;
	byte p1;
	byte p2;
	word address;
	word source;
	byte value;
	byte valueBitmask;
	BreakpointType type;
	BreakpointCondition condition;
};

enum Dialogs
{
	Controls,
	Rewind,
	Rerecording,
	Disassemble,
	CartInfo,
	MemViewer,
	MemSearch,
	BreakpointList,
	Visualizer,
	MoviePanel,
	DlgMax
};

enum InstructionType
{
	Jump = (1 << 0),
	Compare = (1 << 1),
	Arithmatic8Bit = (1 << 2),
	Arithmatic16Bit = (1 << 3),
	Load8Bit = (1 << 4),
	Load16Bit = (1 << 5),
	Call = (1 << 6),
	Increment = (1 << 7),
	Decrement = (1 << 8),
	Shift = (1 << 9),
	Rotate = (1 << 10),
	GetBit = (1 << 11),
	SetBit = (1 << 12),
	Return = (1 << 13),
	Push = (1 << 14),
	Pop = (1 << 15),
	And = (1 << 16),
	Or = (1 << 17),
	Xor = (1 << 18),
	Not = (1 << 19),
	Swap = (1 << 20)
};

enum MessageType
{
	NO_MESSAGE,
	VOLUME_INCREASED,
	VOLUME_DECREASED,
	SOUND_MUTED,
	SOUND_UNMUTED,
	EMULATION_SPEED_INCREASED,
	EMULATION_SPEED_DECREASED,
	EMULATION_SPEED_MAX,
	EMULATION_STOPPED,
	EMULATION_REWINDING,
	UNKNOWN_OPCODE,
	UNKNOWN_CARTRIDGE,
	STATE_LOADED,
	STATE_SAVED,
	SAVE_SLOT_CHANGED,
	NO_STATE_FILE,
	PAUSED,
	UNPAUSED,
	EMU_SPEED,
	FRAME_COUNT,
	PLAYBACK_STOPPED,
	PLAYBACK_STARTED,
	SOUND_MODE_ON,
	SOUND_MODE_OFF,
	DEBUG_BREAK_EXECUTE,
	DEBUG_BREAK_WRITE,
	DEBUG_BREAK_READ,
	CANT_OPEN_VIDEO_RECORD,
	MESSAGE_MAX
};

struct Message
{
	MessageType type;
	double value;

	Message(MessageType mt, double v)
	{
		type = mt;
		value = v;
	}

	Message(MessageType mt)
	{
		type = mt;
		value = 0;
	}
};

static std::string messageText[MESSAGE_MAX] =
{
	"",
	"Volume: %.0f%%",
	"Volume: %.0f%%",
	"Volume: Muted",
	"Volume: Unmuted",
	"%.2fx",
	"%.2fx",
	"Fast Forward >>",
	"STOPPED",
	"<< Rewind",
	"",
	"Unknown Cartridge",
	"State Loaded",
	"State Saved",
	"Slot %.0f Selected",
	"File Not Found",
	"Paused",
	"Unpaused",
	"%.2f%%",
	"Frame: %.0f",
	"Playback Stopped",
	"Playback Started",
	"Sound Mode %.0f ON",
	"Sound Mode %.0f OFF",
	"Breakpoint Execute: Addr %.4X",
	"Breakpoint Write: Addr %.4X",
	"Breakpoint Read: Addr %.4X",
	"Unable to record video"
};

enum InputFlags
{
	GB_A = 0xF - 1,
	GB_B = 0xF - 2,
	GB_START = 0xF - 4,
	GB_SELECT = 0xF - 8,
	GB_RIGHT = 0xF - 1,
	GB_LEFT = 0xF - 2,
	GB_UP = 0xF - 4,
	GB_DOWN = 0xF - 8,
	GB_NO_CHANGE = 0xFF,

	GBE_RESET = (1 << 0),
	GBE_LOADSTATE = (1 << 1),
	GBE_MOVIE_START = (1 << 2),
	GBE_MOVIE_END = (1 << 3),
};

enum GInputIndex
{
	//P14 input flags
	BIND_RIGHT = 0,
	BIND_LEFT = 1,
	BIND_UP = 2,
	BIND_DOWN = 3,

	//P15 input flags
	BIND_A = 4,
	BIND_B = 5,
	BIND_SELECT = 7,
	BIND_START = 6
};


enum EmulatorInput
{
	//emulator features
	BIND_SAVE_STATE = 8,
	BIND_LOAD_STATE = 9,
	BIND_REWIND = 10,
	BIND_FAST_FORWARD = 11,
	BIND_INCREMENT_SPEED = 12,
	BIND_DECREMENT_SPEED = 13,
	BIND_INCREMENT_STATE = 14,
	BIND_DECREMENT_STATE = 15,
	BIND_MENU = 16,
	BIND_VOL_PLUS = 17,
	BIND_VOL_MINUS = 18,
	BIND_VOL_MUTE = 19,
	BIND_SLOT_INC = 20,
	BIND_SLOT_DEC = 21,
	BIND_FRAME_ADVANCE = 22,
	BIND_PREVIOUS_FRAME = 23,
	BIND_PAUSE_TOGGLE = 24,
	BIND_MAX
};


enum 
{
	VIDEO_RAM_ADDRESS = 0x8000,
	ROM_BANK_SIZE = 0x4000,
	VIDEO_RAM_SIZE = 0x2000,
	UPPER_RAM_SIZE = 0x4000
};

#endif //ENUM