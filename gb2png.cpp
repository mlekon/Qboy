	/* 
	///////
	PRINT GB ROM AS PNG
	///////

	std::ifstream* ifs = new std::ifstream();
	ifs->open("Pokemon - Blue Version (UE) [S][!].gb", std::ios::binary);
	BITMAP* out = create_bitmap_ex(32, 512, 512);
	__int32 buffer;
	int count = 0;
	for(int i = 0; i < 512; i++) {
		for(int j = 0; j < 512; j++) {
			ifs->read((char*)&buffer, 4);
			_putpixel32(out, i, j, buffer);
			count++;
		}
	}
	
	acquire_screen();
	draw_sprite(screen, out, 0, 0);
	release_screen();
	std::cout << count;
	save_bitmap("pkmn.bmp", out, NULL);
	save_tga("pkmn.tga", out, NULL);

	////////
	//////// */