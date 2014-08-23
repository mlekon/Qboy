#include "Gameboy.h"
#include "Disassembly.h"
#include "GBCLCD.h"
#include "GBCMemoryManager.h"
#include <sstream>

const double Gameboy::msPerClock					= (1/4194.304);
const int Gameboy::cyclesPerFrame					= 70256;
const double Gameboy::speeds[SPEED_MAX_INDEX]		= {0.1, 0.25, 0.5, 0.75, 1, 1.25, 1.5, 2, 4, 8, 16, 1000};
const int Gameboy::frameSkips[SPEED_MAX_INDEX]		= {0, 0, 0, 0, 0, 1, 2, 3, 5, 8, 16, 25};

/////////////////////////////////
//executes the main emulation loop
Gameboy::Gameboy()
{
	closing =		false;
	paused =		false;
	recording =		false;
	playingBack =	false;
	broken =		false;

	winMutex = CreateMutexEx(NULL, L"GB", NULL, SYNCHRONIZE);

	bpm = new BreakpointMap();

	//rewinding vars
	rewindConstructTimer	= 0;
	rewindingTimer			= 0;
	rwBufferSize			= 200;
	rwLoadInterval			= 1;
	rwSaveInterval			= 0;
	rewindBufferNext		= 0;
	rewindBufferConstructed	= 0;
	rewinding				= false;

	//state saving/loading vars
	loadedFlag		= false;
	savedFlag		= false;
	stateSlotFlag	= false;
	currentSaveSlot	= 0;

	port10 = port20 = port10Latch = port20Latch = 0xf;
	emuInput = 0;

	//timing vars
	emulationSpeed		= 0;
	procStepTime		= 0;
	procStepClocks		= 0;
	speedMultiplier		= 1;
	timeToWait			= 0;
	timeDeficit			= 0;
	maxFrameSkip		= 10;
	framesSkipped		= 0;
	speedIndex			= 4;
	speedFlag			= false;
	messageDelay		= 1000;
	messageTimer		= clock();
	speedTimer			= 0;
	frameAdvance		= 0;
	prevStateFlag		= false;

	mov = NULL;

	//default window size
	displayScale = 3;

	//set up graphics, sound, and input
	allegro_init();
	install_keyboard();
	reserve_voices(5, 0);
	install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
	install_joystick(JOY_TYPE_AUTODETECT);
	joyData = joy;
	set_color_depth(32);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, LCD::baseDisplayX, LCD::baseDisplayY, 0,0);
	set_display_switch_mode(SWITCH_BACKGROUND);

	//initialize gameboy components
	cart = NULL;
	mem = NULL;
	z80 = NULL;
	lcd = NULL;
	time = NULL;
	audio = NULL;
#ifdef GBC
	////// GBC //////
	mem = new GBCMemoryManager();
#else
	mem = new MemoryManager();
#endif GBC
	z80 = new Z80();
	
#ifdef GBC
	////// GBC //////
	lcd = new GBCLCD();
#else
	lcd = new LCD();
#endif
	time = new Timer();
	audio = new Audio();

#ifdef GBC
	////// GBC //////
	((GBCMemoryManager*)mem)->setLCD((GBCLCD*)lcd);
#endif

	lcd->setScale(displayScale);
	paused = true;

	//initialize individual states on demand
	s = new State*[MAX_SAVE_SLOT];
	for(int i = 0; i < MAX_SAVE_SLOT; i++)
		s[i] = new State();

	//initialize each state in the rewind buffer
	rb = new State*[rwBufferSize];
	for(int i = 0; i < rwBufferSize; i++)
		rb[i] = new State();

	//assign default key bindings
	keybinds[BIND_RIGHT]			= KEY_RIGHT, 
	keybinds[BIND_LEFT]				= KEY_LEFT, 
	keybinds[BIND_UP]				= KEY_UP, 
	keybinds[BIND_DOWN]				= KEY_DOWN, 
	keybinds[BIND_A]				= KEY_Z, 
	keybinds[BIND_B]				= KEY_X, 
	keybinds[BIND_SELECT]			= KEY_C, 
	keybinds[BIND_START]			= KEY_V;
	keybinds[BIND_SAVE_STATE]		= KEY_P;
	keybinds[BIND_LOAD_STATE]		= KEY_O;
	keybinds[BIND_REWIND]			= KEY_Q;
	keybinds[BIND_FAST_FORWARD]		= KEY_W;
	keybinds[BIND_INCREMENT_SPEED]	= KEY_S;
	keybinds[BIND_DECREMENT_SPEED]	= KEY_A;
	keybinds[BIND_INCREMENT_STATE]	= KEY_0;
	keybinds[BIND_DECREMENT_STATE]	= KEY_9;
	keybinds[BIND_MENU]				= KEY_F1;
	keybinds[BIND_VOL_PLUS]			= KEY_PGUP;
	keybinds[BIND_VOL_MINUS]		= KEY_PGDN;
	keybinds[BIND_VOL_MUTE]			= KEY_END;
	keybinds[BIND_SLOT_INC]			= KEY_CLOSEBRACE;
	keybinds[BIND_SLOT_DEC]			= KEY_OPENBRACE;
	keybinds[BIND_FRAME_ADVANCE]	= KEY_N;
	keybinds[BIND_PREVIOUS_FRAME]	= KEY_B;
	keybinds[BIND_PAUSE_TOGGLE]		= KEY_SPACE;
}

