#ifndef GB_C
#define GB_C

#include "allegro.h"
#include "winalleg.h"
#include "Gameboy.h"
#include <time.h>
#include "stdafx.h"
#include "gb.h"
#include <Windows.h>
#include <WindowsX.h>
#include <winsync.h>
#include "Commctrl.h"
#include "commdlg.h"

#define MAX_LOADSTRING 100

WCHAR totalSize[64];
WCHAR totallength[64];

CHOOSECOLOR cc;
OPENFILENAME fn;
COLORREF cr[16];
HBRUSH hb;
WCHAR filePath[MAX_PATH];
StrBuffer recentFiles[10];
DWORD gbColors[4];
RomInfo ri;
RegisterInfo regi;
UINT visualizerTimer;
UINT rerecTimer;
UINT brokenTimer;

int saveInterval = 1;
int bufferStates = 500;
int loadInterval = 1;
int volume = 100;
int zoom = 3;
const int displayX = 160;
const int displayY = 144;
const int maxStates = 500;
const int minStates = 1;
const int sizePerState = 16384;
const int minLoadInt = 1;
const int maxLoadInt = 10000;
const int minSaveInt = 1;
const int maxSaveInt = 10000;

int checkedVolumeItem;
int checkedZoomItem;

wchar_t* disassemblyStrings[256];
wchar_t* cbDisassemblyStrings[256];
char opcodeOperands[512];
char clocks[512];

wchar_t buf[5];
wchar_t* columnNames[] = 
{
	L"", L"0", L"1", L"2", L"3", L"4",
	L"5", L"6", L"7", L"8", L"9", L"A",
	L"B", L"C", L"D", L"E", L"F"
};

wchar_t* registerNames[] =
{
	L"F", L"A", L"C", L"B", L"E", L"D", L"L", L"H",
	L"SP", L"PC", L"HF", L"ZF", L"NF",L"CF"
};

wchar_t* cartridgeTypes[] =
{
	L"ROM",
	L"ROM+MBC1",
	L"ROM+MBC1+RAM",
	L"ROM+MBC1+RAM+BATT",
	L"",
	L"ROM+MBC2",
	L"ROM+MBC2+BATT",
	L"",
	L"ROM_RAM",
	L"ROM+RAM+BATT",
	L"",
	L"ROM+MMMO1",
	L"ROM+MMMO1+SRAM",
	L"ROM+MMMO1+SRAM+BATT",
	L"", L"", L"", L"",
	L"ROM+MBC3+RAM",
	L"ROM+MBC3+RAM+BATT",
	L"", L"", L"", L"", L"",
	L"ROM+MBC5",
	L"ROM+MBC5+RAM",
	L"ROM+MBC5+RAM+BATT",
	L"ROM+MBC5+RUMBLE",
	L"ROM+MBC5+RUMBLE+SRAM",
	L"ROM+MBC5+RUMBLE+SRAM+BATT",
	L"Pocket Camera",
	L"Bandai TAMA5",
	L"Hudson HuC-3"
};

LVCOLUMN columns[17];

HWND dialogs[DlgMax];
HWND controls = NULL;
HWND rerecPanel = NULL;
HWND memPanel = NULL;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HANDLE gbThreadHandle;
DWORD gbThreadId;
Gameboy* gb = NULL;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL				CheckDialogs(LPMSG msg);

