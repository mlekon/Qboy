#ifndef GAMEBOY_H
#define GAMEBOY_H

class IGameBoyComponent;

#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include "allegro.h"
#include "winalleg.h"
#include <Windows.h>
#include "enum.h"
#include "MemoryManager.h"
#include "LCD.h"
#include "Z80.h"
#include "Timer.h"
#include "Audio.h"
#include "Movie.h"
#include <iostream>

#define SPEED_MAX_INDEX		13
#define MAX_SAVE_SLOT		10

class Gameboy
{
private:
	HANDLE winMutex;

	BreakpointMap* bpm;

	MemoryManager* mem;							//pointer to memory component
	Z80* z80;									//pointer to cpu
	Cartridge* cart;							//pointer to cartridge
	LCD* lcd;									//pointer to display
	Timer* time;								//pointer to the timer handler
	Audio* audio;								//pointer to the audio

	Movie* mov;
	State** s;									//array of individual states; loaded/saved explicitly
	State** rb;									//rewinding buffer; saved automatically, loaded repeated
												//when the user is pressing the rewind key

	JOYSTICK_INFO* joyData;

	std::list<Message> messages;
	int messageDelay;
	int messageTimer;

	bool volumeFlag;
	bool speedFlag;
	int speedIndex;
	int emulationSpeed;
	double procStepTime;						//real time taken to execute last set of cpu instructions
	double procStepClocks;						//time in cpu clocks of last cpu set
	double speedMultiplier;						//overall execution speed multiplier
	double rewindConstructTimer;				//system time when the last r.w. buffer state was saved
	double rewindingTimer;						//system time when the last r.w. buffer state was loaded

	bool rewindEnabled;
	int rwSaveInterval;
	int rwLoadInterval;
	int rwBufferSize;							//size in bytes of the rewinding buffer

	byte port10;
	byte port20;
	byte port10Latch;
	byte port20Latch;
	byte emuInput;

	int speedTimer;
	int speedStart;

	int keybinds[BIND_MAX];						//key codes for each available action
	int rewindBufferNext;						//index of the rewinding buffer to place the next state
	int rewindBufferConstructed;				//number of valid states in the rewinding buffer
	int numRewindStates;						//maximum number of states in the r.w. buffer
	int currentSaveSlot;
	int timeToWait;								//number of ms to wait after each cpu set
	int timeDeficit;							//number of ms behind the normal timing
	int maxFrameSkip;							//maximum number of consecutive frames to skip
	int framesSkipped;							//number of consecutive frames skipped
	int displayScale;							//scale of the window from the original 160 x 144

	int frameAdvance;
	bool frameAdvanceFlag;
	bool pauseToggleFlag;
	bool timerActive;							//is the internal timer ticking?
	bool loadedFlag;							//true after state load when key hasn't been released yet
	bool savedFlag;								//true after state save when key hasn't been released yet
	bool rewinding;								//only true while the rewind key is held; stops new r.w.
	bool stateSlotFlag;							//buffer states from being written to the buffer
	bool closing;								//is the program closing?
	bool paused;
	bool recording;
	bool playingBack;
	bool prevStateFlag;
	bool broken;								//is the debugger active and has the program broken?
public:
	static const double msPerClock;				//miliseconds per cpu clock (4.194304 MHz)
	static const int cyclesPerFrame;			//cpu cycles elapsed before drawing the frame
	static const double speeds[SPEED_MAX_INDEX];
	static const int frameSkips[SPEED_MAX_INDEX];
	Gameboy();
	~Gameboy();
	void mainLoop();
	void update();
	void input();
	void cpu();
	void display();
	void interrupts();
	void initTimer();
	void timer();
	void states();
	void timing();

	void constructState(State*);
	void restoreState(State*);
	void exitGame();
	void loadGame(std::string);

	Z80* getZ80();
	MemoryManager* getMemory();
	LCD* getLCD();
	Cartridge* getCartridge();

	void configureGameboyType(GameboyType gbType);
	std::list<Message>* getMessages();
	void addMessage(MessageType);
	void addMessage(MessageType, double);
	int getMessageTimer();
	int getMessageDelay();
	Message peekMessage();
	void resetMessageTimer();
	int getDisplayScale();
	BreakpointMap* getBpm();
	
	//called from other threads
	void loadUserState(int);
	void saveUserState(int);
	void setDisplayScale(int);
	void setSoundLevel(double v);
	void soundModeToggle(int mode);
	bool pause(bool b);
	bool pause();
	void setInput(byte p1, byte p2);
	void setInputLatch(byte p1, byte p2);
	void setColors(int, int, int, int);
	void setTextOverlayColor(int);
	void setEmulationSpeed(int);
	void startRecording(std::string title, std::string author, std::string file, int format);
	void stopRecording();
	void loadRecording(std::string fileName);
	void stopPlayback();
	void advanceEmulation(int frames);
	void nextFrame(int frames);
	void reset();
	void rewindFrames(int);
	void saveState();
	void addBreakpoint(Breakpoint bp);
	void removeBreakpoint(word address);
	void debugBreak();
	void debugContinue();
	bool cartridgeIsLoaded();
	bool debugIsBroken();
	int rewindSaveInterval(int);
	int rewindLoadInterval(int);
	int rewindBufferSize(int);
	int filledBufferSize();
	int totalBufferSize();
	int getColor(int);
	int getFrameCount();
	int getLagFrames();
	int getCurrentEmulationSpeed();
	RomInfo getRomInfo();
	RegisterInfo getRegisterInfo();
	word getRegister(Register r);
	byte readMemory(int address);
	word readStack(word offset);
	byte readCartridgeROM(int bank, int address);
	byte readCartridgeRAM(int bank, int addresss);
	Instruction getInstruction(word address);
	Breakpoint* enumerateBreakpoints(int index);
	void clearBreakpoints();
	void step(int numInstructions);
	bool getMemoryVisualization(BITMAP**);
};

#endif //GAMEBOY_H