void Gameboy::mainLoop()
{
	//main loop. executes processor, draws display,
	//reads input, manage state saving and loading,
	//executes the main menu
	while(!closing)
	{
		if(!paused && !broken && WaitForSingleObject(winMutex,INFINITE) == 0)
		{
			procStepTime = clock();
			procStepClocks = z80->cpuClocks;

			//input();   //experimental: input moved to just before vblank call
			update();
			states();
			timing();

			ReleaseMutex(winMutex);
		}
		else
		{
			rest(10);
			
			if(cart != NULL)
			{
				states();
				timing();
			}
		}
	}

	allegro_exit();
}

Gameboy::~Gameboy()
{
	delete cart;
	delete mem;
	delete z80;
	delete lcd;
	delete time;
	delete audio;
}


/////////////////////////////////
//set input ports for the gameboy controls the
//value is saved in the memory manager once per frame
void Gameboy::input()
{
	//set directional inputs
	//subtract the control's binary value from port20
	if(key[keybinds[BIND_RIGHT]])
		port20 -= 1;
	if(key[keybinds[BIND_LEFT]])
		port20 -= 2;
	if(key[keybinds[BIND_UP]])
		port20 -= 4;
	if(key[keybinds[BIND_DOWN]])
		port20 -= 8;

	//set the other inputs
	//subtract the control's binary value from port10
	if(key[keybinds[BIND_A]])
		port10 -= 1;
	if(key[keybinds[BIND_B]])
		port10 -= 2;
	if(key[keybinds[BIND_SELECT]])
		port10 -= 4;
	if(key[keybinds[BIND_START]])
		port10 -= 8;

	if(recording && mov)
		mov->record(port20, port10, 0, lcd->getFrame());
	else if(playingBack && mov)
	{
		byte i = mov->playback(lcd->getFrame());
		byte e = mov->playbackEmu(lcd->getFrame());
		port10 = i & 0xf;
		port20 = i >> 4;
	}

	//store the input port values in the memeory mananger
	mem->setInputPorts(port10, port20);

	//there are two ports: one for directional
	//buttons, and another for all others
	//a depressed button is represented by a 0 bit
	port10 = port10Latch;
	port20 = port20Latch;

	//set the cpu active if a button is pressed
	if(port10 != 0xf || port20 != 0xf)
		z80->cpuActive = true;
}