int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	MSG msg;
	HACCEL hAccelTable;

	ZeroMemory(dialogs, sizeof(dialogs));

	initDisassembly(disassemblyStrings, opcodeOperands, clocks);
	initRegistry();

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = 0xffffffff;
	InitCommonControlsEx(&icc);


	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINTEST));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) && !CheckDialogs(&msg))/*!IsDialogMessage(memPanel, &msg) && 
			!IsDialogMessage(rerecPanel, &msg) && !IsDialogMessage(controls, &msg))*/
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	shutdownRegistry();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ZeroMemory(&fn, sizeof(fn));
	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = hWnd;
	fn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	fn.lpstrFile = filePath;
	fn.nMaxFile = sizeof(filePath);
	fn.lpstrFilter = L".gb\0";
	fn.nFilterIndex = 0;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = NULL;
	fn.lpstrInitialDir = NULL;

	win_set_window(hWnd);
	int menuHeight = 0;


	for(int i = 0; i < 10; i++)
	{
		if(_tcslen(recentFiles[i]) == 0)
			wcscpy_s(recentFiles[i], L"---");
	}

	MENUITEMINFO mii;
	mii.fMask = MIIM_SUBMENU;

	updateRecentFileList(hWnd);

	RECT r;
	GetMenuItemRect(hWnd, GetMenu(hWnd), 0, &r);
	ClientResize(hWnd, zoom);
	ClientResize(hWnd, zoom);
	gbThreadHandle = CreateThread(NULL, 0, RunGameboy, NULL, NULL, &gbThreadId);
	rerecTimer = SetTimer(rerecPanel, ID_RERECORDING_PANEL_TIMER, 100, NULL);
	brokenTimer = SetTimer(dialogs[BreakpointList], ID_BREAKPOINT_TIMER, 200, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		dialogs[Disassemble] =				CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DISASSEMBLY), hWnd, (DLGPROC)DisassemblyWindowProc);
		dialogs[MemViewer] = memPanel =		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MEM), hWnd, (DLGPROC)MemViewWindowProc);
		dialogs[Rerecording] = rerecPanel = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RERECORDING_PANEL), hWnd, (DLGPROC)ReRecWindowProc);
		dialogs[Controls] = controls =		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CONTROLS), hWnd, (DLGPROC)ControlsWindowProc);
		dialogs[MemSearch] =				CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MEM_SEARCH), hWnd, (DLGPROC)MemSrchWindowProc);
		dialogs[BreakpointList] =			CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_BREAKPOINTS), hWnd, (DLGPROC)BreakpointListProc);
		dialogs[Visualizer] =				CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_VISUALIZER), hWnd, (DLGPROC)MemVisualizerProc);
		dialogs[MoviePanel] =				CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MOVIE_SETUP), hWnd, (DLGPROC)MovieSetupProc);
		break;
	case WM_TIMER:
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_OPEN_ROM:
			{
				bool wasPaused = gb->pause(true);
				char fileNameTemp[512];	
				if(GetOpenFileName(&fn) != 0)
				{
					WideCharToMultiByte(CP_ACP, 0, fn.lpstrFile, -1, fileNameTemp, sizeof(fileNameTemp), 0, 0);

					for(int i = 8; i >= 0; i--)
						wcscpy_s(recentFiles[i + 1], recentFiles[i]);
					wcscpy_s(recentFiles[0], fn.lpstrFile);

					updateRecentFileList(hWnd);

					gb->loadGame(fileNameTemp);//load rom
				}
		
				ComboBox_ResetContent(GetDlgItem(memPanel, IDC_DATA_SOURCE));
				ri = gb->getRomInfo();

				wchar_t buf[20];
				ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), L"System Memory");
				for(int i = 0; i < ri.numRomBanks; i++)
				{
					swprintf_s(buf, L"ROM Bank %d", i);
					ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), buf);
				}
				for(int i = 0; i < ri.numRamBanks; i++)
				{
					swprintf_s(buf, L"RAM Bank %d", i);
					ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), buf);
				}

				gb->pause(wasPaused);
			}
			break;
		case ID_FILE_ROMINFO:
			{
				ri = gb->getRomInfo();

				wchar_t buf[20];
				ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), L"System Memory");
				for(int i = 0; i < ri.numRomBanks; i++)
				{
					swprintf_s(buf, L"ROM Bank %d", i);
					ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), buf);
				}
				for(int i = 0; i < ri.numRamBanks; i++)
				{
					swprintf_s(buf, L"RAM Bank %d", i);
					ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), buf);
				}
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)About);
			}
			break;
		case ID_EMULATION_VISUALIZER:
			if(dialogs[Visualizer] != NULL)
			{
				ShowWindow(dialogs[Visualizer], SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			break;
		case IDM_EXIT:
			gb->exitGame();
			delete gb;
			DestroyWindow(hWnd);
			break;
		case ID_EMULATION_REWINDBUFFER:
			if(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REWIND_DIALOG), hWnd, (DLGPROC)RewindBufferProc))
			{
				gb->rewindLoadInterval(loadInterval);
				gb->rewindSaveInterval(saveInterval);
				gb->rewindBufferSize(bufferStates);
			}
			gb->pause(false);
			break;
		case ID_DISPLAY_SETCOLORS:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_COLORS), hWnd, (DLGPROC)ColorsProc);
			gb->setColors(gbColors[0], gbColors[1], gbColors[2], gbColors[3]);
			gb->pause(false);
			break;
		case ID_FILE_RESET:
			gb->reset();
			break;
		case ID_EMULATION_NEXTFRAME:
			gb->nextFrame(1);
			break;
		case ID_EMULATION_BREAKPOINTS:
			if(dialogs[BreakpointList] != NULL)
			{
				ShowWindow(dialogs[BreakpointList], SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			break;
		case ID_LOADRECENT_11 + 0:
		case ID_LOADRECENT_11 + 1:
		case ID_LOADRECENT_11 + 2:
		case ID_LOADRECENT_11 + 3:
		case ID_LOADRECENT_11 + 4:
		case ID_LOADRECENT_11 + 5:
		case ID_LOADRECENT_11 + 6:
		case ID_LOADRECENT_11 + 7:
		case ID_LOADRECENT_11 + 8:
		case ID_LOADRECENT_11 + 9:
			{
				int index = wmId - ID_LOADRECENT_11;
				WIN32_FIND_DATA fileFind;

				if(recentFiles[index][0] == L'-')
				{
					// No file name present. Don't do anything
				}
				else if(FindFirstFile(recentFiles[index],&fileFind) != INVALID_HANDLE_VALUE)
				{
					bool wasPaused = gb->pause(true);
					char temp[256];
					WideCharToMultiByte(CP_ACP, 0, recentFiles[index], -1, temp, sizeof(recentFiles[index]), 0, 0);

					gb->loadGame(temp);//load rom
		
					ComboBox_ResetContent(GetDlgItem(memPanel, IDC_DATA_SOURCE));
					ri = gb->getRomInfo();

					wchar_t buf[20];
					ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), L"System Memory");
					for(int i = 0; i < ri.numRomBanks; i++)
					{
						swprintf_s(buf, L"ROM Bank %d", i);
						ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), buf);
					}
					for(int i = 0; i < ri.numRamBanks; i++)
					{
						swprintf_s(buf, L"RAM Bank %d", i);
						ComboBox_AddString(GetDlgItem(memPanel, IDC_DATA_SOURCE), buf);
					}
					gb->pause(wasPaused);
				}
				else
				{
					MessageBox(NULL, L"ROM File not found.", L"Error", MB_OK);
					for(int i = index; i < 8; i++)
						wcscpy_s(recentFiles[i], recentFiles[i + 1]);
					wcscpy_s(recentFiles[9], L"---");

					updateRecentFileList(hWnd);
				}
			}
			break;
		case ID_LOADSTATE_STATE0:
			gb->loadUserState(0);
			break;
		case ID_LOADSTATE_STATE1:
			gb->loadUserState(1);
			break;
		case ID_LOADSTATE_STATE2:
			gb->loadUserState(2);
			break;
		case ID_LOADSTATE_STATE3:
			gb->loadUserState(3);
			break;
		case ID_LOADSTATE_STATE4:
			gb->loadUserState(4);
			break;
		case ID_LOADSTATE_STATE5:
			gb->loadUserState(5);
			break;
		case ID_LOADSTATE_STATE6:
			gb->loadUserState(6);
			break;
		case ID_LOADSTATE_STATE7:
			gb->loadUserState(7);
			break;
		case ID_LOADSTATE_STATE8:
			gb->loadUserState(8);
			break;
		case ID_LOADSTATE_STATE9:
			gb->loadUserState(9);
			break;
		case ID_SAVESTATE_STATE0:
			gb->saveUserState(0);
			break;
		case ID_SAVESTATE_STATE1:
			gb->saveUserState(1);
			break;
		case ID_SAVESTATE_STATE2:
			gb->saveUserState(2);
			break;
		case ID_SAVESTATE_STATE3:
			gb->saveUserState(3);
			break;
		case ID_SAVESTATE_STATE4:
			gb->saveUserState(4);
			break;
		case ID_SAVESTATE_STATE5:
			gb->saveUserState(5);
			break;
		case ID_SAVESTATE_STATE6:
			gb->saveUserState(6);
			break;
		case ID_SAVESTATE_STATE7:
			gb->saveUserState(7);
			break;
		case ID_SAVESTATE_STATE8:
			gb->saveUserState(8);
			break;
		case ID_SAVESTATE_STATE9:
			gb->saveUserState(9);
			break;
		case ID_WINDOWZOOM_1X:
			zoom = 1;
			gb->setDisplayScale(1);
			ClientResize(hWnd, 1);
			ClientResize(hWnd, 1);
			break;
		case ID_WINDOWZOOM_2X:
			zoom = 2;
			gb->setDisplayScale(2);
			ClientResize(hWnd, 2);
			ClientResize(hWnd, 2);
			break;
		case ID_WINDOWZOOM_3X:
			zoom = 3;
			gb->setDisplayScale(3);
			ClientResize(hWnd, 3);
			ClientResize(hWnd, 3);
			break;
		case ID_WINDOWZOOM_4X:
			zoom = 4;
			gb->setDisplayScale(4);
			ClientResize(hWnd, 4);
			ClientResize(hWnd, 4);
			break;
		case ID_WINDOWZOOM_5X:
			zoom = 5;
			gb->setDisplayScale(5);
			ClientResize(hWnd, 5);
			ClientResize(hWnd, 5);
			break;
		case ID_VOLUME_MUTE:
			volume = 0;
			gb->setSoundLevel(0);
			break;
		case ID_VOLUME_20:
			volume = 20;
			gb->setSoundLevel(.2);
			break;
		case ID_VOLUME_40:
			volume = 40;
			gb->setSoundLevel(.4);
			break;
		case ID_VOLUME_60:
			volume = 60;
			gb->setSoundLevel(.6);
			break;
		case ID_VOLUME_80:
			volume = 80;
			gb->setSoundLevel(.8);
			break;
		case ID_VOLUME_100:
			volume = 100;
			gb->setSoundLevel(1);
			break;
		case ID_EMULATION_PAUSE:
			gb->pause();
			break;
		case ID_SPEED_10:
			gb->setEmulationSpeed(0);
			break;
		case ID_SPEED_0:
			gb->setEmulationSpeed(1);
			break;
		case ID_SPEED_1:
			gb->setEmulationSpeed(2);
			break;
		case ID_SPEED_2:
			gb->setEmulationSpeed(3);
			break;
		case ID_SPEED_3:
			gb->setEmulationSpeed(4);
			break;
		case ID_SPEED_4:
			gb->setEmulationSpeed(5);
			break;
		case ID_SPEED_5:
			gb->setEmulationSpeed(6);
			break;
		case ID_SPEED_6:
			gb->setEmulationSpeed(7);
			break;
		case ID_SPEED_7:
			gb->setEmulationSpeed(8);
			break;
		case ID_SPEED_8:
			gb->setEmulationSpeed(9);
			break;
		case ID_SPEED_9:
			gb->setEmulationSpeed(10);
			break;
		case ID_SPEED_MAX:
			gb->setEmulationSpeed(11);
			break;
		case ID_EMULATION_MEMORYVIEWER:
			if(memPanel != NULL)
				ShowWindow(memPanel, SW_SHOW);
		case ID_CHANNELS_CHANNEL1:
			gb->soundModeToggle(0);
			break;
		case ID_CHANNELS_CHANNEL2:
			gb->soundModeToggle(1);
			break;
		case ID_CHANNELS_CHANNEL3:
			gb->soundModeToggle(2);
			break;
		case ID_CHANNELS_CHANNEL4:
			gb->soundModeToggle(3);
			break;
		case ID_RECORDING_BEGINRECORDING:
			if(rerecPanel != NULL)
			{
				ShowWindow(dialogs[MoviePanel], SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			//gb->pause(true);
			//gb->startRecording();
			//gb->pause(false);
			break;
		case ID_RECORDING_STOPRECORDING:
			gb->stopRecording();
			break;
		case ID_RECORDING_LOADRECORDING:
			{
				gb->pause(true);
				GetOpenFileName(&fn);
				char fileNameTemp[512];	
				WideCharToMultiByte(CP_ACP, 0, fn.lpstrFile, -1, fileNameTemp, sizeof(fileNameTemp), 0, 0);
				gb->loadRecording(fileNameTemp);
				gb->pause(false);
			}
			break;
		case ID_RECORDING_STOPPLAYBACK:
			gb->stopPlayback();
			break;
		case ID_RECORDING_RERECORDINGPANEL:
			if(rerecPanel != NULL)
			{
				ShowWindow(rerecPanel, SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			break;
		case ID_REWIND_PREVIOUSSTATE:
			gb->pause(true);
			gb->rewindFrames(1);
			break;
		case ID_REWIND_SAVENOW:
			gb->saveState();
			break;
		case ID_EMULATION_DISASSEMBLY:
			if(dialogs[Disassemble] != NULL)
			{
				ShowWindow(dialogs[Disassemble], SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			break;
		case ID_CONTROLS:
			if(controls != NULL)
			{
				ShowWindow(controls, SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		gb->exitGame();
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		UpdateCartInfo(hWnd);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hWnd, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL CALLBACK ColorsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		gb->pause(true);
		ZeroMemory(&cc, sizeof(cc));
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = hWnd;
		cc.lpCustColors = (LPDWORD) cr;
		cc.rgbResult = 0;
		cc.Flags = CC_RGBINIT;
		gbColors[0] = gb->getColor(0);
		gbColors[1] = gb->getColor(1);
		gbColors[2] = gb->getColor(2);
		gbColors[3] = gb->getColor(3);
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_COLOR1:
			cc.rgbResult = gb->getColor(0);
			gbColors[0] = gb->getColor(0);
			if(ChooseColor(&cc) != 0)
				gbColors[0] = cc.rgbResult;
			break;
		case IDC_COLOR2:
			cc.rgbResult = gb->getColor(1);
			gbColors[1] = gb->getColor(1);
			if(ChooseColor(&cc) != 0)
				gbColors[1] = cc.rgbResult;
			break;
		case IDC_COLOR3:
			cc.rgbResult = gb->getColor(2);
			gbColors[2] = gb->getColor(2);
			if(ChooseColor(&cc) != 0)
				gbColors[2] = cc.rgbResult;
			break;
		case IDC_COLOR4:
			cc.rgbResult = gb->getColor(3);
			gbColors[3] = gb->getColor(3);
			if(ChooseColor(&cc) != 0)
				gbColors[3] = cc.rgbResult;
			break;
		case IDC_COLOR5:
			{
				int overlayColor = 0;
				if(ChooseColor(&cc) != 0)
					gb->setTextOverlayColor(cc.rgbResult);
			}
			break;
		case IDOK:
			EndDialog(hWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;
		default:
			return false;
		};
		return true;
	default:;
	};

	return false;
}

INT_PTR CALLBACK RewindBufferProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			gb->pause(true);
			long x = MAKELONG((short)maxStates, (short)minStates);
			SendDlgItemMessage(hWnd, IDC_STATES_SPIN, UDM_SETRANGE, 0, MAKELONG((short)maxStates, (short)minStates));
			SendDlgItemMessage(hWnd, IDC_LOAD_SPIN, UDM_SETRANGE, 0, MAKELONG((short)maxLoadInt, (short)minLoadInt));
			SendDlgItemMessage(hWnd, IDC_SAVE_SPIN, UDM_SETRANGE, 0, MAKELONG((short)maxSaveInt, (short)minSaveInt));
			SetDlgItemInt(hWnd, IDC_BUFFER_STATES, bufferStates, FALSE);
			SetDlgItemInt(hWnd, IDC_LOAD_INTERVAL, loadInterval, FALSE);
			SetDlgItemInt(hWnd, IDC_SAVE_INTERVAL, saveInterval, FALSE);
			SetDlgItemText(hWnd, IDC_SIZE_PER_STATE, L"16384");
			SetDlgItemInt(hWnd, IDC_BUFFER_STATES, bufferStates, FALSE);
			_itow_s(bufferStates * sizePerState, totalSize, 64, 10);
			calcRewindStats(hWnd);
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case UDN_DELTAPOS:
			if(((NMUPDOWN*)lParam)->hdr.idFrom == IDC_STATES_SPIN)
			{
				NMUPDOWN* hdr = (NMUPDOWN*)lParam;
				BOOL b;
				bufferStates = GetDlgItemInt(hWnd, IDC_STATES_SPIN, &b, FALSE);
				int newVal = (hdr->iDelta + hdr->iPos);
				if(newVal > maxStates)
					newVal = maxStates;
				else if(newVal < minStates)
					newVal = minStates;

				calcRewindStats(hWnd);
				return true;
			}
			else if(((NMUPDOWN*)lParam)->hdr.idFrom == IDC_LOAD_SPIN)
			{
				calcRewindStats(hWnd);
			}
			else if(((NMUPDOWN*)lParam)->hdr.idFrom == IDC_SAVE_SPIN)
			{
				calcRewindStats(hWnd);
			}
		}
		return false;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case EN_CHANGE:
			calcRewindStats(hWnd);
			break;
		case IDOK:
			calcRewindStats(hWnd);
			EndDialog(hWnd, IDOK);
			break;		
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

INT_PTR CALLBACK MemViewWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			HWND list = GetDlgItem(hWnd, IDC_MEM_LIST);

			LVCOLUMN c;
			c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
			c.cx = 40; c.cxMin = 40; c.cxIdeal = 40;
			c.pszText = columnNames[0];
			c.iSubItem = 0;
			c.iOrder = 0;
			ListView_InsertColumn(list, 0, &c);

			for(int j = 1; j < 17; j++)
			{
				columns[j].mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
				columns[j].cx = 25;
				columns[j].cxMin = 25;
				columns[j].iSubItem = j;
				columns[j].pszText = columnNames[j];
				columns[j].iOrder = j;
				ListView_InsertColumn(list, j+1, &columns[j]);
			}
			c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
			c.cx = 120; c.cxMin = 120; c.cxIdeal = 120;
			c.iSubItem = 17;
			c.pszText = L"Text";
			c.iOrder = 17;		
			ListView_InsertColumn(list, 17, &c);

			ListView_SetItemCountEx(list, 4096, NULL);
			return true;
		}
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(dialogs[MemViewer], IDOK);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code)
		{
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam; 

			if(plvdi->hdr.idFrom == IDC_MEM_LIST)
			{
				if(plvdi->item.mask & LVIF_TEXT)
				{
					if(plvdi->item.iSubItem == 0)
					{
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", plvdi->item.iItem * 16);
					}
					else if(plvdi->item.iSubItem == 17)
					{
						wchar_t txt[17];

						int source = ComboBox_GetCurSel(GetDlgItem(memPanel, IDC_DATA_SOURCE));

						if(source == 0 || source < 0)
						{
							for(int i = 0; i < 16; i++)
								txt[i] = gb->readMemory(plvdi->item.iItem * 16 + i);
						}
						else if(source > 0 && source <= ri.numRomBanks)
						{
							for(int i = 0; i < 16; i++)
								txt[i] = gb->readCartridgeROM(source-1, plvdi->item.iItem * 16 + i);
						}
						else
						{
							for(int i = 0; i < 16; i++)
								txt[i] = gb->readCartridgeRAM(source-1, plvdi->item.iItem * 16 + i);

						}
	
						for(int i = 0; i < 16; i++)	
						{
							if(txt[i] <= 31) 
								txt[i] = '.';
						}
						txt[16] = 0;
						wcscpy_s(plvdi->item.pszText, plvdi->item.cchTextMax, txt);
					}
					else
					{
						byte m;
						int source = ComboBox_GetCurSel(GetDlgItem(memPanel, IDC_DATA_SOURCE));
							
						if(source == 0 || source < 0)
							m = (gb == NULL) ? 0 : gb->readMemory(plvdi->item.iItem * 16 + (plvdi->item.iSubItem-1));
						else if(source > 0 && source <= ri.numRomBanks)
							m = (gb == NULL) ? 0 : gb->readCartridgeROM(source-1, plvdi->item.iItem * 16 + (plvdi->item.iSubItem-1));
						else
							m = (gb == NULL) ? 0 : gb->readCartridgeRAM(source-1-ri.numRomBanks, plvdi->item.iItem * 16 + (plvdi->item.iSubItem-1));
						swprintf_s(buf, L"%.2X", m);
						plvdi->item.pszText = buf;
					}
				}

				if(plvdi->item.mask & LVIF_PARAM)
				{
					byte m = (gb == NULL) ? 0 : gb->readMemory(plvdi->item.iItem);
					plvdi->item.lParam = m;
				}
			}
			return true;
		}
		case LVN_ODCACHEHINT:
			return true;
		case LVN_ODFINDITEM:
			return -1;
		default:
			return false;
		};
	case WM_COMMAND:
		if(LOWORD(wParam) != IDCANCEL && !gb->cartridgeIsLoaded())
			return false;

		switch(HIWORD(wParam))
		{
		case CBN_SELCHANGE:
		{
			HWND list = GetDlgItem(memPanel, IDC_MEM_LIST);
			int index2 = ListView_GetTopIndex(list);
			int index = ComboBox_GetCurSel(GetDlgItem(memPanel, IDC_DATA_SOURCE));

			if(index == 0 || index < 0)
				ListView_SetItemCountEx(list, 4096, NULL);
			else if(index > 0 && index <= ri.numRomBanks)
				ListView_SetItemCountEx(list, 1024, NULL);
			else
				ListView_SetItemCountEx(list, 1024, NULL);


			ListView_RedrawItems(list, index2, index2 + 400);
			break;
		}
		default:
			break;
		};

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(memPanel, IDOK);
			return true;
		case IDC_MEM_SEARCH_BTN:
			if(dialogs[MemSearch] != NULL)
			{
				ShowWindow(dialogs[MemSearch], SW_SHOW);
			}
			else
			{
				MessageBox(hWnd, L"CreateDialog returned NULL", L"Warning!",  
					MB_OK | MB_ICONINFORMATION);
			}
			break;
		case IDC_MEM_REFRESH:
			{
				HWND list = GetDlgItem(memPanel, IDC_MEM_LIST);
				int index = ListView_GetTopIndex(list);
				ListView_RedrawItems(list, index, index + 400);
			}
			break;
		case IDC_GOTO_ADDR:
			{
				HWND list = GetDlgItem(memPanel, IDC_MEM_LIST);
				wchar_t addr[8];
				wchar_t addr2[10];
				GetDlgItemText(memPanel, IDC_ADDR_TXT, addr, 8);

				wcscpy_s(addr2, addr);

				long a = wcstoul(addr2, NULL, 16);
				ListView_EnsureVisible(list, (abs(a) & 0xfff0) / 16, FALSE);
			}
			break;
		default:
			return false;

		break;
		};
	default:
		return false;
	};
	return false;
}

