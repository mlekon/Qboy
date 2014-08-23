#include "Movie.h"
#include <iostream>
#include <fstream>


Movie::Movie(std::string movieName, std::string authorName, std::string fileName, std::string cartName)
{
	inputFrames.clear();
	keyFrames.clear();
	playbackPosition = 0;
	previousInput = 0;
	recordPosition = 0;
	previousFrame = 0;
	this->authorName = authorName;
	this->fileName = fileName;
	this->movieName = movieName;
	this->cartName = cartName;
}

void Movie::record(byte p20, byte p10, byte inputEmu, int currentFrame)
{
	byte input = (p20 << 4) | p10;

	//overwrite if given a frame that had been recorded already
	if(currentFrame < (int)inputFrames.size())
	{
		inputFrames[currentFrame] = input;
		recordPosition = currentFrame;
	}

	previousInput = input;
	inputFrames.push_back(input);
	recordPosition++;
	previousFrame = currentFrame;
}

void Movie::record(byte input, byte inputEmu, int currentFrame)
{
	while(recordPosition < currentFrame - 1)
	{
		inputFrames.push_back(0);
		recordPosition++;
	}

	previousInput = input;
	inputFrames.push_back(input);
	recordPosition++;
}


void Movie::unrecordToFrame(int frame)
{
	if(frame >= 0 && frame < (int)inputFrames.size())
	{
		inputFrames.resize(frame, 0);
		previousInput = (byte)inputFrames.back();
	}
}

void Movie::unrecordLast()
{
	inputFrames.pop_back();
	previousInput = (byte)inputFrames.back();
}

byte Movie::playback()
{
	if(playbackPosition < (int)inputFrames.size())
		return (byte)inputFrames[playbackPosition++];
	else
		return 0;
}


byte Movie::playback(int frame)
{
	if(frame >= 0 && frame < (int)inputFrames.size())
		return inputFrames[frame] & 0xff;
	else
		return 0xff;
}


byte Movie::playbackEmu(int frame)
{
	if(frame >= 0 && frame < (int)inputFrames.size())
		return inputFrames[frame] >> 8;
	else
		return 0xff;
}


void Movie::setFormat(int format)
{
	this->format = format;
}

void Movie::setPlaybackPosition(int frame)
{
	if(frame >= 0 && frame < (int)inputFrames.size())
		playbackPosition = frame;
}

void Movie::setRelativePlaybackPosition(int offset)
{
	int resultFrame = playbackPosition + offset;
	if(resultFrame >= 0 && resultFrame < (int)inputFrames.size())
		playbackPosition = resultFrame;
}


void Movie::writeToDisk(int format)
{
	if(format & 1)
		qboyWriteToDisk();
	if(format & 2)
		vbaWriteToDisk();
}


void Movie::vbaWriteToDisk()
{
	char fileName[512];
	sprintf_s(fileName, 512, "%s.vbm", movieName);

	std::fstream out;
	out.open(fileName, std::ios::binary | std::ios::out);

	char signature[4] = {0x56, 0x42, 0x4D, 0x1A};
	char majorVersion[4] = {1, 0, 0, 0};
	char movieUID[4] = {0, 0, 0, 0};
	int frameCount = inputFrames.size();
	char startFlags = 0;
	char controllerFlags = 1;
	char system = 0;
	char emulatorOptions = 0;
	int winSaveType = 0;
	int winFlashSize = 0;
	int emulatorType = 0;
	char romName[12];
	char minorVersion = 1;
	char romCRC = 0;

	if(!out.fail())
	{
		out.write(signature, 4);
		out.write(majorVersion, 4);
		out.write(movieUID, 4);
		out.write((char*)&frameCount, 4);
		out.write((char*)&rerecordCount, 4);
		out.write(&startFlags, 1);
		out.write(&controllerFlags, 1);
		out.write(&system, 1);
		out.write(&emulatorOptions, 1);
		out.write((char*)&winSaveType, 4);
		out.write((char*)&winFlashSize, 4);
		out.write((char*)&emulatorType, 4);
	}
}