/////////////////////////////////
//controls state saving and loading. this includes
//the rewinding operation, which is done by rapidly loading
//states progressing backward through time
void Gameboy::states()
{
	char slotChange = 0;
	if(key[keybinds[BIND_SLOT_INC]])
		slotChange = 1;
	else if(key[keybinds[BIND_SLOT_DEC]])
		slotChange = -1;
	else
		stateSlotFlag = false;


	//load only the most recent rewind state and pause
	//the frame is drawn immediately with whatever is in the video ram
	if(key[keybinds[BIND_PREVIOUS_FRAME]])
	{
		if(!prevStateFlag && rewindBufferConstructed > 0)
		{
			//index of the next state to load
			int next = (rewindBufferNext == 0) ? rwBufferSize-1 : rewindBufferNext-1;

			restoreState(rb[next]);
			lcd->fullRender();

			//update the rewind buffer
			rewindBufferConstructed--;
			rewindBufferNext = next;
			prevStateFlag = true;

			if(!paused)
				audio->pauseSound();

			paused = true;
		}
	}
	else
	{
		prevStateFlag = false;
	}

	//change the save slot if a key was just pressed
	if(slotChange != 0 && !stateSlotFlag)
	{
		currentSaveSlot += slotChange;

		//bounds checking
		if(currentSaveSlot < 0) currentSaveSlot = 0;
		if(currentSaveSlot >= MAX_SAVE_SLOT) currentSaveSlot = MAX_SAVE_SLOT - 1;

		addMessage(SAVE_SLOT_CHANGED, currentSaveSlot);
		stateSlotFlag = true;
	}


	//begin rewinding
	if(key[keybinds[BIND_REWIND]])
	{
		rewindingTimer++;
		
		//restore next buffered state if state exists and 100 ms have elapsed
		if(rewindBufferConstructed > 0 && rewindingTimer > rwLoadInterval)
		{
			if(rewinding == false)
				addMessage(EMULATION_REWINDING);

			//index of the next state to load
			int next = (rewindBufferNext == 0) ? rwBufferSize-1 : rewindBufferNext-1;

			rewinding = true;
			restoreState(rb[next]);

			//update the rewind buffer
			rewindBufferConstructed--;
			rewindBufferNext = next;

			//reset the timer for the next state restoration
			rewindingTimer = 0;

			//prevent cartridge ram from being written 
			//(possible repeatedly) while rewinding
			cart->lockRam();
		}
	}
	else
	{
		//rewinding key released; being filling buffer again
		cart->unlockRam();
		rewinding = false;
	}
	

	if(!paused)
		rewindConstructTimer++;

	//construct the next state in the rewinding buffer every 300 ms
	if(rewindConstructTimer > rwSaveInterval && !rewinding)
	{
		constructState(rb[rewindBufferNext % rwBufferSize]);

		//reset timer for the next state save
		rewindConstructTimer = 0;

		//update the rewind buffer with the newest state
		rewindBufferNext = (rewindBufferNext+1) % rwBufferSize;
		rewindBufferConstructed = (rewindBufferConstructed == rwBufferSize) ? rwBufferSize : rewindBufferConstructed + 1;
	}

	//save state to the disk in single keypress
	if(key[keybinds[BIND_SAVE_STATE]] && !savedFlag)
	{
		constructState(s[currentSaveSlot]);
		s[currentSaveSlot]->save_to_disk(cart->romName, currentSaveSlot);
		addMessage(STATE_SAVED);

		//prevent saving multiple times if the key is held
		savedFlag = true;
	}
	else if(!key[keybinds[BIND_SAVE_STATE]])
	{
		//save key released- save again on next press
		savedFlag = false;
	}

	//load state on single keypress
	if(key[keybinds[BIND_LOAD_STATE]] && !loadedFlag)
	{
		//load if file exists
		if(!s[currentSaveSlot]->load_from_disk(cart->romName, currentSaveSlot))
		{
			restoreState(s[currentSaveSlot]);
			addMessage(STATE_LOADED);
		}
		else
			addMessage(NO_STATE_FILE);
		loadedFlag = true;	
	}
	//load state key released; allow another load
	else if(!key[keybinds[BIND_LOAD_STATE]])
	{
		loadedFlag = false;
	}
}


