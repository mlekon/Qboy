#include "GBCLCD.h"
#include "Gameboy.h"
#include <ctime>

GBCLCD::GBCLCD()
{
	framecount =			0;
	lastDrawTimeClocks =	0;
	framesToSkip =			0;
	skippedFrames =			1;
	totalFrames =			0;

	overlayTextColor = makecol(0, 64, 192);

	lcdActive =		true;
	mode =			MODE2;
	mode0_lcdc =	false;
	mode1_lcdc =	false;
	mode2_lcdc =	false;
	mode3_lcdc =	false;

	viewport =		create_bitmap(168, 160);

	//initialize the default color values 
	colorNumbers[COLOR_WHITE] =			makecol32(255, 255, 255);
	colorNumbers[COLOR_LIGHT_GRAY] =	makecol32(162, 162, 162);
	colorNumbers[COLOR_DARK_GRAY] =		makecol32(96, 96, 96);
	colorNumbers[COLOR_BLACK] =			makecol32(0, 0, 0);
}


GBCLCD::~GBCLCD()
{
}


/////////////////////////////////
//set the pointers to the memory and cpu components
void GBCLCD::setComponents(GBCMemoryManager* m, Z80* z, Gameboy* gb)
{
	this->gb = gb;
	z80 = z;
	mem = m;
	gbcmem = m;
}



