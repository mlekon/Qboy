#pragma once


#include "enum.h"
#include "resource.h"

#define ID_RERECORDING_PANEL_TIMER 44444
#define	ID_LOADRECENT 35000

INT_PTR CALLBACK RewindBufferProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ControlsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ReRecWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MemViewWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DisassemblyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MemSrchWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BreakpointListProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddBreakpointProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MemVisualizerProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MovieSetupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK EnhButtonSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

BOOL CALLBACK ColorsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void calcRewindStats(HWND);
DWORD WINAPI RunGameboy(LPVOID lpParameter);
int MenuHeight(HWND hwnd, HMENU hmenu);
void ClientResize(HWND hWnd, int scale); 
void UpdateCartInfo(HWND hwnd);
void initDisassembly(wchar_t** d, char* o, char* clocks);
void initRegistry();
void shutdownRegistry();
void updateRecentFileList(HWND hWnd);