INT_PTR CALLBACK ReRecWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_REWIND), EnhButtonSubclassProc, 0,IDC_REREC_REWIND);
		//SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_PREV), EnhButtonSubclassProc, 0, IDC_REREC_PREV);
		//SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_RECORD), EnhButtonSubclassProc, 0, IDC_REREC_RECORD);
		//SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_PAUSE), EnhButtonSubclassProc, 0, IDC_REREC_PAUSE);
		//SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_PLAY), EnhButtonSubclassProc, 0, IDC_REREC_PLAY);
		//SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_NEXT), EnhButtonSubclassProc, 0, IDC_REREC_NEXT);
		SetWindowSubclass(GetDlgItem(hWnd, IDC_REREC_FF), EnhButtonSubclassProc, 0, IDC_REREC_FF);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code)
		{
		case NM_CLICK:
			{
				NMITEMACTIVATE nm = *((NMITEMACTIVATE*)lParam);
				if(nm.hdr.idFrom == IDC_PROGRESS1)
				{
					int x = 0;
				}
				LVHITTESTINFO hti;
				hti.pt = nm.ptAction;
				int subitem = ListView_SubItemHitTest(hWnd, &hti);
			
				break;
			}
		}
		break;
	case WM_TIMER:
		switch(wParam)
		{
		case ID_RERECORDING_PANEL_TIMER:
			if(gb != NULL)
			{
				wchar_t buffer[10];

				swprintf_s(buffer, L"%.2f%%", float(gb->getCurrentEmulationSpeed()) / 10.0f);
				SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETRANGE32, 0, gb->totalBufferSize());
				SendDlgItemMessage(hWnd, IDC_PROGRESS1, PBM_SETPOS, gb->filledBufferSize(), 0);
				SetDlgItemInt(hWnd, IDC_FRAME_NUM, gb->getFrameCount(), true);
				SetDlgItemInt(hWnd, IDC_LAG_FRAMES, gb->getLagFrames(), true);
				SetDlgItemText(hWnd, IDC_EMU_SPEED, buffer);
			}
			return 0;

		default:
			return false;
		}
		break;
	case WM_QUIT:
	case WM_DESTROY:
		KillTimer(hWnd, rerecTimer);
		EndDialog(rerecPanel, IDOK);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(rerecPanel, IDOK);
			return true;
		case IDC_OPEN_MEM_VIEWER:
			if(memPanel != NULL)
			{
				ShowWindow(memPanel, SW_SHOW);
			}
			break;
		case IDC_SHOW_CONTROLS:
			if(controls != NULL)
				ShowWindow(controls, SW_SHOW);
		case IDC_REREC_PREV:
			gb->pause(true);
			gb->rewindFrames(1);
			break;
		case IDC_REREC_RECORD:
			ShowWindow(dialogs[Rerecording], SW_SHOW);
			break;
		case IDC_REREC_PAUSE:
			gb->pause(true);
			break;
		case IDC_REREC_PLAY:
			gb->pause(false);
			break;
		case IDC_REREC_NEXT:
			gb->nextFrame(1);
			break;
		}
	default:
		return false;
	};
	return true;
}

