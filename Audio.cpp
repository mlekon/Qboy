#include "Audio.h"


const double Audio::msPerClock				= (1/4194.304);
const double Audio::msPerVal				= 39.0625;
const double Audio::sweepTimes[8]			= {0, 8, 16, 23, 31, 39, 47, 55};
const double Audio::volumeCoefficient		= 255 / 7;
const double Audio::sampleCoefficient		= 16;
const double Audio::dutyCycleLength			= 8;
const double Audio::dutyTimeOn[4]			= {1, 2, 4, 6};
const double Audio::numModes				= 4;
const double Audio::clocksPerSweepStep		= 7.8 * 4194.304;
const double Audio::clocksPerEnvelopeStep	= 65536;
const byte Audio::defaultWaveDuty			= 2;

Audio::Audio()
{
	muted = false;
	masterVolume = 0.1;
	channelControl = 0x77;
	SOmodes = 0;

	srand(time(0));

#ifdef DEBUG
	debugLogPosition = 0;
#endif

	for(byte i = 0; i < numModes; i++)
	{
		modeSamples[i] = create_sample(8, 0, 1024, MAX_WAVEFORM_DATA);
		modeVoices[i] = allocate_voice(modeSamples[i]);
	}


	for(int i = 0; i < numModes; i++)
	{
		modeTimers[i] = 0;
		modeUseTimer[i] = false;
		modePlaying[i] = false;
		modeAmplitudes[i] = 0;
	}
	modeAmplitudes[mode3] = 16;

	modeMasterEnable[0] = true;
	modeMasterEnable[1] = true;
	modeMasterEnable[2] = true;
	modeMasterEnable[3] = false;

	mode1SweepActive = false;
	mode1SweepTimer = 0;
	mode1SweepInitial = 0;
	mode1SweepNum = 0;
	mode1EnvelopeActive = false;
	mode1EnvelopeTimer = 0;
	mode1EnvelopeInitial = 0;
	mode1EnvelopeNum = 0;
	mode1Frequency = 0;

	mode1Frequency = 0;
	mode1Sweep = 0x80;
	mode1Duty = 1;
	mode1Envelope = 0xf3;
	mode1FreqLow = 0;
	mode1FreqHi = 0xbf;

	mode2EnvelopeActive = false;
	mode2EnvelopeTimer = 0;
	mode2EnvelopeVolume = 0;
	mode2EnvelopeSteps = 0;
	mode2Frequency = 0;

	mode2Duty = 1;
	mode2Envelope = 0;
	mode2FreqLow = 0;
	mode2FreqHi = 0xbf;

	mode3On = 0x7f;
	mode3Length = 0xff;
	mode3Output = 0x9f;
	mode3FreqLow = 0xbf;
	mode3FreqHi = 0;
	mode3Frequency = 0xbf;
	mode3FrequencyTimer = 0;
	mode3DataCounter = 0;
	
	mode4Length = 0xff;
	mode4Envelope = 0;
	mode4Poly = 0;
	mode4Count = 0xbf;

	modeStatus = 0xf0;
}


Audio::~Audio()
{
	for(int i = 0; i < numModes; i++)
		deallocate_voice(modeVoices[i]);
}


void Audio::setComponents(MemoryManager* m, Z80* z, Gameboy* gb)
{
	this->gb = gb;
	z80 = z;
	mem = m;

	for(int i = 0; i < numModes; i++)
	{
		loadWaveformData(i);
		voice_set_playmode(modeVoices[i], PLAYMODE_LOOP);
	}

	setSoundRegister(NR51, 0xf3);
}

void Audio::adjustWaveformData(word addr, byte val)
{
	byte bytePosition = addr - WAVEFORM_BASE_ADDRESS;
	byte highSample = (val >> 4) >> mode3Shifts;
	byte lowSample = (val & 0xf) >> mode3Shifts;

	((char*)modeSamples[mode3]->data)[bytePosition * 2] = (lowSample >> 5) * sampleCoefficient;
	((char*)modeSamples[mode3]->data)[bytePosition * 2 - 1] = (highSample >> 5) * sampleCoefficient;
}


