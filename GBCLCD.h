#pragma once

#include "allegro.h"
#include "enum.h"
#include "GBCMemoryManager.h"

class GBCMemoryManager;

class GBCLCD : public LCD
{
private:
	GBCMemoryManager* gbcmem;
	bool hblankDMA;
public:
	GBCLCD();
	~GBCLCD();
	virtual void setComponents(GBCMemoryManager*, Z80*, Gameboy*);

	bool draw();
	void drawBackground(byte);
	void drawWindow(byte);
	void drawSprites();
	void renderScanline(byte);
	void vblank(byte ly);

	void constructState(State*);
	void restoreState(State*);
	void reset();
	void startHBlankDMA();
};
