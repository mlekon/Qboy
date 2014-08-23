#ifndef LCD_H
#define LCD_H

#include <list>
#include <string>
#include "allegro.h"
#include "enum.h"
#include "MemoryManager.h"
#include "IGameBoyComponent.h"
#include "opencv2\opencv.hpp"

using namespace cv;

#define DEFAULT_SCALE	2

/*
	GB actual/virtual screen :
					255
	----------166--------------------
	|					  |			|
	|					  |			|
	|					  |			|
	|					  143		|
	|					  |			|
	|					  |			|
	|					  |			|255
	|----------------------			|
	|								|
	|								|
	|								|
	|								|
	|								|
	---------------------------------
	//70,221 v sync time
	//4613 vblank duration
	//65608 non-vblank time
	//429 h-sync time
*/


enum ColorIndex
{
	COLOR_WHITE = 0,
	COLOR_LIGHT_GRAY = 1,
	COLOR_DARK_GRAY = 2,
	COLOR_BLACK = 3
};

enum LCD_Mode
{
	MODE0,
	MODE1,
	MODE2,
	MODE3,
	MODE_VB_RENDER,
	MODE_HL_DONE
};

class LCD : public IGameBoyComponent
{
protected:
	struct VideoStateComponent : StateComponent
	{
		bool oamAccess;						//is oam ram accessible?
		bool vramAccess;					//is vram accessible?
		bool memoryMode;					//memory mode of MBC1 carts
		int lcdHorizontalPhase;				//current mode of the LCD
	};

	VideoWriter* videoWriter;
	Mat* bitmapMatrix;

	static const byte vblankStartScanline;	//scanline to trigger vblank interrupt
	static const byte vblankRenderScanline;	//scanline to render video buffer to screen

	double lastDrawTimeClocks;				//clock ccycles since the last draw operation

	int colorPalette[4];					//map to colorNumbers used by the game data
	int colorNumbers[4];					//RGB values of the displayable colors
	int overlayTextColor;
	int framecount;							//number of frames that have been drawn
	int skippedFrames;						//consecutively skipped frame
	int framesToSkip;						//the number of frames that should be skipped
	int displayScale;
	int totalFrames;
	int lagFrames;

											//flags to control which mode is set to true
	LCD_Mode mode;
	bool mode0_lcdc;						//screen is in H-blank
	bool mode1_lcdc;						//screen is in V-blank
	bool mode2_lcdc;						//OAM is being accessed
	bool mode3_lcdc;						//OAM andn video ram are being accessed

	bool lcdActive;							//is the screen active?
	bool drawSpritesFlag;					//are the sprites to be drawn?
	bool drawBgWindowFlag;					//are the background and window to be drawn?

	bool recordingAVI;						//are we recording a video now?
	bool recordingMethod;					//true = record real time; speed up/slow downs in final video
											//false = gameboy time. every frame is recorded at 60 fps

	MemoryManager* mem;						//memory component. needed to read video ram
	Z80* z80;								//cpu component. needed for proper timing

	static BITMAP* viewport;				//the portion of the video ram to display

	word windowMapOffset;					//address of the window's tile map
	word bgMapOffset;						//address of the background's tile map
	word tileDataOffset;					//address of the tile display data

	byte spriteHeight;
public:
	static const byte tileDataLength =		16;
	static const byte spriteWidth =			8;
	static const int baseDisplayX;			//x size of the original gameboy screen
	static const int baseDisplayY;			//y size of the original gameboy screen

	LCD();
	~LCD();
	void setComponents(MemoryManager*, Z80*, Gameboy*);
	void skipFrames(int);
	void skipNextFrame();
	int getScale();
	void setScale(int);
	int getSkippedFrames();
	bool toggleActive();
	byte getLCDC();	
	void setColors(int, int, int, int);
	void setOverlayTextColor(int);
	int getColor(int i);
	int getFrame();
	int getLagFrames();

	virtual bool draw();
	virtual void drawBackground(byte);
	virtual void drawWindow(byte);
	virtual void drawSprites();
	void showMessages();
	virtual void renderScanline(byte);
	virtual void vblank(byte ly);
	virtual void fullRender();

	void startAVI(std::string fileName, int codec, bool method);
	void stopAVI();

	void constructState(State*);
	void restoreState(State*);
	void reset();
};

#endif //LCD_H