void Audio::setWaveformSample(byte sample)
{
	byte b = (sample & 0xf);

	switch((nr32 >> 5) & 3)
	{
	case 0:
		b = 0;
		break;
	case 2:
		b >>= 1;
		break;
	case 3:
		b >>= 2;
		break;
	};

	for(int i = 0; i < MAX_WAVEFORM_DATA;)
	{

		((char*)modeSamples[mode3]->data)[i++] = b * sampleCoefficient;
		((char*)modeSamples[mode3]->data)[i++] = b * sampleCoefficient;
		((char*)modeSamples[mode3]->data)[i++] = b * sampleCoefficient;
		((char*)modeSamples[mode3]->data)[i++] = b * sampleCoefficient;
		((char*)modeSamples[mode3]->data)[i++] = 0;
		((char*)modeSamples[mode3]->data)[i++] = 0;
		((char*)modeSamples[mode3]->data)[i++] = 0;
		((char*)modeSamples[mode3]->data)[i++] = 0;
	}
}

void Audio::loadWaveformData(int mode)
{
	if(mode == mode3)
		for(int j = 0; j < MAX_WAVEFORM_DATA; j++)
			((byte*)modeSamples[mode]->data)[j] = (getWaveformDataByte(j) >> mode3Shifts) * sampleCoefficient;
	else
		adjustWaveDuty(mode, (mode == mode1) ? mode1Duty : mode2Duty);
}


void Audio::adjustWaveDuty(int mode, byte duty)
{
	int i = 0;
	for(; i < MAX_WAVEFORM_DATA;)
	{
		int j = 0;
		for(; j < dutyTimeOn[duty & 3]; j++)
			((byte*)modeSamples[mode]->data)[i++] = modeAmplitudes[mode] * sampleCoefficient + rand()%3;	
		for(; j < dutyCycleLength; j++)
			((byte*)modeSamples[mode]->data)[i++] = rand()%3;
	}
}


byte Audio::getWaveformDataByte(byte b)
{
	if(b & 1)
		//get the low sample
		return mem->readRam(WAVEFORM_BASE_ADDRESS + (b / 2)) & 0xf;
	else
		//get the high sample
		return mem->readRam((WAVEFORM_BASE_ADDRESS + (b / 2))) >> 4;
}


void Audio::startMode(int mode)
{
	if(mode == modeAll)
		for(int i = 0; i < numModes; i++)
		{
			if(modeMasterEnable[i] && !muted)
			{
				voice_start(modeVoices[i]);
				modePlaying[i] = true;
				modeStatus &= (1 << mode);
				mem->writeRam(NR52, modeStatus);
			}
		}
	else
	{
		if(modeMasterEnable[mode] && !muted)
		{
			voice_start(modeVoices[mode]);
			modePlaying[mode] = true;
			modeStatus &= (1 << mode);
			mem->writeRam(NR52, modeStatus);
		}
	}
}


void Audio::stopMode(int mode)
{
	if(mode == modeAll)
		for(int i = 0; i < numModes; i++)
		{
			voice_stop(modeVoices[i]);
			modePlaying[i] = false;
			modeStatus &= (0xff - (1 << i));
		}
	else
	{
		voice_stop(modeVoices[mode]);
		modePlaying[mode] = false;
		voice_set_position(modeVoices[mode], 0);
		modeStatus &= (0xff - (1 << mode));
	}
}


void Audio::shiftMode3Data(byte val)
{
	mode3Shifts = 0;
	switch((val >> 5) & 3)
	{
	case 0:
		mode3Shifts = 8;
		break;
	case 1:
		mode3Shifts = 0;
		break;
	case 2:
		mode3Shifts = 1;
		break;
	case 3:
		mode3Shifts = 2;
		break;
	}

	byte* mode3Data = (byte*)modeSamples[mode3]->data;
	for(int i = 0; i < MAX_WAVEFORM_DATA; i++)
		mode3Data[i] = (getWaveformDataByte(i) >> mode3Shifts) * sampleCoefficient;
}


