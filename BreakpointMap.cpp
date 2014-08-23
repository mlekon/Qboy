#include "BreakpointMap.h"


BreakpointMap::BreakpointMap()
{
	numBreakpoints = 0;
	memset(breakpointMap, 0, sizeof(Breakpoint*) * 0x10000);
	minBreakpoint = maxBreakpoint = 0;

	executeBreakpoints = readBreakpoints = writeBreakpoints = 0;
	//this->addressSpaceSize = addressSpaceSize;
}


/////////////////////////////////
//returns the breakpoint at the given address
Breakpoint* BreakpointMap::getBreakpoint(word address)
{
	return breakpointMap[address];
}


/////////////////////////////////
//returns a valid breakpoint given by a sequential index starting at 0
Breakpoint* BreakpointMap::enumerateBreakpoints(int index)
{
	Breakpoint* bp = breakpointMap[0];

	int j = minBreakpoint;
	for(int i = 0; i < index && j < maxBreakpoint; i++)
	{
		while(bp == NULL && j < maxBreakpoint)
			bp = breakpointMap[j++];
	}

	return bp;
}


/////////////////////////////////
//returns the result of the condition specified by
//the breakpoint at "address"
bool BreakpointMap::checkBreakpoint(word address, byte value)
{
	if(numBreakpoints == 0) return false;

	//check for a breakpoint at this address
	Breakpoint* bp = getBreakpoint(address);
	if(bp == NULL) return false;

	if(bp->address == address)
	{
		//check the condition against the given value
		switch(bp->condition)
		{
		case GreaterThan:
			return value > (bp->value & bp->valueBitmask);
		case LessThan:
			return value < (bp->value & bp->valueBitmask);
		case GreaterThanEqual:
			return value >= (bp->value & bp->valueBitmask);
		case LessThanEqual:
			return value <= (bp->value & bp->valueBitmask);
		case Equal:
			return value == (bp->value & bp->valueBitmask);
		case NotEqual:
			return value != (bp->value & bp->valueBitmask);
		case NoCondition:
			return true;
		};
	}
	return false;
}


/////////////////////////////////
//inserts a breakpoint at a specific address which
//will halt the emulator when that address is executed,
//read from or written to (depending on flags)
void BreakpointMap::addBreakpoint(word address, Breakpoint bp)
{
	breakpointMap[address] = new Breakpoint();
	*(breakpointMap[address]) = bp;

	if(address < minBreakpoint)	minBreakpoint = address;
	if(address > maxBreakpoint)	maxBreakpoint = address;

	//update type counters
	if(bp.type & BreakpointExecute) executeBreakpoints++;
	if(bp.type & BreakpointRead) readBreakpoints++;
	if(bp.type & BreakpointWrite) writeBreakpoints++;
}


/////////////////////////////////
//finds the breakpoint at the next lowest address from the given address
word BreakpointMap::previousBreakpoint(word address)
{
	int i = 0;
	for(i = address; i >= 0; i--)
	{
		if(breakpointMap[i] != NULL)
			return (word)i;
	}

	return 0;
}
	
	
/////////////////////////////////
//finds the breakpoint at the next highest address from the given address
word BreakpointMap::nextBreakpoint(word address)
{
	int i = 0;
	for(i = address; i < addressSpaceSize; i++)
	{
		if(breakpointMap[i] != NULL)
			return (word)i;
	}

	return 0;
}
	



/////////////////////////////////
//removes the breakpoint at the given address if it exists
Breakpoint BreakpointMap::removeBreakpoint(word address)
{
	Breakpoint bp;
	memset(&bp, 0, sizeof(Breakpoint));

	if(breakpointMap[address] != NULL)
	{
		bp = *(breakpointMap[address]);
		int bpType = breakpointMap[address]->type;
		
		if(address == minBreakpoint)	minBreakpoint = address;
		if(address == maxBreakpoint)	maxBreakpoint = address;

		if(bpType & BreakpointExecute) executeBreakpoints--;
		if(bpType & BreakpointRead) readBreakpoints--;
		if(bpType & BreakpointWrite) writeBreakpoints--;

		delete breakpointMap[address];
		breakpointMap[address] = NULL;
		numBreakpoints--;
	}
	return bp;
}


