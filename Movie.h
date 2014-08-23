#ifndef MOVIE_H
#define MOVIE_H

#include <vector>
typedef unsigned char byte;
typedef unsigned short word;

enum EmulationInput
{
	MovieStart,
	MovieStop,
	HardwareReset,
	CartridgeInsert,
	CartridgeRemove
};

class Movie
{
private:
	std::string movieName;
	std::string cartName;
	std::string authorName;
	std::string fileName;
	std::vector<int> keyFrames;
	std::vector<word> inputFrames;
	byte previousInput;

	int rerecordCount;
	int previousFrame;
	int playbackPosition;
	int recordPosition;

	int format;
public:
	Movie(std::string movieName, std::string authorName, std::string fileName, std::string cartName);

	void setAuthor(std::string author);
	void setMovieName(std::string name);

	void record(byte p20, byte p10, byte emuInput, int currentFrame);
	void record(byte input, byte emuInput, int currentFrame);
	void unrecordToFrame(int frame);
	void unrecordLast();

	byte playback();
	byte playback(int frame);
	byte playbackEmu(int frame);
	void setPlaybackPosition(int frame);
	void setRelativePlaybackPosition(int offset);
	void setFormat(int format);

	int getFormat();

	void writeToDisk(int format);
	void qboyWriteToDisk();
	void vbaWriteToDisk();
	void writeToDisk(std::fstream& out);
	static Movie* loadFromDisk(std::string);
	static Movie* loadFromDisk(std::fstream& in);
};

#endif	//MOVIE_H