/////////////////////////////////
//alters the sound output
//each bit of each sound register has a specific purpose
//each bit is described as [#] where # is the number of the
//description which applies to that bit. [X] indicates
//that the bit is not used
byte Audio::setSoundRegister(word addr, byte val)
{
	switch(addr)
	{
	case NR10:
		//[X][1][1][1][2][3][3][3]
		//1: Sweep time (period). 7.8 ms * value
		//2: Direction of sweep, 0 = add, 1 = subtract
		//3: Number of shifts
		mode1Sweep = nr10 = val;
		mode1SweepInitial = mode1SweepNum = val & 7;
		mode1SweepActive = (val >> 4) != 0;
		mode1SweepTimer = clocksPerSweepStep * (val & 7);
		break;
	case NR11:
		//[1][1][2][2][2][2][2][2]
		//1: wave pattern duty, 12.5% * value
		//2: sound length, (64 - value) * (1/256) seconds
		if((val >> 6) != mode1Duty)
			adjustWaveDuty(mode1, val >> 6);
		nr11 = val;
		mode1Duty = val >> 6;
		modeTimers[mode1] = (64.0f - float(val & 0x3f)) * (1.0f/256.0f) * 4194304.0f;
		break;
	case NR12:
		//[1][1][1][1][2][3][3][3]
		//1: Initial envelope value
		//2: Envelope direction, 0 = decrease, 1 = increase
		//3: Envelope period, value * (1/64) seconds
		mode1Envelope = nr12 = val;
		mode1EnvelopeTimer = 0;
		modeAmplitudes[mode1] = mode1Envelope >> 4;

		if((val & 7) == 0)
		{
			mode1EnvelopeActive = false;
			mode1EnvelopeInitial = mode1EnvelopeVolume = 0;
		}
		else
		{
			mode1EnvelopeActive = true;
			mode1EnvelopeInitial = mode1EnvelopeVolume = val >> 4;
			mode1EnvelopeNum = val & 7;
			adjustWaveDuty(mode1, mode1Duty);
		}
		break;
	case NR13:
		//[1][1][1][1][1][1][1][1]
		//1: lower 8 bits of sound frequency
		mode1FreqLow = nr13 = val;
		mode1Frequency = (mode1Frequency & 0x700) + val;
		voice_set_frequency(modeVoices[mode1], gbToPhysicalFreq(mode1Frequency, mode1));
		break;
	case NR14:
		//[1][2][X][X][X][3][3][3]
		//1: reset the sound when set
		//2: flag for using sound length, 1 = stop sound when timer finishes
		//3: upper 3 bits of the sound frequency
		nr14 = val;
		mode1FreqHi = nr14 & 7;
		modeUseTimer[mode1] = (val & 0x40) == 1;
		mode1Frequency = (mode1Frequency & 0xff) + ((val & 7) << 8);
		voice_set_frequency(modeVoices[mode1], gbToPhysicalFreq(mode1Frequency, mode1));

		if(val & 0x80)
		{
			modeAmplitudes[mode1] = 0;
			adjustWaveDuty(mode1, mode1Duty);

			if((nr11 & 0x3f) == 0)
			{
				nr11 = (nr11 & 0xc0) | 0x3f;
				mode1EnvelopeTimer = 0;
				modeAmplitudes[mode1] = nr12 >> 4;
				adjustWaveDuty(mode1, mode1Duty);
			}
			//mode1SweepActive = false;
			//mode1EnvelopeActive = false;
			//mode1SweepNum = mode1SweepInitial;
			//modeAmplitudes[mode1] = mode1EnvelopeInitial;
			//modeTimers[mode1] = 64.0f * (1.0f/256.0f) * 4194304.0f;
		
			//mode1ShadowRegister = mode1Frequency;
			//mode1SweepTimer = clocksPerSweepStep * (mode1SweepNum & 7);
			//if(mode1SweepNum != 0)
			//{
			//	mode1Frequency = mode1Frequency + ((mode1Sweep & 8) ? -1 : 1) * (mode1Frequency / (2 << mode1SweepInitial));
			//	//mode1SweepNum--;
			//	if(mode1Frequency > 0xfff)
			//	{
			//		modeStatus &= 0xfe;
			//		mem->writeRam(NR52, modeStatus);
			//	}
			//	mode1SweepTimer -= clocksPerSweepStep * ((mode1Sweep >> 4) & 7);
			//	mode1FreqLow = mode1Frequency & 0xff;
			//	mem->writeRam(NR13, mode1FreqLow);
			//	mode1FreqHi = (mode1FreqHi & 0xf8) + ((mode1Frequency >> 8) & 7);
			//	mem->writeRam(NR14, mode1FreqHi);
			//	voice_set_frequency(modeVoices[mode1], gbToPhysicalFreq(mode1Frequency, mode1));
			//}
		}
		break;
	case NR21:
		//[1][1][2][2][2][2][2][2]
		//1: wave pattern duty, 12.5% * value
		//2: sound length, (64 - value) * (1/256) seconds
		if((val >> 6) != mode2Duty)
			adjustWaveDuty(mode2, (val >> 6));
		nr12 = val;
		mode2Duty = val >> 6;
		modeTimers[mode2] = ((64.0f - float(val & 0x3f)) * (1.0f/256.0f)) * 4194304.0f;
		break;
	case NR22:
		//[1][1][1][1][2][3][3][3]
		//1: Initial envelope value
		//2: Envelope direction, 0 = decrease, 1 = increase
		//3: Envelope period, value * (1/64) seconds
		mode2Envelope = nr22 = val;
		mode2EnvelopeTimer = 0;
		modeAmplitudes[mode2] = mode2Envelope >> 4;

		if((val & 7) == 0)
		{
			mode2EnvelopeActive = false;
			mode2EnvelopeVolume = 0;
		}
		else
		{
			mode2EnvelopeActive = true;
			mode2EnvelopeVolume = mode2EnvelopeInitial = val >> 4;
			mode2EnvelopeSteps = val & 7;
			adjustWaveDuty(mode2, mode2Duty);
		}
		break;
	case NR23:
		//[1][1][1][1][1][1][1][1]
		//1: lower 8 bits of sound frequency
		mode2FreqLow = nr23 = val;
		mode2Frequency = (mode2Frequency & 0x700) | val;
		voice_set_frequency(modeVoices[mode2], gbToPhysicalFreq(mode2Frequency, mode2));
		break;
	case NR24:
		//[1][2][X][X][X][3][3][3]
		//1: reset the sound when set
		//2: flag for using sound length, 1 = stop sound when timer finishes
		//3: upper 3 bits of the sound frequency
		nr24 = val;
		mode2FreqHi = nr24 & 7;
		mode2Frequency = ((val & 7) << 8) | (mode2Frequency & 0xff);
		modeUseTimer[mode2] = (val & 0x40) == 1;
		voice_set_frequency(modeVoices[mode2], gbToPhysicalFreq(mode2Frequency, mode2));
		
		if(val & 0x80)
		{
			if((nr21 & 0x3f) == 0)
			{
				nr21 = (nr21 & 0xc0) | 0x3f;
				mode2EnvelopeTimer = 0;
				modeAmplitudes[mode2] = nr22 >> 4;
				adjustWaveDuty(mode2, mode2Duty);
			}
		}
		break;
	case NR30:
		//[1][X][X][X][X][X][X][X]
		//1: output sound flag. 0 = stop, 1 = ok
		nr30 = val;
		if((val & 0x80) == 0 && mode3On)
			stopMode(mode3);
		else if((val & 0x80) != 0 && !mode3On)
			startMode(mode3);
		mode3On = (val & 0x80) == 1;
		break;
	case NR31:
		//[1][1][1][1][1][1][1][1]
		//1: sound length, (256 - value) * (1/256) seconds
		mode3Length = nr31 = val;
		modeTimers[mode3] = ((256.0f - (float)val) * (1.0f/256.0f)) * 4194304.0f;
		break;
	case NR32:
		//[X][1][1][X][X][X][X][X]
		//1: output shifted waveform data
		mode3Output = nr32 = val;
		break;
	case NR33:
		//[1][1][1][1][1][1][1][1]
		//1: lower 8 bits of the sound frequency
		mode3FreqLow = nr33 = val;
		mode3Frequency = (mode3Frequency & 0x700) | val;
		voice_set_frequency(modeVoices[mode3], gbToPhysicalFreq(mode3Frequency, mode3));
		break;
	case NR34:
		//[1][2][X][X][X][3][3][3]
		//1: reset the sound when set
		//2: flag for using sound length, 1 = stop sound when timer finishes
		//3: upper 3 bits of the sound frequency
		mode3FreqHi = nr34 = val;
		mode3Frequency = ((val & 7) << 8) | (mode3Frequency & 0xff);
		if(val & 0x80)
		{
			mode3DataCounter = 0;
			mode3FrequencyTimer = clocksPerSecond / gbToPhysicalFreq(mode3Frequency, mode3);
		}

		modeUseTimer[mode3] = (val & 0x40) == 1;
		voice_set_frequency(modeVoices[mode3], gbToPhysicalFreq(mode3Frequency, mode3));
		break;
	case NR41:
		//[X][X][1][1][1][1][1][1]
		//1: sound length
		nr41 = val;
		break;
	case NR42:
		//[1][1][1][1][2][3][3][3]
		//1: Initial envelope value
		//2: Envelope direction, 0 = decrease, 1 = increase
		//3: Envelope period, value * (1/64) seconds
		mode4Envelope = nr42 = val;
		mode4EnvelopeTimer = 0;

		if((val & 7) == 0)
		{
			mode4EnvelopeActive = false;
			mode4EnvelopeVolume = 0;
		}
		else
		{
			mode4EnvelopeActive = true;
			mode4EnvelopeVolume = val >> 4;
			mode4EnvelopeSteps = val & 7;
			modeAmplitudes[mode4] = mode4Envelope >> 4;
			adjustWaveDuty(mode4, mode4Pattern >> 6);
		}
		break;
	case NR43:
		//[1][1][1][1][2][3][3][3]
		//1: selection of the shift clock frequency for the polynomial counter 
		//2: selection of polynomial counter's step
		//3: selection of frequency ratio
		nr43 = val;
		mode4Frequency = gbToPhysicalFreq(val, mode4);
		break;
	case NR44:
		//[1][2][X][X][X][X][X][X]
		//1: reset the sound when set
		//2: flag for using sound length, 1 = stop sound when timer finishes
		nr44 = val;
		break;
	case NR50:
		//[1][2][2][2][3][4][4][4]
		//1: Vin -> SO2 on/off
		//2: SO2 output volume
		//3: Vin -> SO1 on/off
		//4: SO1 output volume
		nr50 = val;
		if((val & 0x70) != (channelControl & 0x70))
		{
			for(int i = 0; i < numModes; i++)
			{
				if(modeOutputTerminals[i] & SO2)
					voice_set_volume(modeVoices[i], ((val >> 4) & 7) * 32 * masterVolume);
			}
		}

		if((val & 0x7) != (channelControl & 0x7))
		{
			for(int i = 0; i < numModes; i++)
			{
				if(modeOutputTerminals[i] & SO1)
					voice_set_volume(modeVoices[i], (val & 7) * 32 * masterVolume);
			}
		}

		channelControl = val;
		break;
	case NR51:
		//[1][2][3][4][5][6][7][8]
		//1: output sound 1 to SO1
		//2: output sound 2 to SO1
		//3: output sound 3 to SO1
		//4: output sound 4 to SO1
		//5: output sound 1 to SO2
		//6: output sound 2 to SO2
		//7: output sound 3 to SO2
		//8: output sound 4 to SO2
		//for(int i = 0; i < numModes; i++)
		//{
		//	modeOutputTerminals[i] = (val & (1 << i)) | (val & (1 << (i + 4)));
		//	(modeOutputTerminals[i] == 0) ? stopMode(i) : startMode(i); 
		//}
		SOmodes = nr51 = val;
		break;
	case NR52:
		//[1][X][X][X][2][3][4][5]
		//1: all sound on/off
		//2: sound 4 on/off
		//3: sound 3 on/off
		//4: sound 2 on/off
		//5: sound 1 on/off
		if(!(val & 0x80))
		{
			mem->writeRam(NR51, 0);
			stopMode(modeAll);
			SOmodes = 0;
		}
		else
		{
			startMode(modeAll);
		}
		modeStatus = nr52 = val;
		break;
	default:
		//write to waveform data (0xff30 - 0xff3f)
		adjustWaveformData(addr, val);
		break;
	}

	sound(z80->getClockCounter());
	z80->resetClockCounter();

#ifdef DEBUG
	audioDebugLog[debugLogPosition].audioRegister = addr;
	audioDebugLog[debugLogPosition].value = val;

	debugLogPosition = (debugLogPosition + 1) % 256;
	std::cout << addr << " | " << val << std::endl;
#endif

	return val;
}