/////////////////////////////////
//controls the speed of emulation. handles user-controlled
//speed changes and attempts to slow normal emulation
//to more accurate speeds.
void Gameboy::timing()
{
	if(frameAdvance > 0 && !prevStateFlag)
	{
		paused = true;
		frameAdvance = 0;
	}


	if(key[keybinds[BIND_FRAME_ADVANCE]])
	{
		if(!frameAdvanceFlag)
		{
			paused = false;
			broken = false;
			frameAdvanceFlag = true;
			frameAdvance = 1;
		}
	}
	else
		frameAdvanceFlag = false;


	if(key[keybinds[BIND_PAUSE_TOGGLE]])
	{
		if(!pauseToggleFlag)
		{
			pause();
			//paused = !paused;
			//(paused) ? audio->pauseSound() : audio->unpauseSound();
			//addMessage((paused) ? PAUSED : UNPAUSED, 0);
			pauseToggleFlag = true;
		}
	}
	else
		pauseToggleFlag = false;


	if(key[keybinds[BIND_VOL_PLUS]])
	{
		//increase the volume by 5%
		if(!volumeFlag)
		{
			volumeFlag = true;
			audio->setVolume(audio->getVolume() + 0.05);
			addMessage(VOLUME_INCREASED, (audio->getVolume() * 100));
		}
	}
	else if(key[keybinds[BIND_VOL_MINUS]])
	{
		//decrease the volume by 5%
		if(!volumeFlag)
		{
			volumeFlag = true;
			audio->setVolume(audio->getVolume() - 0.05);
			addMessage(VOLUME_DECREASED, (audio->getVolume() * 100));
		}
	}
	else if(key[keybinds[BIND_VOL_MUTE]])
	{
		//toggle the sound on/off
		if(!volumeFlag)
		{
			volumeFlag = true;
			addMessage(((audio->mute()) ? SOUND_MUTED : SOUND_UNMUTED));
		}
	}
	else
		volumeFlag = false;


	if(key[keybinds[BIND_INCREMENT_SPEED]])
	{
		//increase the target emulation speed with the next value
		//in the speeds array
		if(!speedFlag)
		{
			speedFlag = true;
			if(speedIndex + 1 < SPEED_MAX_INDEX-1)
				speedIndex++;

			addMessage(EMULATION_SPEED_INCREASED, speeds[speedIndex]);
		}
	}
	else if(key[keybinds[BIND_DECREMENT_SPEED]])
	{
		//decrease target emulation speed
		if(!speedFlag)
		{
			speedFlag = true;
			if(speedIndex - 1 >= 0)
				speedIndex--;

			//addMessage(EMULATION_SPEED_DECREASED, speeds[speedIndex]);
		}
	}
	else
		speedFlag = false;

	if(key[keybinds[BIND_FAST_FORWARD]])
	{
		if(speedMultiplier != 100000)
			addMessage(EMULATION_SPEED_MAX);

		//increase execution speed and increase frame skip to 
		//reduce processing requirements
		speedMultiplier = 100000;
		lcd->skipFrames(15);
	}
	else
	{
		//set to normal speed
		speedMultiplier = speeds[speedIndex];
		lcd->skipFrames(frameSkips[speedIndex]);

		//calculate the number of clock cycles since the last timing check
		procStepClocks = z80->cpuClocks - procStepClocks;

		//calculate the real-time ticks since the last timing check
		procStepTime = clock() - procStepTime;

		//if the emulation is running faster than the original game boy
		//wait until real time has caught up
		timeToWait += (procStepClocks*(msPerClock/speedMultiplier) - procStepTime);
		timeToWait = (timeToWait < 0) ? 0 : timeToWait;

		//wait if the simulation is at least 10 ms ahead
		//do not wait more than 200 ms
		if(timeToWait > 10)
		{
			int beforeWait = clock();
			rest((timeToWait > 200) ? 200 : timeToWait);
			int afterWait = clock();
			timeToWait -= (afterWait - beforeWait);
			if(timeToWait < -200 || timeToWait > 200) timeToWait = 0;
		}
	}

}


/////////////////////////////////
//executes the given number of instructions
//using the Z80. all associated operations
//such as interrupts, timers, and display drawing
//are also done.
void Gameboy::update()
{
	int soundTimer = 0;
	//this loop executes hundreds of thousands of times 
	//per second, so even small improvements may
	//make a significant different in performance
	do
	{
		z80->procOpcode();
		time->timer(z80->lastOpClocks);
		speedTimer += z80->lastOpClocks;

		if(speedTimer >= 4194304)
		{
			emulationSpeed = float(1000.0f / float(clock() - speedStart + 1)) * 1000.0f;
			addMessage(EMU_SPEED, float(1000 / float(clock() - speedStart + 1) * 100));
			speedStart = clock();
			speedTimer -= 4194304;
		}

#ifdef AUDIO_ENABLE
		soundTimer++;
		if(soundTimer > 256)
		{
			soundTimer = 0;
			audio->sound(z80->clockCounter);
			z80->resetClockCounter();
		}
#endif	
	} while(!broken && lcd->draw() == false);

}


/////////////////////////////////
//calls constructState from each system component
//in order to create a complete snapshot of the system
void Gameboy::constructState(State* s)
{
	z80->constructState(s);
	mem->constructState(s);
	lcd->constructState(s);
	cart->constructState(s);

	//if(mov != NULL && recording)
	//	s->mov = *mov;
}


/////////////////////////////////
//calls restoreState from each system component
//in order to restore the state described in the
//given state object
void Gameboy::restoreState(State* s)
{
	z80->restoreState(s);
	mem->restoreState(s);
	lcd->restoreState(s);
	cart->restoreState(s);

	//reset execution speed timing variables
	procStepTime = clock();
	procStepClocks = z80->cpuClocks;

	//if(recording)
	//	*mov = s->mov;
}


/////////////////////////////////
//getters for the system components
Z80*			Gameboy::getZ80()		{ return z80; }
MemoryManager*	Gameboy::getMemory()	{ return mem; }
LCD*			Gameboy::getLCD()		{ return lcd; }
Cartridge*		Gameboy::getCartridge()	{ return cart; }


/////////////////////////////////
//causes the main loop to exit
void Gameboy::exitGame()
{
	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		closing = true;
		CloseHandle(winMutex);
	}
}


