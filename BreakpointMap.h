#ifndef BREAKPOINTMAP_H
#define BREAKPOINTMAP_H

#include "enum.h"
#include <vector>
#include <list>
#include <map>

class BreakpointMap
{
private:
	int numPages;
	int numBreakpoints;
	int minBreakpoint;
	int maxBreakpoint;

	static const int addressSpaceSize = 0x10000;

	void addBreakpointToTree(Breakpoint* bp);
public:
	int executeBreakpoints;
	int readBreakpoints;
	int writeBreakpoints;	
	//std::map<int, Breakpoint*> breakpointMap;
	Breakpoint* breakpointMap[0x10000];


	BreakpointMap();

	Breakpoint* getBreakpoint(word address);
	Breakpoint* enumerateBreakpoints(int index);
	bool checkBreakpoint(word address, byte value);
	void addBreakpoint(word address, Breakpoint bp);
	Breakpoint removeBreakpoint(word address);
	word nextBreakpoint(word address);
	word previousBreakpoint(word address);

	void clearAll();
};

#endif	//BREAKPOINTMAP_H