void Audio::sound(int elapsedCycles)
{
	procMode1(elapsedCycles);
	procMode2(elapsedCycles);
	procMode3(elapsedCycles);
	procMode4(elapsedCycles);
}


void Audio::procMode1(int elapsedClocks)
{
	if(mode1SweepActive)
	{
		mode1SweepTimer -= elapsedClocks;
		if(mode1SweepTimer <= 0 && mode1SweepNum != 0)
		{
			mode1ShadowRegister = mode1Frequency = mode1Frequency + ((mode1Sweep & 8) ? -1 : 1) * (mode1Frequency / (2 << mode1SweepInitial));
			mode1FreqLow = mode1Frequency & 0xff;
			mem->writeRam(NR13, mode1FreqLow);
			mode1FreqHi = (mode1FreqHi & 0xf8) + ((mode1Frequency >> 8) & 7);
			mem->writeRam(NR14, mode1FreqHi);
			mode1SweepNum--;

			if(mode1Frequency > 0xfff)
			{
				modeStatus &= 0xfe;
				mem->writeRam(NR52, modeStatus);
				mode1SweepTimer = false;
			}

			//voice_set_frequency(modeVoices[mode1], gbToPhysicalFreq(mode1Frequency, mode1));
			mode1SweepTimer -= clocksPerSweepStep * ((mode1Sweep >> 4) & 7);
			mode1FreqLow = mode1Frequency & 0xff;
			mem->writeRam(NR13, mode1FreqLow);
			mode1FreqHi = (mode1FreqHi & 0xf8) + ((mode1Frequency >> 8) & 7);
			mem->writeRam(NR14, mode1FreqHi);
		}
		else if(mode1SweepNum == 0)
		{
			mode1SweepTimer = false;
			mode1SweepActive = false;
		}
	}


	if(mode1EnvelopeActive)
	{
		mode1EnvelopeTimer += elapsedClocks;
		if(mode1EnvelopeTimer >= (mode1Envelope & 7) * clocksPerEnvelopeStep&& (nr12 & 7) >= 0)
		{
			mode1EnvelopeVolume += ((mode1Envelope & 8) ? 1  : -1);
			modeAmplitudes[mode1] = mode1EnvelopeVolume;
			mode1EnvelopeTimer -= (mode1Envelope & 7) * clocksPerEnvelopeStep;
			(nr12 & 7) ? nr12-- : nr12;
			mem->writeRam(NR12, nr12);

			if(mode1EnvelopeVolume > 0 && mode1EnvelopeVolume < 0xf)
				adjustWaveDuty(mode1, mode1Duty);
			else
			{
				mode1EnvelopeActive = false;
				mode1EnvelopeVolume = 0;
			}
		}
	}


	/////////// mode 1 //////////////
	if(modeUseTimer[mode1])
	{
		modeTimers[mode1] -= elapsedClocks;
		if(modeTimers[mode1] <= 0)
		{
			modeUseTimer[mode1] = false;
			modeStatus = modeStatus & 0xfe;
			modeTimers[mode1] = 0;
			mem->writeRam(NR52, modeStatus);
			stopMode(mode1);
		}
	}
}