/////////////////////////////////
//creates a new cartridge object from the
//given file and notifies all relevant subsystems
void Gameboy::loadGame(std::string file)
{
	if(WaitForSingleObject(winMutex,INFINITE) == 0)
	{
		if(cart != NULL)
		{
			delete cart;
			cart = NULL;
		}

		cart = Cartridge::cartFactory(file, this);
		
		speedMultiplier = 1;
		audio->enableSound();
		if(peekMessage().type == UNKNOWN_CARTRIDGE)
		{
			speedMultiplier = 0.01;
			audio->disableSound();
			delete cart;
			cart = NULL;
			paused = true;

			return;
		}

		recording = false;
		playingBack = false;

		configureGameboyType(cart->getGameboyType());
//#ifdef GBC
//		////// GBC //////
//		((GBCLCD*)lcd)->setComponents((GBCMemoryManager*)mem, z80, this);
//		((GBCMemoryManager*)mem)->setComponents(z80, cart, time, audio, this);
//#else
//		lcd->setComponents(mem, z80, this);
//		mem->setComponents(z80, cart, time, audio, this);
//#endif
//		z80->setComponents(mem, cart, lcd, this);
//		cart->setComponents(mem, z80, this);
//		time->setComponents(mem, z80, this);
//		audio->setComponents(mem, z80, this);

		reset();
	
		ReleaseMutex(winMutex);
	}
}


/////////////////////////////////
//adds a message to display over the game
//used to notify the user of emulator features
void Gameboy::addMessage(MessageType mt)
{
	addMessage(mt, 0);
}


/////////////////////////////////
//adds a message to display over the game
//used to notify the user of emulator features
void Gameboy::addMessage(MessageType mt, double value)
{
	messages.push_back(Message(mt, value));
	messageTimer = clock();

	if(messages.size() > 1)
		messages.pop_front();
}


/////////////////////////////////
//returns the time since the last message
//was removed from the list
int Gameboy::getMessageTimer()
{
	return messageTimer;
}


/////////////////////////////////
//sets the message timer to the current
//system time, resetting the time before
//the next message is removed
void Gameboy::resetMessageTimer()
{
	messageTimer = clock();
}



/////////////////////////////////
//returns the time between message removal
int Gameboy::getMessageDelay()
{
	return messageDelay;
}


/////////////////////////////////
//returns a pointer to the list containing
//the messages to display
std::list<Message>* Gameboy::getMessages()
{
	return &messages;
}


/////////////////////////////////
//returns the newest message in the list
Message Gameboy::peekMessage()
{
	if(messages.size() == 0)
		return Message(NO_MESSAGE);

	return messages.back();
}


/////////////////////////////////
//gives the screen scale to the LCD component
int Gameboy::getDisplayScale()
{
	return displayScale;
}


void Gameboy::loadUserState(int stateIndex)
{
	if(cart == NULL)
		return;

	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		//load if file exists
		if(!s[stateIndex]->load_from_disk(cart->romName, stateIndex))
		{
			restoreState(s[stateIndex]);
			lcd->fullRender();
			broken = false;
			addMessage(STATE_LOADED);
		}
		else
			addMessage(NO_STATE_FILE);

		ReleaseMutex(winMutex);
	}
}


void Gameboy::saveUserState(int stateIndex)
{
	if(cart == NULL)
		return;

	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		constructState(s[stateIndex]);
		s[currentSaveSlot]->save_to_disk(cart->romName, stateIndex);
		addMessage(STATE_SAVED);

		ReleaseMutex(winMutex);
	}
}


void Gameboy::setDisplayScale(int scale)
{
	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		displayScale = scale;
		lcd->setScale(scale);
		set_gfx_mode(GFX_AUTODETECT_WINDOWED, LCD::baseDisplayX * scale, LCD::baseDisplayY * scale, 0, 0);
		set_display_switch_mode(SWITCH_BACKGROUND);
		ReleaseMutex(winMutex);
	}
}


void Gameboy::setSoundLevel(double v)
{
	if(cart == NULL)
		return;

	if(v > 0)
	{
		audio->setVolume(v);
		addMessage(VOLUME_INCREASED, (audio->getVolume() * 100));
	}
	else
	{
		addMessage(((audio->mute()) ? SOUND_MUTED : SOUND_UNMUTED));
	}
}


bool Gameboy::pause(bool b)
{
	if(cart == NULL)
		return false;

	bool ret = paused;
	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		if(cart != NULL)
		{		
			ret = paused;
			broken = paused = b;

			if(audio)
			{
				audio->mute();
				(paused) ? audio->pauseSound() : audio->unpauseSound();
			}
		}
		ReleaseMutex(winMutex);
	}
	return ret;
}


