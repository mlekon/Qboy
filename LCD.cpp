#include "LCD.h"
#include "Gameboy.h"
#include <ctime>

BITMAP* LCD::viewport = NULL;

const int LCD::baseDisplayX = 160;
const int LCD::baseDisplayY = 144;
const byte LCD::vblankRenderScanline = 144;
const byte LCD::vblankStartScanline = 144;

LCD::LCD()
{
	framecount =			0;
	lastDrawTimeClocks =	0;
	framesToSkip =			0;
	skippedFrames =			1;
	totalFrames =			0;
	displayScale =			3;

	overlayTextColor = makecol(0, 64, 192);

	lcdActive =		true;
	mode =			MODE2;
	mode0_lcdc =	false;
	mode1_lcdc =	false;
	mode2_lcdc =	false;
	mode3_lcdc =	false;

	videoWriter = NULL;
	recordingAVI = false;

	if(!viewport) viewport =		create_bitmap_ex(32, 160, 144);

	//initialize the default color values 
	colorNumbers[COLOR_WHITE] =			makecol32(255, 255, 255);
	colorNumbers[COLOR_LIGHT_GRAY] =	makecol32(162, 162, 162);
	colorNumbers[COLOR_DARK_GRAY] =		makecol32(96, 96, 96);
	colorNumbers[COLOR_BLACK] =			makecol32(0, 0, 0);
}


LCD::~LCD()
{
	delete videoWriter;
}


/////////////////////////////////
//skips the given number of frames
void LCD::skipFrames(int i)
{
	framesToSkip = i;
}


/////////////////////////////////
//skips a single frame
void LCD::skipNextFrame()
{
	skippedFrames--;
}


/////////////////////////////////
//get the number of consecutively skipped frames
int LCD::getSkippedFrames()
{
	return skippedFrames;
}


/////////////////////////////////
//set the pointers to the memory and cpu components
void LCD::setComponents(MemoryManager* m, Z80* z, Gameboy* gb)
{
	this->gb = gb;
	z80 = z;
	mem = m;
}


/////////////////////////////////
//disables drawing to the screen
bool LCD::toggleActive()
{
	return (lcdActive = !lcdActive);
}


/////////////////////////////////
//gets the flags describing the screen's state
byte LCD::getLCDC()
{
	return mem->readRam(LCDC);
}


/////////////////////////////////
//renders the video ram to the viewport bitmap
bool LCD::draw()
{
	//the number of cycles since the last scan line was drawn
	double t = z80->cpuClocks - this->lastDrawTimeClocks;

	//the current scan line to draw
	int ly;

	switch(mode)
	{
	case MODE2:
		//mode 2: screen is accessing OAM
		if(!mode2_lcdc) {
			if(mem->readRam(STAT) & 0x20)
				mem->requestInterrupt(LCDC_STATUS);
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfc) + 2);
			mem->setOamAccess(false);
			mem->setVramAccess(true);
			mode2_lcdc = true;
			mode0_lcdc = false;		
		}
		if(t > 80)
			mode = MODE3;
		break;
	case MODE3:
		//mode 3: screen is accessing OAM and video ram
		if(!mode3_lcdc) {
			ly = mem->readRam(LY);
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfc) + 3);
			mem->setOamAccess(true);
			mem->setVramAccess(true);
			mode3_lcdc = true;
			mode2_lcdc = false;
			renderScanline(ly);
		}
		if(t > 252)
			mode = MODE0;
		break;
	case MODE0:
		//mode 0: screen is in horizontal blank
		if(!mode0_lcdc) {
			if(mem->readRam(STAT) & 0x8)
				mem->requestInterrupt(LCDC_STATUS);
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfc) + 0);
			mem->setOamAccess(false);
			mem->setVramAccess(true);
			mode0_lcdc = true;
			mode3_lcdc = false;
		}
		if(t > 456)
			mode = MODE_HL_DONE;
		break;
	case MODE_HL_DONE:
		//H-blank has finished; draw the next scan line
		lastDrawTimeClocks = z80->cpuClocks;
		mode0_lcdc = false;
		ly = mem->readRam(LY);

		//increment the scan line to draw after the next H-blank
		mem->writeRam(LY, (ly+1));
		if(ly + 1 == mem->readRam(LYC) && mem->readRam(STAT) & 0x40)
		{
			mem->requestInterrupt(LCDC_STATUS);
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfb) | 4);
		}

		if(ly + 1 == vblankStartScanline)
			mode = MODE_VB_RENDER;
		else
			mode = MODE2;
		break;
	case MODE_VB_RENDER:
		ly = mem->readRam(LY);
		gb->input();

		//have the input registers been read this frame?
		if(mem->lagFrame()) lagFrames++;

		totalFrames++;
		vblank(ly);
		mode = MODE1;
		mem->setOamAccess(true);
		mem->setVramAccess(true);
		return true;
	case MODE1:
		//increment the scan line
		if(t >= 456)
		{
			ly = mem->readRam(LY);
			this->lastDrawTimeClocks = z80->cpuClocks;
			mem->writeRam(LY, (ly + 1) % 155);
			if((ly + 1) % 155 == mem->readRam(LYC) && mem->readRam(STAT) & 0x40)
			{
				mem->requestInterrupt(LCDC_STATUS);
				mem->writeRam(STAT, (mem->readRam(STAT) & 0xfb) | 4);
			}

			if(ly+1 == 155)
				mode = MODE2;
		}
		break;
	}

	return false;
}