void Audio::procMode2(int elapsedClocks)
{
	if(mode2EnvelopeActive)
	{
		mode2EnvelopeTimer += elapsedClocks;
		if(mode2EnvelopeTimer >= (mode2Envelope & 7) * clocksPerEnvelopeStep && (nr22 & 7) >= 0)
		{
			modeAmplitudes[mode2] = mode2EnvelopeVolume;
			mode2EnvelopeVolume += ((mode2Envelope & 8) ? 1 : -1);
			mode2EnvelopeTimer -= (mode2Envelope & 7) * clocksPerEnvelopeStep;
			(nr22 & 7) ? nr22-- : nr22;
			mem->writeRam(NR22, nr22);

			if(mode2EnvelopeVolume > 0 && mode2EnvelopeVolume < 0xf)
				adjustWaveDuty(mode2, mode2Duty);
			else
			{
				mode2EnvelopeActive = false;
				mode2EnvelopeVolume = 0;
			}
		}
	}

	///////////// mode 2 //////////////
	if(modeUseTimer[mode2])
	{
		modeTimers[mode2] -= elapsedClocks;
		if(modeTimers[mode2] <= 0)
		{
			modeUseTimer[mode2] = false;
			modeStatus = modeStatus & 0xfd;
			modeTimers[mode2] = 0;
			mem->writeRam(NR52, modeStatus);
			stopMode(mode2);
		}
	}
}