bool Gameboy::pause()
{
	if(cart == NULL)
		return false;

	bool ret = paused;
	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		if(cart != NULL)
		{
			ret = paused;
			if(ret == true) step(1);
			broken = paused = !paused;


			if(audio)
			{
				audio->mute();
				(paused) ? audio->pauseSound() : audio->unpauseSound();
			}
			addMessage((paused) ? PAUSED : UNPAUSED, 0);
		}
		ReleaseMutex(winMutex);
	}
	return ret;
}


void Gameboy::setColors(int w, int lg, int dg, int b)
{
	lcd->setColors(w, lg, dg, b);
}


void Gameboy::setTextOverlayColor(int c)
{
	lcd->setOverlayTextColor(c);
}


int Gameboy::getColor(int i)
{
	return lcd->getColor(i);
}


void Gameboy::setEmulationSpeed(int i)
{
	if(i >= 0 && i < SPEED_MAX_INDEX-1)
	{
		speedIndex = i;
		addMessage(EMULATION_SPEED_INCREASED, speeds[speedIndex]);
	}
}


void Gameboy::startRecording(std::string title, std::string author, std::string file, int format)
{
	if(cart == NULL)
		return;

	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		cart->reset();
		z80->reset();
		lcd->reset();
		time->reset();
		audio->reset();
		mem->reset();

		recording = true;
		if(mov != NULL)
			delete mov;

		mov = new Movie(cart->romName.c_str(), author.c_str(), file.c_str(), cart->romName.c_str());
		mov->setFormat((MovieFormat)format);
		ReleaseMutex(winMutex);
	}
}


void Gameboy::stopRecording()
{
	if(recording)
	{
		recording = false;
		mov->record(0, GBE_MOVIE_END, lcd->getFrame());
		mov->writeToDisk(mov->getFormat());
	}
}


void Gameboy::stopPlayback()
{
	playingBack = false;
}


void Gameboy::loadRecording(std::string fileName)
{
	if(cart == NULL)
		return;

	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		cart->reset();
		z80->reset();
		lcd->reset();
		time->reset();
		audio->reset();
		mem->reset();

		if(mov != NULL)
		{
			delete mov;
		}

		mov = Movie::loadFromDisk(fileName);
		playingBack = true;
		ReleaseMutex(winMutex);
	}
}


int Gameboy::rewindSaveInterval(int ms)
{
	if(ms == 0)
		return rwSaveInterval;
	else
		rwSaveInterval = ms;

	return 0;
}


int Gameboy::rewindLoadInterval(int ms)
{
	if(ms == 0)
		return rwLoadInterval;
	else
		rwLoadInterval = ms;

	return 0;
}


int Gameboy::rewindBufferSize(int states)
{
	if(states < 0)
		rewindEnabled = false;
	else if(states == 0)
		return rwBufferSize;
	else if(states != rwBufferSize)
	{
		if(WaitForSingleObject(winMutex, INFINITE) == 0)
		{
			State** nrb = new State*[states];
			for(int i = 0; i < rwBufferSize && i < states ; i++)
			{
				nrb[i] = rb[i];
			}

			for(int i = rwBufferSize; i < states; i++)
			{
				nrb[i] = new State();
			}

			for(int i = states; i < rwBufferSize; i++)
			{
				delete rb[i];
			}
			delete[] rb;
			rb = nrb;
			rwBufferSize = states;
			rewindBufferConstructed = 0;
			rewindBufferNext = 0;
			ReleaseMutex(winMutex);
		}
	}

	return 0;
}


void Gameboy::nextFrame(int numFrames)
{
	if(cart == NULL)
		return;
	paused = false;
	frameAdvance = numFrames;
}


void Gameboy::setInput(byte p10, byte p20)
{
	if(p10 != GB_NO_CHANGE)
		port10 &= p10;
	
	if(p20 != GB_NO_CHANGE)
		port20 &= p20;
}


/////////////////////////////////
//sets the latch for the input ports
//set bits cause the corresponding control to be
//automatically triggered every frame while the
//latch is active
void Gameboy::setInputLatch(byte p10, byte p20)
{
	//input port 10: A, B, Start, Select
	if(p10 != GB_NO_CHANGE)
	{
		if(p10 > 0xf) 
		{
			p10 = p10 & 0xf;
			port10Latch |= p10;
		}
		else
		{
			port10Latch &= p10;
		}
	}
	
	//input port 20: up, down, left, right
	if(p20 != GB_NO_CHANGE)
	{
		if(p20 > 0xf) 
		{
			p20 = p20 & 0xf;
			port20Latch |= p20;
		}
		else
		{
			port20Latch &= p20;
		}
	}
}


