// emGUI-example.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "emGUI-example.h"

#include <cstdio>
#include <iostream>

#include <deque>

#include <io.h>

#include <fcntl.h>
#include <process.h>
#define SERIAL_READ_TIMEOUT 500 
#define SERIAL_READ_BUF_SIZE 10

#define TIMEOUT 1000



using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define MAX_LOADSTRING 100

#ifndef DEBUG_ZONE
#define DEBUG_ZONE 0;
#endif

HANDLE serialPort;
HANDLE serialThread;
unsigned int  dwThreadId;
// ���������� ����������:
HINSTANCE hInst;                                // ������� ���������
WCHAR szTitle[MAX_LOADSTRING];                  // ����� ������ ���������
WCHAR szWindowClass[MAX_LOADSTRING];            // ��� ������ �������� ����
HWND hWnd;
UINT_PTR uTimerId;

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	

	try
	{
		// Create basic file logger (not rotated)
		logger = spdlog::basic_logger_mt("basic_logger", "app.log");
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log initialization failed: " << ex.what() << std::endl;
	}
	
	logger->info("Hello!");

	serialThreadParams_t threadParams;

	threadParams.portName = L"COM9";
	threadParams.exitFlag = false;
	threadParams.logger = logger;
	threadParams.data = pxGUIGetPlotData();

	// TODO: ���������� ��� �����.
	serialThread = (HANDLE)_beginthreadex(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		&SecondThreadFunc,       // thread function name
		&threadParams,                   // argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);   // returns the thread identifier 
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bGUI_InitInterfce(); //TODO: check false

	// ������������� ���������� �����
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EMGUIEXAMPLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ��������� ������������� ����������:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMGUIEXAMPLE));

	MSG msg;

	// ���� ��������� ���������:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	logger->info("Leaving main thread");
	threadParams.exitFlag = true;
	WaitForSingleObject(serialThread, INFINITE);
	logger->info("Serial thread should be dead!");
	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}

void HandleASuccessfulRead(char * lpBuf, WORD dwRead, serialThreadParams_t * params) {
	static std::deque<char> buf;

	for (int i = 0; i < dwRead; i++)
		buf.push_back(lpBuf[i]);

	static std::string line;

	while (buf.size() > 0) {
		char sym = buf.front();
		buf.pop_front();
		if (sym == '\n' || sym == '\r') {
			if (line.length() > 0) {
				try {
					int data = std::stoi(line);
					handleData(data, params);
				}
				catch (std::invalid_argument& e) {
					logger->error("Invalid string parsing");
				}
				line = "";
			}
			continue;
		}
		line += sym;
	}

}

void handleData(int data, serialThreadParams_t * params) {
	auto pd = params->data;

	logger->info(data);
	pd->psData[pd->ulWritePos] = data;
	pd->ulWritePos++;
	if (pd->ulWritePos >= pd->ulElemCount) {
		pd->ulWritePos = 0;
	}
}

//
//  �������: MyRegisterClass()
//
//  ����������: ������������ ����� ����.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMGUIEXAMPLE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_EMGUIEXAMPLE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   �������: InitInstance(HINSTANCE, int)
//
//   ����������: ��������� ��������� ���������� � ������� ������� ����.
//
//   �����������:
//
//        � ������ ������� ���������� ���������� ����������� � ���������� ����������, � �����
//        ��������� � ��������� �� ����� ������� ���� ���������.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // ��������� ���������� ���������� � ���������� ������
	DWORD dwStyle = WS_BORDER;
	HWND hWnd = CreateWindowW(szWindowClass, 0, dwStyle,
		CW_USEDEFAULT, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, nullptr, hInstance, nullptr);
	SetWindowLong(hWnd, GWL_STYLE, WS_BORDER);     // stupid winapi maigic. You should reset all styles and set style again
	RECT rc;

	GetWindowRect(hWnd, &rc);

	int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

	SetWindowPos(hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	if (!hWnd)
	{
		return FALSE;
	}
	uTimerId = SetTimer(hWnd, 1, 200, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

unsigned __stdcall SecondThreadFunc(void* pArguments) {
	auto prm = (serialThreadParams_t *)pArguments;
	if (!prm)
		return 1;

	auto logger = prm->logger;
	serialPort = CreateFile(prm->portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		NULL);
	if (serialPort == INVALID_HANDLE_VALUE) {
		return 2;
	}

	// Serial port settings
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(serialPort, &dcbSerialParams))
	{
		logger->error("getting state error");
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(serialPort, &dcbSerialParams))
	{
		logger->error("error setting serial port state");
	}


	static DWORD dwRead;
	static BOOL fWaitingOnRead = FALSE;
	static OVERLAPPED osReader = { 0 };
	static DWORD dwRes;
	static char lpBuf[10];
	logger->info("Starting serial read thread from {}", (char *) prm->portName);

	// Issue read operation.
	while (ReadFile(serialPort, lpBuf, 1, &dwRead, NULL) && !prm->exitFlag) {
		// read completed immediately
		//logger->info("Read 1 byte");
		HandleASuccessfulRead(lpBuf, dwRead, prm);
	}

	logger->info("Leaving serial read thread");
	CloseHandle(serialPort);
	serialPort = INVALID_HANDLE_VALUE;
	;// throw TTYException();
	return 0;
}
//
//  �������: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����������:  ������������ ��������� � ������� ����.
//
//  WM_COMMAND � ���������� ���� ����������
//  WM_PAINT � ���������� ������� ����
//  WM_DESTROY � ��������� ��������� � ������ � ���������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		/*case WM_COMMAND:
			//{
			  //  int wmId = LOWORD(wParam);
				//DestroyWindow(hWnd);
			}
			break;*/
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc_tmp = BeginPaint(hWnd, &ps);
		Graphics gr_tmp(hdc_tmp);
		vGUIsetCurrentHDC(&gr_tmp);
		vGUIpaintEventHandler();
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		KillTimer(hWnd, uTimerId);
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		vGUIpushClickHandler(lParam);
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_LBUTTONUP:
		vGUIpopClickHandler(lParam);
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_ERASEBKGND:
		vGUIeraseBackgroudHandler();
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