INT_PTR CALLBACK DisassemblyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
	{
		HWND list = GetDlgItem(hWnd, IDC_DISASSEMBLY_LIST);
		HWND regList = GetDlgItem(hWnd, IDC_REG_LIST);
		HWND stackList = GetDlgItem(hWnd, IDC_STACK_LIST);

		LVCOLUMN c;
		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 25; c.cxMin = 25; c.cxIdeal = 25;
		c.pszText = L"  ";
		c.iSubItem = 0;
		c.iOrder = 0;
		ListView_InsertColumn(list, 0, &c);

		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 40; c.cxMin = 40; c.cxIdeal = 40;
		c.pszText = L"Addr";
		c.iSubItem = 1;
		c.iOrder = 1;
		ListView_InsertColumn(list, 1, &c);
		
		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 50; c.cxMin = 50; c.cxIdeal = 50;
		c.pszText = L"Hex";
		c.iSubItem = 2;
		c.iOrder = 2;
		ListView_InsertColumn(list, 2, &c);

		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 160; c.cxMin = 160; c.cxIdeal = 160;
		c.pszText = L"Disassembly";
		c.iSubItem = 3;
		c.iOrder = 3;
		ListView_InsertColumn(list, 3, &c);
		ListView_SetItemCountEx(list, 1024, NULL);

		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 40; c.cxMin = 40; c.cxIdeal = 40;
		c.pszText = L"Addr";
		c.iSubItem = 0;
		c.iOrder = 0;
		ListView_InsertColumn(stackList, 0, &c);
		
		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 50; c.cxMin = 50; c.cxIdeal = 50;
		c.pszText = L"Stack";
		c.iSubItem = 1;
		c.iOrder = 1;
		ListView_InsertColumn(stackList, 1, &c);
		ListView_SetItemCountEx(stackList, 256, NULL);

		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 35;
		c.cxMin = 35;
		c.iSubItem = 0;
		c.pszText = L"Reg";
		c.iOrder = 0;
		ListView_InsertColumn(regList, 0, &c);

		c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
		c.cx = 70;
		c.cxMin = 70;
		c.iSubItem = 1;
		c.pszText = L"Val";
		c.iOrder = 1;
		ListView_InsertColumn(regList, 1, &c);
		ListView_SetItemCountEx(regList, 14, NULL);
	}
		break;
	case WM_NOTIFY:
		if(!gb || !gb->cartridgeIsLoaded())
			return false;

		switch (((LPNMHDR) lParam)->code)
		{
		case NM_CLICK:
		{
			NMITEMACTIVATE nm = *((NMITEMACTIVATE*)lParam);
			LVHITTESTINFO hti;
			hti.pt = nm.ptAction;
			int subitem = ListView_SubItemHitTest(hWnd, &hti);
			
			if(subitem == 0)
			{
				Breakpoint bp;
				Instruction i = gb->getInstruction(nm.iItem);
				if(gb->getBpm()->getBreakpoint(i.address))
				{
					gb->removeBreakpoint(i.address);
				}
				else
				{
					bp.type = BreakpointExecute;
					bp.address = i.address;
					gb->addBreakpoint(bp);
				}
				HWND list = GetDlgItem(hWnd, IDC_DISASSEMBLY_LIST);
				ListView_RedrawItems(list, 0, 128);
			}

			break;
		}
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam; 
			Instruction i;

			if(plvdi->hdr.idFrom == IDC_DISASSEMBLY_LIST)
			{
				if(plvdi->item.mask & LVIF_TEXT)
				{
					i = gb->getInstruction(plvdi->item.iItem);
					if(plvdi->item.iSubItem == 0)
					{
						Breakpoint* bp = gb->getBpm()->getBreakpoint(i.address);
						if(bp)
						{
							switch(bp->type)
							{
							case BreakpointExecute:
								swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"E");
								break;
							case BreakpointRead:
								swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"R");
								break;
							case BreakpointWrite:
								swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"W");
								break;
							};
						}
					}
					else if(plvdi->item.iSubItem == 1)
					{
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%0.4X", i.address);
					}
					else if(plvdi->item.iSubItem == 2)
					{	

						wchar_t* format = NULL;
						switch(i.numP)
						{
						case 0:
							format = L"%.2X";
							break;
						case 1:
							format = L"%.2X%.2X";
							break;
						case 2:
							format = L"%.2X%.2X%.2X";
						default:
							format = L"";
						}
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, format, i.opcode, i.p1, i.p2);
					}
					else
					{
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, disassemblyStrings[i.opcode], i.p2, i.p1);
					}
				}

				if(plvdi->item.mask & LVIF_PARAM)
				{
					byte m = (gb == NULL) ? 0 : gb->readMemory(plvdi->item.iItem);
					plvdi->item.lParam = m;
				}
			}
			else if(plvdi->hdr.idFrom == IDC_STACK_LIST)
			{
				if(plvdi->item.mask & LVIF_TEXT)
				{
					i = gb->getInstruction(plvdi->item.iItem);
					if(plvdi->item.iSubItem == 0)
					{
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%0.4X", gb->getRegister(SP) - (plvdi->item.iItem * 2));
					}
					else
					{	
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%0.4X", gb->readStack(plvdi->item.iItem * 2));
					}
				}

				if(plvdi->item.mask & LVIF_PARAM)
				{
					byte m = (gb == NULL) ? 0 : gb->readMemory(plvdi->item.iItem);
					plvdi->item.lParam = m;
				}
			}
			else if(plvdi->hdr.idFrom == IDC_REG_LIST)
			{
				if(plvdi->item.mask & LVIF_TEXT)
				{
					if(plvdi->item.iItem == 0)
						regi = gb->getRegisterInfo();

					if(plvdi->item.iSubItem == 0)
						swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%s", registerNames[plvdi->item.iItem]);
					else
					{
						switch(plvdi->item.iItem)
						{
						case A:
						case B:
						case C:
						case D:
						case E:
						case F:
						case H:
						case L:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.2X", regi.reg[plvdi->item.iItem]);
							break;
						case 8:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", ((regi.reg[plvdi->item.iItem+1]) << 8) | regi.reg[plvdi->item.iItem]);
							break;
						case 9:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", regi.PC);
							break;
						case 10:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", regi.HF);
							break;
						case 11:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", regi.ZF);
							break;
						case 12:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", regi.NF);
							break;
						case 13:
							swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%.4X", regi.CF);
							break;
						}
					}
				}
			}
		}
			break;
		default:
			return false;
		}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_NEXT_INSTRUCTION:
			{
				if(!gb || !gb->cartridgeIsLoaded())
					return false;

				HWND list = GetDlgItem(hWnd, IDC_DISASSEMBLY_LIST);
				HWND regList = GetDlgItem(hWnd, IDC_REG_LIST);
				HWND stackList = GetDlgItem(hWnd, IDC_STACK_LIST);
				gb->step(1);
				ListView_RedrawItems(stackList, 0, 30);
				ListView_RedrawItems(regList, 0, 14);
				ListView_RedrawItems(list, 0, 64);
				ListView_EnsureVisible(list, 0, TRUE);
			}
			break;
		case IDCANCEL:
			EndDialog(dialogs[Disassemble], IDOK);
			break;
		}
	default:
		return false;
	}

	return true;
}