/////////////////////////////////
//resets the current scan line
void LCD::reset()
{
	lastDrawTimeClocks =	0;
	totalFrames =			0;
	mode =					MODE2;
	lcdActive =				true;
	mode0_lcdc =			false;
	mode1_lcdc =			false;
	mode2_lcdc =			false;
	mode3_lcdc =			false;
	framecount =			0;
	lagFrames =				0;
}


/////////////////////////////////
//stores the time since the last drawing operation
//in the given state object
void LCD::constructState(State* s)
{
	s->lcdHorizontalPhase = this->lastDrawTimeClocks;
	s->lcdMode = mode;
	s->frame = totalFrames;
}


/////////////////////////////////
//restores the time since the last drawing operation
//from the given state object
void LCD::restoreState(State* s)
{
	this->lastDrawTimeClocks = s->lcdHorizontalPhase;
	mode = (LCD_Mode)s->lcdMode;
	skippedFrames = 1;
	framesToSkip = 0;
	totalFrames = s->frame;
}


/////////////////////////////////
//renders the sprites in numeric order on top of the
//background and window
void LCD::drawSprites()
{
	//are sprites turned on?
	if(!(mem->readRam(LCDC) & 0x2))
		return;

	//loop through each of the 40 sprites
	for(int i = 39; i >= 0; i--)
	{
		//get the sprite's tile index and screen location
		int patternNumber = mem->readRam(0xfe00 + (i * 4) + 2);
		int sx = mem->readRam(0xfe00 + (i * 4) + 1);
		int sy = mem->readRam(0xfe00 + (i * 4));
		
		//adjust the index if the height is 16 pixels
		//so that two tiles will be displayed
		if(spriteHeight == 16)
			patternNumber &= 0xfe;

		//don't bother drawing sprites that wouldn't appear on the screen
		if(sx == 0 || sx >= baseDisplayX+8 || sy == 0 || sy >= baseDisplayY+16)
			continue;

		byte spriteFlags = mem->readRam(0xfe00 + (i*4) + 3);
						
		//horizontal and vertical flip loop counters
		//allows pixels to be renders in reverse order if flipped
		int hf = (spriteFlags & 0x20) ? 7 : 0;
		int vf = (spriteFlags & 0x40) ? spriteHeight-1 : 0;

		//get the sprite's palette and load the colors
		int color_map = mem->readRam((spriteFlags & 0x10) ? OBP1 : OBP0);
		colorPalette[COLOR_WHITE] =			colorNumbers[color_map & 0x3];
		colorPalette[COLOR_LIGHT_GRAY] =	colorNumbers[(color_map >> 2) & 0x3];
		colorPalette[COLOR_DARK_GRAY] =		colorNumbers[(color_map >> 4) & 0x3];
		colorPalette[COLOR_BLACK] =			colorNumbers[(color_map >> 6) & 0x3];

		//address of the tile display data
		int tileAddr = 0x8000 + (tileDataLength * patternNumber);

		//translate sprite data coordinates to screen coordinates
		int px = sx-8;
		int py = sy-16;

		//loop through each vertical row in the sprite (8 or 16)
		for(int j = 0; j < spriteHeight; j++)
		{
			//one row of sprite pixel is determined by 2 bytes
			byte b1 = mem->readRam(tileAddr+(2*j));
			byte b2 = mem->readRam(tileAddr+(2*j)+1);

			//loop through each column of the sprite (always 8)
			for(int k = 0; k < 8; k++)
			{
				//each of the 8 pixels' colors are determined by the bits 
				//in the two bytes in the same position. (byte1 & 0x1) is the
				//first bit, (byte2 & 0x1) is the second bit in the color index
				//of the pixel in column 1
				int pixelColorIndex = ((((b2 >> (7-k)) & 1) << 1) + ((b1 >> (7-k)) & 1));

				//a color index of 0 for a sprite is transparent
				//don't draw
				if(pixelColorIndex != 0) 
				{
					//viewport coordinates of the pixel
					int xx = px + abs(k - hf);
					int yy = py + abs(j - vf);

					//only draw pixels on the screen
					if(xx >= 0 && xx < 160 && yy >= 0 && yy < 144)
					{
						//if the priority bit is set the sprite is only drawn
						//over background of color at index of 0
						//if the priority bit is not set, draw the sprite
						//over the background and window
						if(!(spriteFlags & 0x80) || ((spriteFlags & 0x80) && _getpixel32(viewport, xx, yy) == colorPalette[0]))
						{
							_putpixel32(viewport, xx, yy, colorPalette[pixelColorIndex]);
						}
					}
				}
			}
		}
	}
}


