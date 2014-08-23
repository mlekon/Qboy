#include "State.h"
#include <iostream>
#include <fstream>

State::State()
{
}

State::State(bool auto_write)
{
}


/////////////////////////////////
//write the raw state object data to the disk
bool State::save_to_disk(std::string state_name, int slotNumber)
{
	char ext[512];
	sprintf_s(ext, 512, "%s%d.state", state_name.c_str(), slotNumber);
	//const char* ext = state_name.append(".state").c_str();
	std::fstream fs(ext, std::ios::out | std::ios::binary);

	if(!fs.fail())
		fs.write((char*)this, sizeof(*this));
	
	fs.close();
	return fs.fail();
}


/////////////////////////////////
//loads raw state object data from the disk into this
bool State::load_from_disk(std::string state_name , int slotNumber)
{
	char ext[512];
	sprintf_s(ext, 512, "%s%d.state", state_name.c_str(), slotNumber);
	std::fstream fs(ext, std::ios::in | std::ios::binary);
	if(!fs.fail())
		fs.read((char*)this, sizeof(*this));
	
	fs.close();
	return fs.fail();
}