INT_PTR CALLBACK ControlsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL checked;
	switch(msg)
	{
	case WM_QUIT:
		EndDialog(controls, IDOK);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(controls, IDOK);
			return true;
		}

		if(!gb->cartridgeIsLoaded())
			return false;


		switch(HIWORD(wParam))
		{
		case BN_CLICKED:
			checked = IsDlgButtonChecked(hWnd, LOWORD(wParam));
			switch(LOWORD(wParam))
			{
			case IDC_A:
				gb->setInput(GB_A, GB_NO_CHANGE);
				break;
			case IDC_B:
				gb->setInput(GB_B, GB_NO_CHANGE);
				break;
			case IDC_START:
				gb->setInput(GB_START, GB_NO_CHANGE);
				break;
			case IDC_SELECT:
				gb->setInput(GB_SELECT, GB_NO_CHANGE);
				break;
			case IDC_UP:
				gb->setInput(GB_NO_CHANGE, GB_UP);
				break;
			case IDC_DOWN:
				gb->setInput(GB_NO_CHANGE, GB_DOWN);
				break;
			case IDC_LEFT:
				gb->setInput(GB_NO_CHANGE, GB_LEFT);
				break;
			case IDC_RIGHT:
				gb->setInput(GB_NO_CHANGE, GB_RIGHT);
				break;
			case IDC_A_HOLD:
				gb->setInputLatch((checked) ? GB_A : 0x10 + (0xf ^ GB_A), GB_NO_CHANGE);
				break;
			case IDC_B_HOLD:
				gb->setInputLatch((checked) ? GB_B : 0x10 + (0xf ^ GB_B), GB_NO_CHANGE);
				break;
			case IDC_START_HOLD:
				gb->setInputLatch((checked) ? GB_START : 0x10 + (0xf ^ GB_START), GB_NO_CHANGE);
				break;
			case IDC_SELECT_HOLD:
				gb->setInputLatch((checked) ? GB_SELECT : 0x10 + (0xf ^ GB_SELECT), GB_NO_CHANGE);
				break;
			case IDC_UP_HOLD:
				gb->setInputLatch(GB_NO_CHANGE, (checked) ? GB_UP : 0x10 + (0xf ^ GB_UP));
				break;
			case IDC_DOWN_HOLD:
				gb->setInputLatch(GB_NO_CHANGE, (checked) ? GB_DOWN : 0x10 + (0xf ^ GB_DOWN));
				break;
			case IDC_LEFT_HOLD:
				gb->setInputLatch(GB_NO_CHANGE, (checked) ? GB_LEFT : 0x10 + (0xf ^ GB_LEFT));
				break;
			case IDC_RIGHT_HOLD:
				gb->setInputLatch(GB_NO_CHANGE, (checked) ? GB_RIGHT : 0x10 + (0xf ^ GB_RIGHT));
				break;
			}
			break;
		default:
			return false;
		}
	default:
		return false;
	}
	return true;
}