/////////////////////////////////
//draw the window at the given scan line
//if it covers that area
void LCD::drawWindow(byte ly)
{
	int wy = mem->readRam(WY);
	int wx = mem->readRam(WX)-7;
	int mapY = (ly - wy) / 8;
	int lineY = (ly - wy) % 8;
				
	//is the window enabled, and does the window
	//extend far enough to cover the current scan line?
	if((mem->readRam(LCDC) & 1) && (mem->readRam(LCDC) & 0x20) && ly >= wy)
	{
		//loop through each pixel on the scan line that
		//the window covers
		acquire_bitmap(viewport);
		for(int i = 0; i < (baseDisplayX - wx) / 8; i++)
		{
			//calculate the tile index of the pixel and the address
			//of the tile's data
			int index = mem->read(windowMapOffset + ((mapY * 32) + i));
			word tileAddr = tileDataOffset + (index * tileDataLength) + (lineY * 2);
						
			//adjust tile data addresses if necessary
			if(!(mem->readRam(LCDC) & 0x10))
			{
				tileDataOffset = 0x9000;
				tileAddr = tileDataOffset + ((char)index * tileDataLength) + (lineY * 2);
			}

			//bytes representing a row of 8 pixels' colors
			byte b1 = mem->read(tileAddr);
			byte b2 = mem->read(tileAddr + 1);

			//draw a set of pixels
			for(int j = 0; j < 8; j++)
			{
				//calculate the viewport coordinates and color index
				int px = (wx + (i*8) + j)%168;
				int py = ly%144;
				int pixelColorIndex = ((((b2 >> (7-j)) & 1) << 1) + ((b1 >> (7-j)) & 1));

				_putpixel32(viewport, px, py, colorPalette[pixelColorIndex]);
			}
		}
		release_bitmap(viewport);
	}
}


/////////////////////////////////
//renders given scan line of the background tiles
void LCD::drawBackground(byte ly)
{
	byte scx = mem->readRam(SCX);
	byte scy = mem->readRam(SCY);

	int tile_row = ((scy+ly)%256)/8;
	int line_of_tile = ((scy+ly)%256)%8;

	//range of pixels to draw
	byte first_x = scx/8;
	byte last_x = first_x+21;

	acquire_bitmap(viewport);
	for(byte i = first_x; i < last_x; i++) 
	{
		//get the index of the tile
		int index;
		word tile_index_address; 

		tile_index_address = bgMapOffset + (tile_row*32) + (i%32);

		if(mem->readRam(LCDC) & 0x10) 
			//unsigned index
			index = (byte)mem->readRam(tile_index_address); 
		else
			//signed index
			index = (char)mem->readRam(tile_index_address);
		
		//get the pattern of the tile from the index
		word b1_address = tileDataOffset + (tileDataLength * index) + (line_of_tile * 2);
		
		//pixel color data
		byte b1 = mem->readRam(b1_address);
		byte b2 = mem->readRam(b1_address + 1);

		//draw the row of pixels for this tile
		for(byte j = 0; j < 8; j++) 
		{ 
			byte x = (((i*8)+j)-scx);
			if(x < 160 && ly < 144)
			{
				byte pixelColorIndex = ((((b2 >> (7-j)) & 1) << 1) | ((b1 >> (7-j)) & 1));
				_putpixel32(viewport, x, ly, colorPalette[pixelColorIndex]);
			}
		}
	}
	release_bitmap(viewport);
}