/////////////////////////////////
//completely resets the gameboy and all of its components
//retaining the currently loaded cartridge data
void Gameboy::reset()
{
	if(cart == NULL)
		return;

	int status = 0;
	if((status = WaitForSingleObject(winMutex, INFINITE)) == 0)
	{
		cart->reset();
		z80->reset();
		lcd->reset();
		time->reset();
		audio->reset();
		mem->reset();

		ReleaseMutex(winMutex);

		//record input of emulator function
		emuInput = GBE_RESET;
	}
}


/////////////////////////////////
//Toggles the given sound mode on/off
void Gameboy::soundModeToggle(int mode)
{
	if(cart == NULL)
		return;

	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		//enable if currently disabled
		if(audio->isModeEnabled(mode))
		{
			audio->masterDisableMode(mode);
			addMessage(SOUND_MODE_OFF, mode+1);
		}
		//disable if currently enabled
		else
		{
			audio->masterEnableMode(mode);
			addMessage(SOUND_MODE_ON, mode+1);
		}

		ReleaseMutex(winMutex);
	}
}


/////////////////////////////////
//Gets info about the currently loaded rom
RomInfo Gameboy::getRomInfo()
{
	RomInfo ri;
	ZeroMemory(&ri, sizeof(ri));

	if(cart != NULL && cart->isReady())
	{
		ri = cart->getRomInfo();
	}

	return ri;
}


/////////////////////////////////
//Gets the number of valid buffer states
int Gameboy::filledBufferSize()
{
	return rewindBufferConstructed;
}


/////////////////////////////////
//Gets the max size (in states) of the rewind buffer
int Gameboy::totalBufferSize()
{
	return rwBufferSize;
}


void Gameboy::rewindFrames(int numFrames)
{
	if(WaitForSingleObject(winMutex, INFINITE) == 0 && rewindBufferConstructed > 0)
	{
		//index of the next state to load
		int next = (rewindBufferNext == 0) ? rwBufferSize-1 : rewindBufferNext-1;

		restoreState(rb[next]);
		lcd->fullRender();

		//update the rewind buffer
		rewindBufferConstructed--;
		rewindBufferNext = next;
		paused = true;

		ReleaseMutex(winMutex);
	}
}


void Gameboy::saveState()
{
	if(WaitForSingleObject(winMutex, INFINITE) == 0 && rewindConstructTimer > rwSaveInterval)
	{
		constructState(rb[rewindBufferNext % rwBufferSize]);

		//reset timer for the next state save
		rewindConstructTimer = 0;

		//update the rewind buffer with the newest state
		rewindBufferNext = (rewindBufferNext+1) % rwBufferSize;
		rewindBufferConstructed = (rewindBufferConstructed == rwBufferSize) ? rwBufferSize : rewindBufferConstructed + 1;

		ReleaseMutex(winMutex);
	}
}


int Gameboy::getFrameCount()
{
	return lcd->getFrame();
}


int Gameboy::getLagFrames()
{
	return lcd->getLagFrames();
}


/////////////////////////////////
//gets a dump of the current cpu registers
RegisterInfo Gameboy::getRegisterInfo()
{
	return z80->getRegisterInfo();
}


/////////////////////////////////
//gets a byte from the gameboy system ram
byte Gameboy::readMemory(int address)
{
	return mem->read(address);
}


/////////////////////////////////
//gets the value of the specified CPU register
word Gameboy::getRegister(Register r)
{
	return z80->readReg(r);
}


/////////////////////////////////
//gets a byte relative to the stack pointer
word Gameboy::readStack(word offset)
{
	return (mem->readRam(z80->readReg(SP) - (offset-1)) << 8) | (mem->readRam(z80->readReg(SP) - offset));
}


/////////////////////////////////
//gets a byte from the cartridge rom in the specified bank
byte Gameboy::readCartridgeROM(int bank, int address)
{
	return cart->readFromROMBank(bank & 0xff, address & 0xffff);
}


/////////////////////////////////
//gets a byte from the cartridge ram in the specified bank
byte Gameboy::readCartridgeRAM(int bank, int address)
{
	return cart->readFromRAMBank(bank & 0xff, address & 0xffff);
}


/////////////////////////////////
//gets the opcode and the following two bytes
//used to generate the disassembly
Instruction Gameboy::getInstruction(word offset)
{
	Instruction i;
	word address, pc;
	address = pc = z80->readReg(PC);
	
	if(offset < 0 || offset > 1024)
	{
		i.opcode = 0; i.p1 = 0;	i.p2 = 0; i.numP = 0;
		return i;
	}

	for(int j = 0; j < offset; j++)
	{
		address += (opcodeOperands[mem->read(address)] + 1);
	}

	i.opcode = mem->read(address);
	i.address = address;
	i.p1 = mem->read(address+1);
	i.p2 = mem->read(address+2);
	i.numP = opcodeOperands[i.opcode];

	return i;
}