void Movie::qboyWriteToDisk()
{
	char fileName[512];

	if(this->fileName.size() > 0)
		sprintf_s(fileName,  512, "%s.qbmv", this->fileName.c_str());
	else
		sprintf_s(fileName, 512, "%s-F%d.qbmv", movieName.c_str(), inputFrames.size());

	std::fstream out;
	out.open(fileName, std::ios::binary | std::ios::out);
	int vecSize = inputFrames.size();
	if(!out.fail())
	{
		char nullTerminator = '\0';
		out.write((char*)(&vecSize), sizeof(int));
		out.write((char*)(&recordPosition), sizeof(int));
		out.write(movieName.c_str(), movieName.length());
		out.write(&nullTerminator, sizeof(char));
		out.write(cartName.c_str(), cartName.length());
		out.write(&nullTerminator, sizeof(char));
		char* outputBuffer = new char[inputFrames.size()];

		for(int i = 0; i < (int)inputFrames.size(); i++)
			outputBuffer[i] = (byte)inputFrames[i];

		out.write(outputBuffer, inputFrames.size() * sizeof(byte));
		delete[] outputBuffer;
	}
	out.close();
}


void Movie::writeToDisk(std::fstream& out)
{
	int vecSize = inputFrames.size();
	if(!out.fail())
	{
		char nullTerminator = '\0';
		out.write((char*)(&vecSize), sizeof(int));
		out.write((char*)(&recordPosition), sizeof(int));
		out.write(movieName.c_str(), movieName.length());
		out.write(&nullTerminator, sizeof(char));
		out.write(cartName.c_str(), cartName.length());
		out.write(&nullTerminator, sizeof(char));
		char* outputBuffer = new char[inputFrames.size()];

		for(int i = 0; i < (int)inputFrames.size(); i++)
			outputBuffer[i] = (byte)inputFrames[i];

		out.write(outputBuffer, inputFrames.size() * sizeof(byte));
		delete[] outputBuffer;
	}
	out.close();
}


Movie* Movie::loadFromDisk(std::string movieFileName)
{
	std::fstream in;
	in.open(movieFileName, std::ios::binary | std::ios::in);

	char buffer[512];
	int vecSize = 0;
	int recordPosition = 0;

	if(!in.fail())
	{
		in.read((char*)(&vecSize), sizeof(int));
		in.read((char*)(&recordPosition), sizeof(int));

		in.getline((char*)(&buffer), 512, '\0');
		std::string movieName(buffer);
		in.getline((char*)(&buffer), 512, '\0');
		std::string cartName(buffer);

		Movie* mov = new Movie(movieName, "author", movieName, cartName);
		
		mov->inputFrames.reserve(vecSize * sizeof(byte));
		mov->recordPosition = recordPosition;
		char* inputBuffer = new char[vecSize];
		in.read(inputBuffer, vecSize * sizeof(byte));
		for(int i = 0; i < vecSize; i++)
			mov->inputFrames.push_back(inputBuffer[i]);
		delete[] inputBuffer;
		return mov;
	}

	return NULL;
}


Movie* Movie::loadFromDisk(std::fstream& in)
{
	char buffer[512];
	int vecSize = 0;
	int recordPosition = 0;

	if(!in.fail())
	{
		in.read((char*)(&vecSize), sizeof(int));
		in.read((char*)(&recordPosition), sizeof(int));

		in.getline((char*)(&buffer), 512, '\0');
		std::string movieName(buffer);
		in.getline((char*)(&buffer), 512, '\0');
		std::string cartName(buffer);

		Movie* mov = new Movie(movieName, "author", movieName, cartName);
		
		mov->inputFrames.reserve(vecSize * sizeof(byte));
		mov->recordPosition = recordPosition;
		char* inputBuffer = new char[vecSize];
		in.read(inputBuffer, vecSize * sizeof(byte));
		for(int i = 0; i < vecSize; i++)
			mov->inputFrames.push_back(inputBuffer[i]);
		delete[] inputBuffer;
		return mov;
	}

	return NULL;
}

int Movie::getFormat()
{
	return format;
}