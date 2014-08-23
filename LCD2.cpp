#include "LCD.h"
#include <ctime>

#define WORKS 1

LCD::LCD()
{
	oam_int = false;
	framecount = 0;
	lcd_active = true;
	background = create_bitmap(256, 256);
	last_draw_time_clocks = 0;
	viewport = create_bitmap(160, 144);
	color_numbers[0] = makecol32(255, 255, 255);
	color_numbers[1] = makecol32(192, 192, 192);
	color_numbers[2] = makecol32(96, 96, 96);
	color_numbers[3] = makecol32(0, 0, 0);
}

void LCD::set_components(MemoryManager* m, Z80* z)
{
	this->z80 = z;
	this->mem = m;
}

bool LCD::toggle_active()
{
	return (lcd_active = !lcd_active);
}

byte LCD::getLCDC()
{
	return mem->read(LCDC);
}

void LCD::proc()
{
}
//70,221 v sync time
//4613 vblank duration
//65608 non-vblank time
//429 h-sync time
void LCD::draw()
{
	int t = z80->cpu_clocks - this->last_draw_time_clocks;
	int ly = mem->read(LY);

#if WORKS == 0
	if(t < 456) {
		return;
	}
#endif

#if WORKS == 1
	//drawing scanlines
	if(ly <= 145)
	{
		if(t < 80)
		{
			if(mem->read(STAT) & 0x40 && !oam_int) {
				//mem->write(0xff0f, 2);
				oam_int = true;
			}
			mem->write(STAT, (mem->read(STAT) & 0xfc) + 2);
			mem->set_oam_access(false);
			mem->set_vram_access(true);
		}
		else if(t < 252)
		{
			oam_int = false;
			mem->write(STAT, (mem->read(STAT) & 0xfc) + 3);
			mem->set_oam_access(false);
			mem->set_vram_access(false);
		}
		else if(t < 456)
		{
			mem->write(STAT, (mem->read(STAT) & 0xfc) + 0);
			mem->set_oam_access(true);
			mem->set_vram_access(true);
		}
		else if(t >= 456)
		{
#endif

			//draw scanline
			last_draw_time_clocks = z80->cpu_clocks;

			//beginning address of the map containing tile map indexes
			word window_map_offset = 0;
			word bg_map_offset = 0;

			//beginning address of the graphical tile data
			word tile_data_offset = 0;

			byte sprite_width = 8;
			byte sprite_height = 0;
			byte tile_data_length = 16;
			bool draw_sprites = true;
			bool draw_bg_window = true;

			if(getLCDC() & 0x40)
			{
				window_map_offset = 0x9c00;
			}
			else
			{
				window_map_offset = 0x9800;
			}

			if(getLCDC() & 0x10)
			{
				tile_data_offset = 0x8000;
			}
			else
			{
				tile_data_offset = 0x9000;
			}

			if(getLCDC() & 0x8)
			{
				bg_map_offset = 0x9c00;
			}
			else
			{
				bg_map_offset = 0x9800;
			}

			if(getLCDC() & 0x4)
			{
				sprite_height = 16;
			}
			else
			{
				sprite_height = 8;
			}

			draw_sprites = getLCDC() & 2;
			draw_bg_window = getLCDC() & 1;

			//mem->write(LCDC, mem->read(LCDC) | 0x80);
			if(getLCDC() & 0x80)
			{

				/*****
				Load Color Palette
				*****/
				int color_map = mem->read(BGP);
				color_palette[0] = color_numbers[color_map & 0x3];
				color_palette[1] = color_numbers[(color_map >> 2) & 0x3];
				color_palette[2] = color_numbers[(color_map >> 4) & 0x3];
				color_palette[3] = color_numbers[(color_map >> 6) & 0x3];

				/*****
				Draw Background
				*****/

				//horizontal line to draw
				int tile_row = ((mem->read(SCY)+mem->read(LY))%256)/8;
				int line_of_tile = ((mem->read(SCY)+mem->read(LY))%256)%8;

				//draw the row specified by LY
				for(int i = 0; i < 32; i++) 
				{
					//get the index of the tile
					int index;
					int tile_index_address; 

					if(getLCDC() & 0x10) 
					{
						tile_index_address = bg_map_offset + (tile_row*32) + i;
						index = (byte)mem->read(tile_index_address); 
					}
					else
					{
						tile_index_address = bg_map_offset + (tile_row*32) + i;
						index = (char)mem->read(tile_index_address);
					}

					//get the pattern of the tile from the index
					byte b1, b2;
					word b1_address = tile_data_offset + (tile_data_length * index) + (line_of_tile * 2);
					b1 = mem->read(b1_address);
					b2 = mem->read(b1_address + 1);

					//draw the row of pixels for this tile
					int ly = mem->read(LY);
					for(int j = 0; j < 8; j++) 
					{
						int pixel_color_index = ((((b2 >> (7-j)) & 1) << 1) + ((b1 >> (7-j)) & 1));
						_putpixel32(background, (i*8)+j, ly, color_palette[pixel_color_index]);
					}


					//////////
					//Draw Window
					//////////
				}

				ly = mem->read(LY);
				int wy = mem->read(WY);
				int wx = mem->read(WX);
				int x = wx-7;
				int map_y = (ly - wy) / 8;
				int line_y = (ly - wy) % 8;

				if(getLCDC() & 0x20 && ly >= wy)
				{
					for(int i = 0; i < (160 - x) / 8; i++)
					{
						//draw line of window
						int index = mem->read(window_map_offset + ((map_y * 32) + i));
						word tile_addr = tile_data_offset + (index * tile_data_length) + (line_y * 2);
							
						if(!getLCDC() & 0x10)
							tile_addr = tile_data_offset + ((char)index * tile_data_length) + (line_y * 2);
							
						byte b1 = mem->read(tile_addr);
						byte b2 = mem->read(tile_addr + 1);

						for(int j = 0; j < 8; j++)
						{
							int px = (wx + (i*8) + j)%160;
							int py = ly%144;
							int pixel_color_index = ((((b2 >> (7-j)) & 1) << 1) + ((b1 >> (7-j)) & 1));
							_putpixel32(viewport, px, py, color_palette[pixel_color_index]);
						}
					}
				}

				if(mem->read(STAT) & 0x40 && mem->read(LY) == mem->read(LYC))
				{
					mem->write(STAT, (mem->read(STAT) & 0xfb) | 4);
				}
				else
				{
					mem->write(STAT, (mem->read(STAT) & 0xfb) | 0);
				}

				if(mem->read(STAT) & 0x10)
				{
					mem->write(0xff0f, 2);
				}
				mem->write(LY, (mem->read(LY)+1) % 154);
			}

			
			//v-bank begins; draw to the screen
			if(mem->read(LY) == 144 && getLCDC() & 0x80)
			{
				/////V-Blank
				mem->write(0xff0f, 1);
			
				mem->write(STAT, (mem->read(STAT) & 0xfc) + 1);

				for(int i = 0; i < 144; i++)
				{
					for(int j = 0; j < 166; j++)
					{
						int source_color = _getpixel32(background, j, i);
						_putpixel32(viewport, j, i, source_color);
					}
				}

				acquire_screen();
#if SHOW_ALL_VID_RAM == 0
				stretch_blit(viewport, screen, 0, 0, 160, 144, 0, 0, 320, 288);
#else
				stretch_blit(background, screen, 0, 0, 256, 256, 0, 0, 512, 512);
#endif
				release_screen();
			}			
		}

#if WORKS == 1
	}
	//wait for the v-blank period to end
	else if(ly >= 145)
	{
		mem->set_oam_access(true);
		mem->set_vram_access(true);
		if(t >= 456)
		{
			this->last_draw_time_clocks = z80->clocks;
			mem->write(LY, (mem->read(LY)+1) % 154);
		}
	}
}
#endif