void Audio::procMode3(int elapsedClocks)
{
	/////////// mode 3 //////////////
	if(modeUseTimer[mode3])
	{
		modeTimers[mode3] -= elapsedClocks;
		if(modeTimers[mode3] <= 0)
		{
			modeUseTimer[mode3] = false;
			modeStatus = modeStatus & 0xfb;
			modeTimers[mode3] = 0;
			mem->writeRam(NR52, modeStatus);
			voice_set_volume(modeVoices[mode3], 0);
		}
	}

	if(nr30 & 0x80)
	{
		mode3FrequencyTimer -= elapsedClocks;
		if(mode3FrequencyTimer <= 0)
		{
			//clocksPerSecond / gbToPhysicalFreq(mode3Frequency, mode3)		????
			mode3DataCounter = (mode3DataCounter + 1) % MAX_WAVEFORM_DATA;
			mode3FrequencyTimer += clocksPerSecond / gbToPhysicalFreq(mode3Frequency, mode3); //(2048 - mode3Frequency) * 2;
			setWaveformSample(getWaveformDataByte(mode3DataCounter));
		}
	}
}


void Audio::procMode4(int elapsedClocks)
{
	/////////// mode 4 //////////////
	if(modeUseTimer[mode4])
	{
		modeTimers[mode4] -= elapsedClocks;
		if(modeTimers[mode4] <= 0)
		{
			modeUseTimer[mode4] = false;
			modeStatus = modeStatus & 0xfe;
			modeTimers[mode4] = 0;
			mem->writeRam(NR52, modeStatus);
			stopMode(mode4);
		}
	}

	if(mode4EnvelopeActive)
	{
		mode4EnvelopeTimer += elapsedClocks;
		if(mode4EnvelopeTimer >= (mode4Envelope & 7) * clocksPerEnvelopeStep)
		{
			modeAmplitudes[mode4] = mode4EnvelopeVolume;
			mode4EnvelopeVolume += ((mode4Envelope & 8) ? 1 : -1);
			mode4EnvelopeTimer -= (mode4Envelope & 7) * clocksPerEnvelopeStep;

			if(mode4EnvelopeVolume > 0 && mode4EnvelopeVolume < 0xf)
				adjustWaveDuty(mode4, mode4Pattern >> 6);
			else
			{
				mode4EnvelopeActive = false;
				mode4EnvelopeVolume = 0;
			}
		}
	}
}