/////////////////////////////////
//renders the video ram to the viewport bitmap
bool GBCLCD::draw()
{
	//the number of cycles since the last scan line was drawn
	double t = z80->cpuClocks - this->lastDrawTimeClocks;

	//the current scan line to draw
	int ly = mem->readRam(LY);

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
			gbcmem->setPaletteAccess(true);
			mode2_lcdc = true;
			mode0_lcdc = false;		
		}
		if(t > 80)
			mode = MODE3;
		break;
	case MODE3:
		//mode 3: screen is accessing OAM and video ram
		if(!mode3_lcdc) {
			mem->writeRam(STAT, (mem->readRam(STAT) & 0xfc) + 3);
			mem->setOamAccess(true);
			mem->setVramAccess(true);
			gbcmem->setPaletteAccess(false);
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
			gbcmem->setPaletteAccess(true);
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
void GBCLCD::reset()
{
	lastDrawTimeClocks =	0;
	totalFrames =			0;
	mode =					MODE2;
}


/////////////////////////////////
//stores the time since the last drawing operation
//in the given state object
void GBCLCD::constructState(State* s)
{
	s->lcdHorizontalPhase = this->lastDrawTimeClocks;
	s->lcdMode = mode;
	s->frame = totalFrames;
}


/////////////////////////////////
//restores the time since the last drawing operation
//from the given state object
void GBCLCD::restoreState(State* s)
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
void GBCLCD::drawSprites()
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
		byte palette = spriteFlags & 3;

		int colorPalette[4];
		for(int p = 1; p < 4; p++)
		{
			word colors = (gbcmem->readPaletteRam(palette * 8 + (p * 2) + 1) << 8) | gbcmem->readPaletteRam(palette * 8 + (p * 2));
			byte r = colors & 0x1f;
			byte g = (colors >> 5) & 0x1f;
			byte b = (colors >> 10) & 0x1f;

			colorPalette[p] = makecol32(r * 8, g * 8, b * 8);
		}

		//horizontal and vertical flip loop counters
		//allows pixels to be renders in reverse order if flipped
		int hf = (spriteFlags & 0x20) ? 7 : 0;
		int vf = (spriteFlags & 0x40) ? spriteHeight-1 : 0;
		
		//address of the tile display data
		int tileAddr = 0x8000 + (tileDataLength * patternNumber);

		//translate sprite data coordinates to screen coordinates
		int px = sx-8;
		int py = sy-16;

		//loop through each vertical row in the sprite (8 or 16)
		for(int j = 0; j < spriteHeight; j++)
		{
			//one row of sprite pixel is determined by 2 bytes
			byte b1 = gbcmem->readVRam(tileAddr+(2*j), (spriteFlags & 0x4) ? 1 : 0);
			byte b2 = gbcmem->readVRam(tileAddr+(2*j)+1, (spriteFlags & 0x4) ? 1 : 0);

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
				if(pixelColorIndex != 0) {
					//viewport coordinates of the pixel
					int xx = px + abs(k - hf);
					int yy = py + abs(j - vf);

					//only draw pixels on the screen
					if(xx >= 0 && xx < 160 && yy >= 0 && yy < 144) {
						//if the priority bit is set the sprite is only drawn
						//over background of color at index of 0
						if((spriteFlags & 0x80) && _getpixel32(viewport, xx, yy) == colorPalette[0])
						{
							_putpixel32(viewport, xx, yy, colorPalette[pixelColorIndex]);
						}
						//if the priority bit is not set, draw the sprite
						//over the background and window
						else if(!(spriteFlags & 0x80))
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
void GBCLCD::drawWindow(byte ly)
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
		for(int i = 0; i <= (baseDisplayX - wx) / 8; i++)
		{
			//calculate the tile index of the pixel and the address
			//of the tile's data
			int index = mem->read(windowMapOffset + ((mapY * 32) + i));
			word tileAddr = tileDataOffset + (index * tileDataLength) + (lineY * 2);
				
			byte tileFlags = gbcmem->readVRam(windowMapOffset + ((mapY * 32) + i), 1);
			byte palette = tileFlags & 3;

			int colorPalette[4];
			for(int p = 1; p < 4; p++)
			{
				word colors = (gbcmem->readPaletteRam(palette * 8 + (p * 2) + 1) << 8) | gbcmem->readPaletteRam(palette * 8 + (p * 2));
				byte r = colors & 0x1f;
				byte g = (colors >> 5) & 0x1f;
				byte b = (colors >> 10) & 0x1f;

				colorPalette[p] = makecol32(r * 8, g * 8, b * 8);
			}

			//adjust tile data addresses if necessary
			if(!(mem->readRam(LCDC) & 0x10))
			{
				tileDataOffset = 0x9000;
				tileAddr = tileDataOffset + ((char)index * tileDataLength) + (lineY * 2);
			}

			//bytes representing a row of 8 pixels' colors
			byte b1 = gbcmem->readVRam(tileAddr, (tileFlags & 4) ? 1 : 0);
			byte b2 = gbcmem->readVRam(tileAddr + 1, (tileFlags & 4) ? 1 : 0);

			//draw a set of pixels
			for(int j = 0; j < 8; j++)
			{
				//calculate the viewport coordinates and color index
				int px = (wx + (i*8) + j);
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
void GBCLCD::drawBackground(byte ly)
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
		byte tileFlags = gbcmem->readVRam(tile_index_address, 1);
		byte palette = tileFlags & 3;

		if(mem->readRam(LCDC) & 0x10) 
			//unsigned index
			index = (byte)mem->readRam(tile_index_address); 
		else
			//signed index
			index = (char)mem->readRam(tile_index_address);
		


		int colorPalette[4];
		for(int p = 1; p < 4; p++)
		{
			word colors = (gbcmem->readPaletteRam(palette * 8 + (p * 2) + 1) << 8) | gbcmem->readPaletteRam(palette * 8 + (p * 2));
			byte r = colors & 0x1f;
			byte g = (colors >> 5) & 0x1f;
			byte b = (colors >> 10) & 0x1f;

			colorPalette[p] = makecol32(r * 8, g * 8, b * 8);
		}

		//get the pattern of the tile from the index
		word b1_address = tileDataOffset + (tileDataLength * index) + (line_of_tile * 2);
		
		//pixel color data
		byte b1 = gbcmem->readVRam(b1_address, (tileFlags & 4) ? 1 : 0);
		byte b2 = gbcmem->readVRam(b1_address + 1, (tileFlags & 4) ? 1 : 0);

		//draw the row of pixels for this tile
		for(byte j = 0; j < 8; j++) 
		{ 
			byte pixelColorIndex = ((((b2 >> (7-j)) & 1) << 1) + ((b1 >> (7-j)) & 1));
			_putpixel32(viewport, (((i*8)+j)-scx)%168, ly%160, colorPalette[pixelColorIndex]);
		}
	}
	release_bitmap(viewport);
}


/////////////////////////////////
//draws the window and background of the given scanline
void GBCLCD::renderScanline(byte ly)
{
	//skip the rendering phase if the frame is to be skipped
	if(skippedFrames <= framesToSkip) 
	{
		//increment the current scan line for the next H-blank
		if(ly == mem->readRam(LYC) && mem->readRam(STAT) & 40)
		{
			mem->requestInterrupt(LCDC_STATUS);
			mem->write(STAT, (mem->read(STAT) & 0xfb) | 4);
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
		//int color_map = mem->read(BGP);
		//colorPalette[0] = colorNumbers[color_map & 0x3];
		//colorPalette[1] = colorNumbers[(color_map >> 2) & 0x3];
		//colorPalette[2] = colorNumbers[(color_map >> 4) & 0x3];
		//colorPalette[3] = colorNumbers[(color_map >> 6) & 0x3];

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
			mem->write(STAT, (mem->read(STAT) & 0xfb) | 4);
		}
	}
}


/////////////////////////////////
//triggers the v-blank interrupt, adjust necessary registers,
//and renders the buffer to the screen
void GBCLCD::vblank(byte ly)
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
		showMessages();

		//draw the viewport to the screen
		acquire_screen();
		stretch_blit(
			viewport,
			screen,
			0,
			0, 
			baseDisplayX, 
			baseDisplayY,
			0, 
			0, 
			baseDisplayX * displayScale,
			baseDisplayY * displayScale);
		release_screen();

		skippedFrames = 1;
	}	
}



//void GBCLCD::setColors(int w, int lg, int dg, int b)
//{
//	colorNumbers[COLOR_WHITE] = makecol(w & 0xFF, (w & 0xFF00) >> 8, (w & 0xFF0000) >> 16);
//	colorNumbers[COLOR_LIGHT_GRAY] = makecol(lg & 0xFF, (lg & 0xFF00) >> 8, (lg & 0xFF0000) >> 16);
//	colorNumbers[COLOR_DARK_GRAY] = makecol(dg & 0xFF, (dg & 0xFF00) >> 8, (dg & 0xFF0000) >> 16);
//	colorNumbers[COLOR_BLACK] = makecol(b & 0xFF, (b & 0xFF00) >> 8, (b & 0xFF0000) >> 16);
//}
//
//
//void GBCLCD::setOverlayTextColor(int c)
//{
//	overlayTextColor = makecol(c & 0xFF, (c & 0xFF00) >> 8, (c & 0xFF0000) >> 16);
//}
//
//
//int GBCLCD::getColor(int i)
//{
//	if(i == 4)
//		return ((overlayTextColor & 0xFF) << 16) | ((overlayTextColor & 0xFF00) | ((overlayTextColor & 0xFF0000) >> 16));
//	else if(i >= 0 && i < 4)
//		return ((colorNumbers[i] & 0xFF) << 16) | ((colorNumbers[i] & 0xFF00) | ((colorNumbers[i] & 0xFF0000) >> 16));
//	else
//		return 0;
//}

