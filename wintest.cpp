// wintest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "wintest.h"
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
DWORD gbColors[4];

int saveInterval = 100;
int bufferStates = 50;
int loadInterval = 100;
const int maxStates = 500;
const int minStates = 1;
const int sizePerState = 16384;
const int minLoadInt = 10;
const int maxLoadInt = 10000;
const int minSaveInt = 10;
const int maxSaveInt = 10000;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINTEST));


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_OPEN_ROM:
			if(GetOpenFileName(&fn) == TRUE)
			{
				//load rom
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_EMULATION_REWINDBUFFER:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REWIND_DIALOG), hWnd, RewindBufferProc);
			break;
		case ID_DISPLAY_SETCOLORS:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_COLORS), hWnd, ColorsProc);
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
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		MessageBox(NULL, L"Clicked", L"Derp", MB_OK);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
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
		ZeroMemory(&cc, sizeof(cc));
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = hWnd;
		cc.lpCustColors = (LPDWORD) cr;
		cc.rgbResult = 0;
		cc.Flags = CC_RGBINIT;
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_COLOR1:
			cc.rgbResult = gbColors[0];
			if(ChooseColor(&cc) == TRUE)
				gbColors[0] = cc.rgbResult;
			break;
		case IDC_COLOR2:
			cc.rgbResult = gbColors[1];
			if(ChooseColor(&cc) == TRUE)
				gbColors[1] = cc.rgbResult;
			break;
		case IDC_COLOR3:
			cc.rgbResult = gbColors[2];
			if(ChooseColor(&cc) == TRUE)
				gbColors[2] = cc.rgbResult;
			break;
		case IDC_COLOR4:
			cc.rgbResult = gbColors[3];
			if(ChooseColor(&cc) == TRUE)
				gbColors[3] = cc.rgbResult;
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
			long x = MAKELONG((short)maxStates, (short)minStates);
			SendDlgItemMessage(hWnd, IDC_STATES_SPIN, UDM_SETRANGE, 0, MAKELONG((short)maxStates, (short)minStates));
			SendDlgItemMessage(hWnd, IDC_LOAD_SPIN, UDM_SETRANGE, 0, MAKELONG((short)maxLoadInt, (short)minLoadInt));
			SendDlgItemMessage(hWnd, IDC_SAVE_SPIN, UDM_SETRANGE, 0, MAKELONG((short)maxSaveInt, (short)minSaveInt));
			SetDlgItemInt(hWnd, IDC_BUFFER_STATES, bufferStates, FALSE);
			SetDlgItemInt(hWnd, IDC_LOAD_INTERVAL, loadInterval, FALSE);
			SetDlgItemInt(hWnd, IDC_SAVE_INTERVAL, saveInterval, FALSE);
			BOOL b;
			SetDlgItemText(hWnd, IDC_SIZE_PER_STATE, L"16384");
			SetDlgItemInt(hWnd, IDC_BUFFER_STATES, bufferStates, FALSE);
			_itow(bufferStates * sizePerState, totalSize, 10);
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
				int numStates = GetDlgItemInt(hWnd, IDC_STATES_SPIN, &b, FALSE);
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
	int bufferLength = (bufferStates * saveInterval) / 1000;

	SetDlgItemInt(hWnd, IDC_BUFFER_LENGTH, bufferLength, FALSE);
	SetDlgItemInt(hWnd, IDC_BUFFER_SIZE, bufferSize, FALSE);
}