/////////////////////////////////
//remove all breakpoints
void BreakpointMap::clearAll()
{
	memset(breakpointMap, 0, sizeof(Breakpoint*) * addressSpaceSize);
	executeBreakpoints = 0;
	readBreakpoints = 0;
	writeBreakpoints = 0;
}

/////////////////////////////////
//returns a valid breakpoint given by a sequential index starting at 0
//Breakpoint* BreakpointMap::enumerateBreakpoints(int index)
//{
//	if(!readBreakpoints && !executeBreakpoints && !writeBreakpoints) return NULL;
//
//	auto iterator = breakpointMap.begin();
//	auto end = breakpointMap.end();
//	int bpCount = -1;
//
//	//iterate over the entire map
//	while(iterator != end)
//	{
//		//only look for valid (not null) items
//		if((*iterator).second != NULL) 
//		{
//			bpCount++;
//			if(bpCount == index)
//			{
//				//return when the correct number of points have been found
//				return iterator->second;
//			}
//		}
//		iterator++;
//	}
//
//	//nothing was found or the desired breakpoint is the last item
//	return (iterator == breakpointMap.end()) ? NULL : iterator->second;
//}


/////////////////////////////////
//returns the result of the condition specified by
//the breakpoint at "address"
//bool BreakpointMap::checkBreakpoint(word address, byte value)
//{
//	if(numBreakpoints == 0) return false;
//
//	//check for a breakpoint at this address
//	Breakpoint* bp = getBreakpoint(address);
//	if(bp == NULL) return false;
//
//	if(bp->address == address)
//	{
//		//check the condition against the given value
//		switch(bp->condition)
//		{
//		case GreaterThan:
//			return value > (bp->value & bp->valueBitmask);
//		case LessThan:
//			return value < (bp->value & bp->valueBitmask);
//		case GreaterThanEqual:
//			return value >= (bp->value & bp->valueBitmask);
//		case LessThanEqual:
//			return value <= (bp->value & bp->valueBitmask);
//		case Equal:
//			return value == (bp->value & bp->valueBitmask);
//		case NotEqual:
//			return value != (bp->value & bp->valueBitmask);
//		case NoCondition:
//			return true;
//		};
//	}
//	return false;
//}
//
//
///////////////////////////////////
////inserts a breakpoint at a specific address which
////will halt the emulator when that address is executed,
////read from or written to (depending on flags)
//void BreakpointMap::addBreakpoint(word address, Breakpoint bp)
//{
//	breakpointMap[address] = new Breakpoint();
//	*(breakpointMap[address]) = bp;
//
//	//update type counters
//	if(bp.type & BreakpointExecute) executeBreakpoints++;
//	if(bp.type & BreakpointRead) readBreakpoints++;
//	if(bp.type & BreakpointWrite) writeBreakpoints++;
//}
//
//
///////////////////////////////////
////removes the breakpoint at the given address if it exists
//Breakpoint BreakpointMap::removeBreakpoint(word address)
//{
//	Breakpoint bp;
//
//	if(breakpointMap[address] != NULL)
//	{
//		bp = *(breakpointMap[address]);
//
//		//update breakpoint type totals
//		if(bp.type & BreakpointExecute) executeBreakpoints--;
//		if(bp.type & BreakpointRead) readBreakpoints--;
//		if(bp.type & BreakpointWrite) writeBreakpoints--;
//
//		delete breakpointMap[address];
//		breakpointMap[address] = NULL;
//	}
//
//	return bp;
//}
//
//
///////////////////////////////////
////remove all breakpoints
//void BreakpointMap::clearAll()
//{
//	breakpointMap.clear();
//	executeBreakpoints = 0;
//	readBreakpoints = 0;
//	writeBreakpoints = 0;
//}