void Audio::pauseSound()
{
	muted = true;
	stopMode(modeAll);
}


void Audio::unpauseSound()
{
	muted = false;
	startMode(modeAll);
}


int Audio::gbToPhysicalFreq(word gbFreq, int mode)
{
	switch(mode)
	{
	case mode1:
	case mode2:
		return 8 * (4194302 / (32 * (2048 - gbFreq)));
	case mode3:
		return 8 * (65536 / (2048 - (gbFreq)));
	case mode4:
		return 524288 / (float)((gbFreq & 3) ? (gbFreq & 3) : 0.5f) / pow(2, (float)(gbFreq >> 4) + 1);
	}
	return 0;
}


void Audio::enableSound()
{
	for(int i = 0; i < numModes; i++)
		modeMasterEnable[i] = true;
}


void Audio::disableSound()
{
	for(int i = 0; i < numModes; i++)
		modeMasterEnable[i] = false;
}


bool Audio::mute()
{
	muted = !muted;
	if(muted)
		pauseSound();
	else
		unpauseSound();

	return muted;
}


void Audio::setVolume(double v)
{
	if(v > 1)
		v = 1;
	else if(v < 0)
		v = 0;

	masterVolume = v;

	for(int i = 0; i < 4; i++)
	{
		
		voice_set_volume(modeVoices[i], masterVolume * 255);
	}
	muted = false;
	unpauseSound();
}