INT_PTR CALLBACK MemSrchWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(dialogs[MemSearch], IDOK);
	}
	return false;
}


LRESULT CALLBACK EnhButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
							   LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		switch(dwRefData)
		{
		case IDC_REREC_REWIND:
			break;
		case IDC_REREC_FF:
			gb->setEmulationSpeed(9);
			break;
		};
		
		return TRUE;
	case WM_LBUTTONUP:
		switch(dwRefData)
		{
		case IDC_REREC_REWIND:
			break;
		case IDC_REREC_FF:
			gb->setEmulationSpeed(4);
			break;
		};

		return TRUE;

	} 
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


INT_PTR CALLBACK BreakpointListProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_TIMER:
		{
			HWND list = GetDlgItem(hWnd, IDC_BREAKPOINT_LIST);
			ListView_RedrawItems(list, 0, 1024);
			break;
		}
	case WM_INITDIALOG:
		{
			HWND list = GetDlgItem(hWnd, IDC_BREAKPOINT_LIST);

			LVCOLUMN c;

			c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
			c.cx = 80; c.cxMin = 80; c.cxIdeal = 80;
			c.pszText = L"Address";
			c.iSubItem = 0;
			c.iOrder = 0;
			ListView_InsertColumn(list, 0, &c);
		
			c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
			c.cx = 100; c.cxMin = 100; c.cxIdeal = 100;
			c.pszText = L"Condition";
			c.iSubItem = 1;
			c.iOrder = 1;
			ListView_InsertColumn(list, 1, &c);

			c.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER | LVCF_SUBITEM;
			c.cx = 100; c.cxMin = 100; c.cxIdeal = 100;
			c.pszText = L"Status";
			c.iSubItem = 2;
			c.iOrder = 2;
			ListView_InsertColumn(list, 2, &c);
			ListView_SetItemCountEx(list, 1024, NULL);
			break;
		}
	case WM_COMMAND:
		{
			HWND list = GetDlgItem(hWnd, IDC_BREAKPOINT_LIST);
			int item = -1, index = 0;

			switch(LOWORD(wParam))
			{
			case IDC_ADD_BREAKPOINT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_BREAKPOINT), hWnd, (DLGPROC)AddBreakpointProc);
				ListView_RedrawItems(list, 0, 1024);
				break;
			case IDC_REMOVE_BREAKPOINT:
				item = ListView_GetNextItem(list, -1, LVNI_SELECTED);
				while(item != -1)
				{
					wchar_t temp[10];
					wchar_t* temp2 = temp;
					ListView_GetItemText(list, item, 0, temp, 10);

					long addr = wcstol(temp, (&temp2 + 4), 16);
					gb->removeBreakpoint(addr);
					item = ListView_GetNextItem(list, ++index, LVNI_SELECTED);
					ListView_RedrawItems(list, 0, 1024);
				}
				break;
			case IDCANCEL:
				EndDialog(dialogs[BreakpointList], IDOK);
				break;
			case IDC_CLEAR:
				gb->clearBreakpoints();
				ListView_RedrawItems(list, 0, 1024);
				break;
			};
			break;
		}
	case WM_NOTIFY:
		{
			if(!gb || !gb->cartridgeIsLoaded())
				return false;

			switch (((LPNMHDR) lParam)->code)
			{
			case LVN_GETDISPINFO:
				{
					NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam; 

					if(plvdi->hdr.idFrom == IDC_BREAKPOINT_LIST)
					{
						if(plvdi->item.mask & LVIF_TEXT)
						{
							Breakpoint* bp = gb->enumerateBreakpoints(plvdi->item.iItem);
							if(bp == NULL)
								return false;

							if(plvdi->item.iSubItem == 0)
							{
								swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"%0.4X", bp->address);
							}
							else if(plvdi->item.iSubItem == 1)
							{	
								//swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, format, i.opcode, i.p1, i.p2);
							}
							else
							{
								swprintf_s(plvdi->item.pszText, plvdi->item.cchTextMax, L"Active");
							}
						}
					}
				}
				break;
			default:
				return false;
			}
			break;
		}
	case WM_QUIT:
	case WM_DESTROY:
		EndDialog(dialogs[BreakpointList], IDOK);
		break;
	default:
		return false;
	}
	return TRUE;
}