/////////////////////////////////
//sets how much to stretch the viewport when drawing to the screen
void LCD::setScale(int displayScale)
{
	this->displayScale = (displayScale > 0 && displayScale < 10) ? displayScale : DEFAULT_SCALE;
}


/////////////////////////////////
//get the factor the screen is stretched to
int LCD::getScale()
{
	return displayScale;
}


/////////////////////////////////
//draws the window and background of the given scanline
void LCD::renderScanline(byte ly)
{
	//skip the rendering phase if the frame is to be skipped
	if(skippedFrames <= framesToSkip) 
	{
		//increment the current scan line for the next H-blank
		if(ly == mem->readRam(LYC) && mem->readRam(STAT) & 40)
		{
			mem->requestInterrupt(LCDC_STATUS);
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfb) | 4);
		}

		return;
	}

	//display data locations addresses
	windowMapOffset =		(mem->readRam(LCDC) & 0x40)	?	0x9c00 : 0x9800;
	tileDataOffset =		(mem->readRam(LCDC) & 0x10)	?	0x8000 : 0x9000;
	bgMapOffset =			(mem->readRam(LCDC) & 0x8)	?	0x9c00 : 0x9800;

	spriteHeight =			(mem->readRam(LCDC) & 0x4)	?	16 : 8;
	drawSpritesFlag =		(mem->readRam(LCDC) & 2) != 0;
	drawBgWindowFlag =		mem->readRam(LCDC) & 1;

	//is the screen active?
	//begin rendering the next scan line
	if(mem->readRam(LCDC) & 0x80)
	{
		//load color palette
		int color_map = mem->readRam(BGP);
		colorPalette[0] = colorNumbers[color_map & 0x3];
		colorPalette[1] = colorNumbers[(color_map >> 2) & 0x3];
		colorPalette[2] = colorNumbers[(color_map >> 4) & 0x3];
		colorPalette[3] = colorNumbers[(color_map >> 6) & 0x3];

		drawBackground(ly);
		drawWindow(ly);

		//update screen status
		if(mem->readRam(STAT) & 0x40 && ly == mem->readRam(LYC))
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfb) | 4);
		else
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfb) | 0);
				
		//request the interrupt as specified by LCDC
		if(mem->readRam(STAT) & 0x10)
			mem->requestInterrupt(LCDC_STATUS);
				
		if(ly+1 == mem->readRam(LYC) && mem->readRam(STAT) & 40)
		{
			mem->requestInterrupt(LCDC_STATUS);
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfb) | 4);
		}
	}
}


/////////////////////////////////
//triggers the v-blank interrupt, adjust necessary registers,
//and renders the buffer to the screen
void LCD::vblank(byte ly)
{
	if(ly == vblankStartScanline) 
	{
		//request v-blank interrupt
		mem->requestInterrupt(VBLANK);
		if(mem->readRam(STAT) & 0x10)
			mem->requestInterrupt(LCDC_STATUS);

		//update screen status
		mem->writeRam(STAT, (mem->readRam(STAT) & 0xfc) + 1);
	}

	if(ly == vblankRenderScanline && mem->readRam(LCDC) & 0x80)
	{			
		//skip the rendering phase if the frame is to be skipped
		if(skippedFrames <= framesToSkip)
		{
			skippedFrames++;
			return;
		}

		drawSprites();

/*		if(recordingAVI && videoWriter && videoWriter->isOpened())
		{
			static Mat bitmapMatrix(Size(160, 144), CV_32S);

			for(int i = 0; i < 144; i++)
			{
				for(int j = 0; j < 160; j++)
				{
					((int*)(bitmapMatrix.ptr(i)))[j] = ((int*)viewport->line[i])[j];
				}
			}
			(*videoWriter) << bitmapMatrix;
		}
*/
		showMessages();

		//draw the viewport to the screen
		acquire_screen();
		stretch_blit(
			viewport,
			screen,
			0,
			0, 
			viewport->w, 
			viewport->h,
			0, 
			0, 
			screen->w,
			screen->h);
		release_screen();
		skippedFrames = 1;
	}	
}