/////////////////////////////////
//advances the emulation by the given number
//of instructions
void Gameboy::step(int numInstructions)
{
	if(WaitForSingleObject(winMutex, INFINITE) == 0 && rewindBufferConstructed > 0)
	{
		broken = true;
		paused = true;
		audio->pauseSound();
		z80->procOpcode();
		time->timer(z80->lastOpClocks);
		lcd->draw();
		lcd->fullRender();
		ReleaseMutex(winMutex);
	}
}


/////////////////////////////////
//returns the most recent record of the
//emulation speed
int Gameboy::getCurrentEmulationSpeed()
{
	return emulationSpeed;
}


/////////////////////////////////
//gets the map of debugger breakpoints
//used to add, remove and check breakpoints
BreakpointMap* Gameboy::getBpm()
{
	return bpm;
}


void Gameboy::addBreakpoint(Breakpoint bp)
{
	bpm->addBreakpoint(bp.address, bp);
}


void Gameboy::removeBreakpoint(word address)
{
	bpm->removeBreakpoint(address);
}


Breakpoint* Gameboy::enumerateBreakpoints(int index)
{
	return bpm->enumerateBreakpoints(index);
}


void Gameboy::debugBreak()
{
	audio->pauseSound();
	broken = true;
	paused = true;
}


void Gameboy::debugContinue()
{
	audio->unpauseSound();
	paused = false;
	broken = false;
}


bool Gameboy::debugIsBroken()
{
	return broken;
}


bool Gameboy::cartridgeIsLoaded()
{
	return !(cart == NULL);
}


void Gameboy::clearBreakpoints()
{
	if(WaitForSingleObject(winMutex, INFINITE) == 0)
	{
		bpm->clearAll();
		ReleaseMutex(winMutex);
	}
}


/////////////////////////////////
//sets up gameboy components to suit the type
//of hardware needed to run the cartridge
void Gameboy::configureGameboyType(GameboyType gbType)
{
	switch(gbType)
	{
/*		case GBC:
			if(mem) delete mem;
			if(lcd) delete lcd;

			mem = new GBCMemoryManager();
			lcd = new GBCLCD();

			((GBCMemoryManager*)mem)->setLCD((GBCLCD*)lcd);
			((GBCLCD*)lcd)->setComponents((GBCMemoryManager*)mem, z80, this);
			((GBCMemoryManager*)mem)->setComponents(z80, cart, time, audio, this);
			break;*/
		default:
		case GB:
			if(mem) delete mem;
			//if(lcd) delete lcd;

			mem = new MemoryManager();
			//lcd = new LCD();


			lcd->setComponents(mem, z80, this);
			mem->setComponents(z80, cart, time, audio, this);
			break;
	};

	lcd->setScale(displayScale);
	z80->setComponents(mem, cart, lcd, this);
	cart->setComponents(mem, z80, this);
	time->setComponents(mem, z80, this);
	audio->setComponents(mem, z80, this);
}



bool Gameboy::getMemoryVisualization(BITMAP** buffer)
{
	bool ret = false;

	//if(WaitForSingleObject(winMutex, INFINITE) == 0 && rewindBufferConstructed > 0)
	//{
		if(!(*buffer))
		{
			*buffer = create_bitmap(256, 256);
		}

		clear_to_color(*buffer, 0);
		if((*buffer)->w >= 256 && (*buffer)->h >= 256)
		{
			for(int i = 0; i < 128; i++)
			{
				for(int j = 0; j < 256; j++)
				{
					byte b = mem->read((i * 256) + j);
					int red = b & 7;
					int green = (b >> 3) & 7;
					int blue = (b >> 3) & 7;
					int color = makecol24(red * 32, green * 32, blue * 16);
					_putpixel24((*buffer), j, i, color);
				}
			}
			for(int i = 128; i < 256; i++)
			{
				for(int j = 0; j < 256; j++)
				{
					byte b = mem->readRam((i * 256) + j);
					int red = b & 7;
					int green = (b >> 3) & 7;
					int blue = (b >> 3) & 7;
					int color = makecol24(red * 32, green * 32, blue * 16);
					_putpixel24((*buffer), j, i, color);
				}
			}

			ret = true;
		}

	//	ReleaseMutex(winMutex);
	//}

	return ret;
}