INT_PTR CALLBACK AddBreakpointProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			break;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDCANCEL:
				{
					EndDialog(hWnd, IDOK);
					break;
				}
			case IDC_ADD:
				{
					Breakpoint bp;
					wchar_t temp[10];
					wchar_t* temp2 = temp;

					GetDlgItemText(hWnd, IDC_ADDRESS, temp, 10);
					if(temp[0] != 0)
					{
						long addr = wcstol(temp, (&temp2 + 4), 16);
						if(addr <= 0xFFFF && addr >= 0)
						{
							memset(&bp, 0, sizeof(Breakpoint));
							bp.address = addr;
							bp.type = BreakpointExecute;
							gb->addBreakpoint(bp);
							EndDialog(hWnd, IDOK);
						}
					}
					break;
				}
			}
		}
	default:
		return false;
	}

	return true;
}


INT_PTR CALLBACK MemVisualizerProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BITMAP* visualization = NULL;

	switch(msg)
	{
	case WM_PAINT:
		if(gb && gb->cartridgeIsLoaded() && gb->getMemoryVisualization(&visualization))
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT r;
			GetClientRect(hWnd, &r);
			draw_to_hdc(hdc, visualization, 0, 0);
			//blit_from_hdc(hdc, visualization, 0, 0, 0, 0, visualization->w, visualization->h);  
			//stretch_blit_from_hdc(hdc, visualization, 0, 0, visualization->w, visualization->h, 0, 0, r.right - r.left, r.bottom - r.top);  
			EndPaint(hWnd, &ps);
		}
		break;
	default:
		return false;
	}

	return true;
}


INT_PTR CALLBACK MovieSetupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CANCEL:
			EndDialog(dialogs[MoviePanel], IDCANCEL);
			break;
		case IDC_OK:
			char title[256];
			char author[256];
			char file[256];

			GetDlgItemTextA(hWnd, IDC_MOVIE_TITLE, title, 256);
			GetDlgItemTextA(hWnd, IDC_AUTHOR_NAME, author, 256);
			GetDlgItemTextA(hWnd, IDC_FILE_NAME, file, 256);

			int qboyformat = Button_GetState(GetDlgItem(hWnd, IDC_FORMAT_QBOY));
			int vbaformat = Button_GetState(GetDlgItem(hWnd, IDC_FORMAT_VBM));

			int format = (qboyformat == BST_CHECKED) ? QBoy : 0;
			format += (vbaformat == BST_CHECKED) ? VBA : 0;

			if(strlen(file) > 0)
			{
				gb->pause(true);
				gb->reset();
				gb->startRecording(title, author, file, format);
				gb->pause(false);
				EndDialog(dialogs[MoviePanel], IDOK);
				return true;
			}
			break;
		}
		break;
	}

	return false;
}



void calcRewindStats(HWND hWnd)
{
	char editBuf[64];
	ZeroMemory(editBuf, 64);
	GetDlgItemTextA(hWnd, IDC_LOAD_INTERVAL, editBuf, 64);
	loadInterval = atoi(editBuf);
	ZeroMemory(editBuf, 64);
	GetDlgItemTextA(hWnd, IDC_SAVE_INTERVAL, editBuf, 64);
	saveInterval = atoi(editBuf);
	ZeroMemory(editBuf, 64);
	GetDlgItemTextA(hWnd, IDC_BUFFER_STATES, editBuf, 64);
	bufferStates = atoi(editBuf);
	
	int bufferSize = bufferStates * sizePerState;
	int bufferLength = (bufferStates * saveInterval) / 60;

	SetDlgItemInt(hWnd, IDC_BUFFER_LENGTH, bufferLength, FALSE);
	SetDlgItemInt(hWnd, IDC_BUFFER_SIZE, bufferSize, FALSE);
}


DWORD WINAPI RunGameboy(LPVOID lpParameter)
{
	gb = NULL;
	gb = new Gameboy();
	gb->rewindLoadInterval(loadInterval);
	gb->rewindSaveInterval(saveInterval);
	gb->rewindBufferSize(bufferStates);
	gb->setDisplayScale(zoom);
	gb->setSoundLevel(volume / 100);
	gb->mainLoop();
	PostQuitMessage(0);
	return NULL;
}


int MenuHeight(HWND hwnd, HMENU hmenu)
{
	RECT rect;
	RECT pmenurect;
	int  i;

	SetRect(&pmenurect, 0, 0, 0, 0);

	for(i = 0; i < GetMenuItemCount(hmenu); i++)
	{
		GetMenuItemRect(hwnd, hmenu, i, &rect);
		UnionRect(&pmenurect, &pmenurect, &rect);
	}
	return pmenurect.bottom - pmenurect.top;
}


void ClientResize(HWND hWnd, int scale)
{
	RECT rcClient, rcWindow;
	POINT ptDiff;
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	int nWidth = displayX * scale;
	int nHeight = displayY * scale;
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd,rcWindow.left, rcWindow.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}