/////////////////////////////////
//redraws the complete frame immediately
//using whatever is in the video memory
void LCD::fullRender()
{
	LCD_Mode backup = mode;
	double cpuClocksBackup = z80->cpuClocks;
	double lastDrawTimeBackup = lastDrawTimeClocks;
	word lyBackup = mem->readRam(LY);
	word LCDCbackup = mem->readRam(LCDC);
	word STATbackup = mem->readRam(STAT);
	byte interruptRequestBackup = mem->readRam(0xff0f);
	byte interruptEnableBackup = mem->readRam(0xffff);

	z80->cpuClocks = 4;
	lastDrawTimeClocks = 0;
	mem->writeRam(LY, 0);
	mem->writeRam(0xffff, 0);
	mode = MODE2;

	while(mode != MODE1)
	{
		draw();
		z80->cpuClocks += 4;
	}

	mode = backup;
	z80->cpuClocks = cpuClocksBackup;
	lastDrawTimeClocks = lastDrawTimeBackup;
	mem->writeRam(LY, lyBackup);
	mem->writeRam(LCDC, LCDCbackup);
	mem->writeRam(STAT, STATbackup);
	mem->writeRam(0xff0f, interruptRequestBackup);
	mem->writeRam(0xffff, interruptEnableBackup);
}


/////////////////////////////////
//display emulator messages on screen, 
//such as when a state is loaded or saved
void LCD::showMessages()
{
	std::list<Message>* messages = gb->getMessages(); 
	if(messages->size() > 0)
	{
		std::list<Message>::iterator i = messages->begin();
		int j = baseDisplayY - font->height;

		//print all messages on screen
		for(; i != messages->end(); i++)
		{
			if(strcmp(messageText[(*i).type].c_str(), "") == 0)
				continue;

			textprintf_ex(viewport,
				font, 
				5, 
				j,
				overlayTextColor, 
				-1,
				messageText[(*i).type].c_str(),
				(*i).value);

			j -= (font->height + 2);
		}

		//remove old messages
		if(clock() - gb->getMessageTimer() >= gb->getMessageDelay())
		{
			messages->pop_front();
			gb->resetMessageTimer();
		}
	}
}


/////////////////////////////////
//sets the gameboy colors, swapping the endianess
void LCD::setColors(int w, int lg, int dg, int b)
{
	colorNumbers[COLOR_WHITE] =			makecol(w & 0xFF, (w & 0xFF00) >> 8, (w & 0xFF0000) >> 16);
	colorNumbers[COLOR_LIGHT_GRAY] =	makecol(lg & 0xFF, (lg & 0xFF00) >> 8, (lg & 0xFF0000) >> 16);
	colorNumbers[COLOR_DARK_GRAY] =		makecol(dg & 0xFF, (dg & 0xFF00) >> 8, (dg & 0xFF0000) >> 16);
	colorNumbers[COLOR_BLACK] =			makecol(b & 0xFF, (b & 0xFF00) >> 8, (b & 0xFF0000) >> 16);
}


/////////////////////////////////
//sets the color of the emulator text overlay
void LCD::setOverlayTextColor(int c)
{
	overlayTextColor = makecol(c & 0xFF, (c & 0xFF00) >> 8, (c & 0xFF0000) >> 16);
}


/////////////////////////////////
//returns the specified gameboy or emulator color
//colors 1-4 are gameboy colors, 0 is the emulator text overlay
int LCD::getColor(int i)
{
	if(i == 4)
		return ((overlayTextColor & 0xFF) << 16) | ((overlayTextColor & 0xFF00) | ((overlayTextColor & 0xFF0000) >> 16));
	else if(i >= 0 && i < 4)
		return ((colorNumbers[i] & 0xFF) << 16) | ((colorNumbers[i] & 0xFF00) | ((colorNumbers[i] & 0xFF0000) >> 16));
	else
		return 0;
}



/////////////////////////////////
//returns the number of elapsed frames
int LCD::getFrame()
{
	return totalFrames;
}



/////////////////////////////////
//returns the number of elapsed frames in which the
//loaded program did not check the input registers
int LCD::getLagFrames()
{
	return lagFrames;
}


void LCD::startAVI(std::string fileName, int codec, bool method)
{
	videoWriter = new VideoWriter(fileName, codec, 60, Size(160, 144));
	recordingAVI = true;
	recordingMethod = method;
	
	if(!videoWriter->isOpened())
	{
		gb->addMessage(CANT_OPEN_VIDEO_RECORD, 0);
		delete videoWriter;
	}
}


void LCD::stopAVI()
{
	recordingAVI = false;

	if(videoWriter)
	{
		delete videoWriter;
		videoWriter = NULL;		
	}
}