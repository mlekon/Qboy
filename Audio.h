#ifndef AUDIO_H
#define AUDIO_H

#include "allegro.h"
#include "MemoryManager.h"
#include "enum.h"
#include "IGameBoyComponent.h"

#define MAX_WAVEFORM_DATA 32
#define WAVEFORM_BASE_ADDRESS 0xff30
#define AUDIO_BITS 8



class Audio : IGameBoyComponent
{
private:
	static const double msPerClock;
	static const double msPerVal;
	static const double sweepTimes[8];
	static const double dutyTimeOn[4];
	static const double volumeCoefficient;
	static const double sampleCoefficient;
	static const double dutyCycleLength;
	static const double numModes;
	static const double clocksPerSweepStep;
	static const double clocksPerEnvelopeStep;
	static const byte defaultWaveDuty;

	struct AudioLog
	{
		int audioRegister;
		int value;
	} audioDebugLog[256];

	enum
	{
		mode1 = 0,
		mode2 = 1,
		mode3 = 2,
		mode4 = 3,
		modeAll = 4
	};

	enum OutputTerminal
	{
		SO1 = (1 << 0),
		SO2 = (1 << 1)
	};

	int debugLogPosition;

	MemoryManager* mem;
	Z80* z80;

	SAMPLE* modeSamples[4];
	int modeVoices[4];
	byte modeOutputTerminals[4];

	bool muted;
	double masterVolume;
	int gvolmidi;
	int gvoldigi;

	int modeTimers[4];
	int modeTimeSteps[4];
	bool modeUseTimer[4];
	bool modePlaying[4];
	bool modeMasterEnable[4];
	byte modeAmplitudes[4];

	bool mode1SweepActive;
	int mode1SweepTimer;
	byte mode1SweepInitial;
	byte mode1SweepNum;
	word mode1ShadowRegister;
	bool mode1EnvelopeActive;
	int mode1EnvelopeTimer;
	byte mode1EnvelopeInitial;
	byte mode1EnvelopeNum;

	byte mode1Sweep;						//NR10
	byte mode1Duty;						//NR11
	byte mode1Envelope;						//NR12
	byte mode1FreqLow;						//NR13
	byte mode1FreqHi;						//NR14
	word mode1Frequency;
	byte mode1EnvelopeVolume;

	bool mode2EnvelopeActive;
	int mode2EnvelopeTimer;
	byte mode2EnvelopeVolume;
	byte mode2EnvelopeSteps;
	byte mode2EnvelopeInitial;
	byte mode2EnvelopeNum;

	byte mode2Duty;						//NR21
	byte mode2Envelope;						//NR22
	byte mode2FreqLow;						//NR23
	byte mode2FreqHi;						//NR24
	word mode2Frequency;

	byte mode3On;							//NR30
	byte mode3Length;						//NR31
	byte mode3Output;						//NR32
	byte mode3FreqLow;						//NR33
	byte mode3FreqHi;						//NR34
	byte mode3Shifts;
	word mode3Frequency;
	int mode3FrequencyTimer;
	byte mode3DataCounter;
	byte waveformData[MAX_WAVEFORM_DATA];

	byte mode4Length;						//NR41
	byte mode4Envelope;						//NR42
	byte mode4Poly;							//NR43
	byte mode4Count;						//NR44
	bool mode4EnvelopeActive;
	int mode4EnvelopeTimer;
	byte mode4EnvelopeVolume;
	byte mode4Pattern;
	byte mode4EnvelopeSteps;
	byte mode4Frequency;

	byte channelControl;					//NR50
	byte SOmodes;							//NR51
	byte modeStatus;						//NR52

	byte nr10, nr11, nr12, nr13, nr14;
	byte nr20, nr21, nr22, nr23, nr24;
	byte nr30, nr31, nr32, nr33, nr34;
	byte nr41, nr42, nr43, nr44;
	byte nr50, nr51, nr52;
public:
	Audio();
	~Audio();

	void setComponents(MemoryManager* m, Z80* z, Gameboy* gb);
	void adjustWaveformData(word addr, byte val);
	void adjustWaveDuty(int mode, byte duty);
	void loadWaveformData(int mode);

	byte getWaveformDataByte(byte b);
	byte setSoundRegister(word addr, byte val);
	void setWaveformSample(byte sample);
	void shiftMode3Data(byte);
	void startMode(int mode);
	void stopMode(int mode);
	void sound(int elapsedCycles);
	void pauseSound();
	void unpauseSound();
	void setVolume(double v);
	void disableSound();
	void enableSound();
	double getVolume();
	bool mute();

	void procMode1(int elapsedClocks);
	void procMode2(int elapsedClocks);
	void procMode3(int elapsedClocks);
	void procMode4(int elapsedClocks);

	int gbToPhysicalFreq(word gbFreq, int mode);

	void masterEnableMode(int mode);
	void masterDisableMode(int mode);
	bool isModeEnabled(int mode);

	void constructState(State*);
	void restoreState(State*);
	void reset();
};

#endif