double Audio::getVolume()
{
	return masterVolume;
}


void Audio::masterEnableMode(int mode)
{
	if(mode >= 0 && mode <= mode4)
	{
		modeMasterEnable[mode] = true;
		startMode(mode);
	}
}


void Audio::masterDisableMode(int mode)
{
	if(mode >= 0 && mode <= mode4)
	{
		modeMasterEnable[mode] = false;
		stopMode(mode);
	}
}


bool Audio::isModeEnabled(int mode)
{
	if(mode >= 0 && mode <= mode4)
	{
		return modeMasterEnable[mode];
	}

	return false;
}


void Audio::constructState(State* s)
{
}


void Audio::restoreState(State* s)
{
}


void Audio::reset()
{
	for(int i = 0; i < numModes; i++)
	{
		modeTimers[i] = 0;
		modeUseTimer[i] = false;
		modePlaying[i] = false;
		modeAmplitudes[i] = 0;
	}
	modeAmplitudes[mode3] = 16;

	modeMasterEnable[0] = true;
	modeMasterEnable[1] = true;
	modeMasterEnable[2] = true;
	modeMasterEnable[3] = false;

	mode1SweepActive = false;
	mode1SweepTimer = 0;
	mode1SweepInitial = 0;
	mode1SweepNum = 0;
	mode1EnvelopeActive = false;
	mode1EnvelopeTimer = 0;
	mode1EnvelopeInitial = 0;
	mode1EnvelopeNum = 0;
	mode1Frequency = 0;

	mode1Frequency = 0;
	mode1Sweep = 0x80;
	mode1Duty = 1;
	mode1Envelope = 0xf3;
	mode1FreqLow = 0;
	mode1FreqHi = 0xbf;

	mode2EnvelopeActive = false;
	mode2EnvelopeTimer = 0;
	mode2EnvelopeVolume = 0;
	mode2EnvelopeSteps = 0;
	mode2Frequency = 0;

	mode2Duty = 1;
	mode2Envelope = 0;
	mode2FreqLow = 0;
	mode2FreqHi = 0xbf;

	mode3On = 0x7f;
	mode3Length = 0xff;
	mode3Output = 0x9f;
	mode3FreqLow = 0xbf;
	mode3FreqHi = 0;
	mode3Frequency = 0xbf;

	mode4Length = 0xff;
	mode4Envelope = 0;
	mode4Poly = 0;
	mode4Count = 0xbf;

	modeStatus = 0xf0;
}