void UpdateCartInfo(HWND hWnd)
{
	ri = gb->getRomInfo();
	wchar_t nameBuffer[16];
	ZeroMemory(nameBuffer, sizeof(wchar_t) * 16);
	for(int i = 0; i < 16; i++)
		nameBuffer[i] = ri.romName[i]; 

	SetDlgItemText(hWnd, IDC_CART_NAME, nameBuffer);
	SetDlgItemInt(hWnd, IDC_ROM_SIZE, ri.totalRomSize, true);
	SetDlgItemInt(hWnd, IDC_ROM_BANKS, ri.numRomBanks, true);
	SetDlgItemInt(hWnd, IDC_RAM_SIZE, ri.totalRamSize, true);
	SetDlgItemInt(hWnd, IDC_RAM_BANKS, ri.numRamBanks, true);
	SetDlgItemInt(hWnd, IDC_LICENSEE, ri.licensee, true);
	SetDlgItemInt(hWnd, IDC_CHECKSUM, ri.checksum, true);
	SetDlgItemInt(hWnd, IDC_REGION, ri.region, true);

	int cartTypeName = ri.cartType;
	if(cartTypeName > 0x1f) cartTypeName -= (0xfd - 0x1f); 
	SetDlgItemText(hWnd, IDC_CART_TYPE, cartridgeTypes[ri.cartType]);
}


BOOL CheckDialogs(LPMSG msg)
{
	for(int i = 0; i < DlgMax; i++)
	{
		if(dialogs[i] != NULL && IsDialogMessage(dialogs[i], msg))
			return true;
	}

	return false;
}


void initRegistry()
{
	HKEY qboyKey;
	HKEY recentKey;
	DWORD intSize = sizeof(int);
	DWORD strSize = sizeof(StrBuffer);
	DWORD type = REG_BINARY;
	
	if(RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\QBoy", NULL, NULL, NULL, KEY_READ | KEY_SET_VALUE | KEY_CREATE_SUB_KEY, NULL, &qboyKey, NULL) == ERROR_SUCCESS)
	{
		RegQueryValueEx(qboyKey, L"RewindSaveInt", NULL, NULL, (LPBYTE)&saveInterval, &intSize);
		RegQueryValueEx(qboyKey, L"RewindLoadInt", NULL, NULL, (LPBYTE)&loadInterval, &intSize);
		RegQueryValueEx(qboyKey, L"RewindBufferSize", NULL, NULL, (LPBYTE)&bufferStates, &intSize);
		RegQueryValueEx(qboyKey, L"Volume", NULL, NULL, (LPBYTE)&volume, &intSize);
		RegQueryValueEx(qboyKey, L"Zoom", NULL, NULL, (LPBYTE)&zoom, &intSize);

		RegQueryValueEx(qboyKey, L"RecentFile1", NULL, NULL, (LPBYTE)&recentFiles[0], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile2", NULL, &type, (LPBYTE)&recentFiles[1], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile3", NULL, &type, (LPBYTE)&recentFiles[2], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile4", NULL, &type, (LPBYTE)&recentFiles[3], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile5", NULL, &type, (LPBYTE)&recentFiles[4], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile6", NULL, &type, (LPBYTE)&recentFiles[5], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile7", NULL, &type, (LPBYTE)&recentFiles[6], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile8", NULL, &type, (LPBYTE)&recentFiles[7], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile9", NULL, &type, (LPBYTE)&recentFiles[8], &strSize);
		RegQueryValueEx(qboyKey, L"RecentFile10", NULL, &type, (LPBYTE)&recentFiles[9], &strSize);

		//load recent file paths
		/*if(RegCreateKeyEx(qboyKey, L"Recent", NULL, NULL, NULL, KEY_READ | KEY_SET_VALUE, NULL, &recentKey, NULL) == ERROR_SUCCESS)
		{
			int enumIndex = 0;
			StrBuffer strBuffer;
			DWORD bufferSize = sizeof(strBuffer);
		
			while(RegEnumValue(&recentKey, enumIndex, strBuffer, &bufferSize, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS && enumIndex < 10)
			{
				wcscpy_s(recentFiles[enumIndex], 256, strBuffer);
				enumIndex++;
			}
			RegCloseKey(recentKey);
		}*/

		RegCloseKey(qboyKey);
	}
}

void shutdownRegistry()
{
	HKEY qboyKey;
	DWORD intSize = sizeof(int);
	DWORD strSize = sizeof(StrBuffer);

	if(RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\QBoy", NULL, NULL, NULL, KEY_READ | KEY_SET_VALUE, NULL, &qboyKey, NULL) == ERROR_SUCCESS)
	{
		RegSetValueEx(qboyKey, L"RewindSaveInt", NULL, REG_DWORD, (BYTE*)&saveInterval, intSize);
		RegSetValueEx(qboyKey, L"RewindLoadInt", NULL, REG_DWORD, (BYTE*)&loadInterval, intSize);
		RegSetValueEx(qboyKey, L"RewindBufferSize", NULL, REG_DWORD, (BYTE*)&bufferStates, intSize);
		RegSetValueEx(qboyKey, L"Volume", NULL, REG_DWORD, (BYTE*)&volume, intSize);
		RegSetValueEx(qboyKey, L"Zoom", NULL, REG_DWORD, (BYTE*)&zoom, intSize);

		RegSetValueEx(qboyKey, L"RecentFile1", NULL, REG_BINARY, (BYTE*)&recentFiles[0], _tcslen(recentFiles[0]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile2", NULL, REG_BINARY, (BYTE*)&recentFiles[1], _tcslen(recentFiles[1]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile3", NULL, REG_BINARY, (BYTE*)&recentFiles[2], _tcslen(recentFiles[2]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile4", NULL, REG_BINARY, (BYTE*)&recentFiles[3], _tcslen(recentFiles[3]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile5", NULL, REG_BINARY, (BYTE*)&recentFiles[4], _tcslen(recentFiles[4]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile6", NULL, REG_BINARY, (BYTE*)&recentFiles[5], _tcslen(recentFiles[5]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile7", NULL, REG_BINARY, (BYTE*)&recentFiles[6], _tcslen(recentFiles[6]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile8", NULL, REG_BINARY, (BYTE*)&recentFiles[7], _tcslen(recentFiles[7]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile9", NULL, REG_BINARY, (BYTE*)&recentFiles[8], _tcslen(recentFiles[8]) * sizeof(wchar_t));
		RegSetValueEx(qboyKey, L"RecentFile10", NULL, REG_BINARY, (BYTE*)&recentFiles[9], _tcslen(recentFiles[9]) * sizeof(wchar_t));

		RegCloseKey(qboyKey);
	}
}


void updateRecentFileList(HWND hWnd)
{
	MENUITEMINFO mii;
	mii.fMask = MIIM_SUBMENU;

	MENUITEMINFO newItem;
	
	HMENU menuBar = GetMenu(hWnd);
	GetMenuItemInfo(menuBar, 3, TRUE, &mii); 
	newItem.cbSize = sizeof(MENUITEMINFO);

	newItem.fMask = MIIM_STRING;

	for(int i = 0; i < 10; i++)
	{
		newItem.dwTypeData = recentFiles[i];
		newItem.cch = _tcslen(recentFiles[i]);
		SetMenuItemInfo(menuBar, ID_LOADRECENT_11 + i, FALSE, &newItem);
	}
}
#endif