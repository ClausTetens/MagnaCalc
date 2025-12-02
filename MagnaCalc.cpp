// 1000Calc.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
#include "framework.h"
#include "MagnaCalc.h"


#define BITS_PR_BYTE 8
#define UINT32_BITS 32
#define MSB_MASK 0x80000000U
#define MANTISSE_BITS 64
#define MANTISSE_UINT32_BLOCKS (MANTISSE_BITS/UINT32_BITS)

#define MESSAGELENGTH 137
class Exception {
private:
	WCHAR message[MESSAGELENGTH];
public:
	Exception(WCHAR * message) {
		wcscpy_s(this->message, message);
	}
};

enum FloatInfo { Normalized, Zero, Denormalized, Infinite, NaN };

class RatherLongFloat {
private:
	UINT32 * mantisse;
	INT32 exponent;
	__int8 signum;
	FloatInfo meta;

	int msb(UINT32 * a) {
		return(*a & MSB_MASK) ? 1 : 0;
	}

	bool isAllZero(UINT32 * a, int length) {
		while(length--) {
			if(*a++) return false;
		}
		return true;
	}

	// return value is only correct with count<=32
	UINT32 lsh(UINT32 * a, int length, int count) {
		UINT64 acc = 0;
		if(count <= 0) return 0;
		// start of 32 bit shift
		if(count >= length * UINT32_BITS) {
			binSetZero(a, length);
			return 0; // not entirely right - ought to be the bits shifted out
		}
		int dst = 0, src = count / UINT32_BITS;
		count %= UINT32_BITS;
		if(dst < src) {
			acc = a[src - 1];
			while(src < length) {
				a[dst++] = a[src++];
			}
			while(dst < length) {
				a[dst++] = 0;
			}
		}
		if(count == 0) return (UINT32)acc;
		// end of 32 bit shift
		acc = 0;
		a += length - 1;
		while(length--) {
			acc |= ((UINT64)*a) << count;
			*a = (UINT32)acc;
			acc >>= UINT32_BITS;
			a--;
		}
		return (UINT32)acc;
	}

	bool normalize(UINT32 * a, int length, int & expo, FloatInfo & meta) {
		if(isAllZero(a, length)) {
			meta = Zero;
			return true;
		}
		// lsh is able to cope with this - just make cnt reflect the amount of UINT32_BITS units
		while(*a == 0) {
			for(int i = 0; i < length - 1; i++) {
				a[i] = a[i + 1];
			}
			a[length - 1] = 0;
			expo -= UINT32_BITS;
		}
		UINT32 mask = MSB_MASK, msw = *a;
		int cnt = 0;
		while(!(msw&mask)) {
			mask >>= 1;
			cnt++;
		}
		lsh(a, length, cnt);
		expo -= cnt;
	}

	void divInt(UINT32 * quotient, int quotientLength, UINT32 * remainder, int remainderLength, UINT32 * a, int aLength, UINT32 * b, int bLength) {
		int tempLength = quotientLength + remainderLength;
		UINT32 * temp = new UINT32[tempLength];
		binSetZero(temp, tempLength);

	}

	void mulIntII(UINT32* result, int resultLength, UINT32* a, int aLength, UINT32* b, int bLength) {
		if(resultLength < aLength + bLength || aLength < 1 || bLength < 1) {
			WCHAR msg[MESSAGELENGTH];
			wsprintfW(msg, L"mulInt length error: %d < %d + %d or no input", resultLength, aLength, bLength);
			throw new Exception(msg);
		}

		binSetZero(result, resultLength);
		UINT64 acc;

	}


	void mulInt(UINT32 * result, int resultLength, UINT32 * a, int aLength, UINT32 * b, int bLength) {
		if(resultLength < aLength + bLength || aLength < 1 || bLength < 1) {
			WCHAR msg[MESSAGELENGTH];
			wsprintfW(msg, L"mulInt length error: %d < %d + %d or no input", resultLength, aLength, bLength);
			throw new Exception(msg);
		}

		binSetZero(result, resultLength);
		UINT64 acc;
		UINT32 * temp = new UINT32[resultLength];
		int ia, ib = bLength - 1, ix, ixInit = resultLength - 1;
		while(ib >= 0) {
			binSetZero(temp, resultLength);
			ia = aLength - 1;
			ix = ixInit--;
			while(ia >= 0) {
				acc = UInt32x32To64(a[ia], b[ib]);
				ia -= 2;
				temp[ix--] = (UINT32)acc;
				temp[ix--] = (UINT32)(acc >> UINT32_BITS);
			}
			addInt(result, result, temp, resultLength);

			binSetZero(temp, resultLength);
			ia = aLength - 2;
			ix = ixInit;
			while(ia >= 0) {
				acc = UInt32x32To64(a[ia], b[ib]);
				ia -= 2;
				temp[ix--] = acc;
				temp[ix--] = acc >> UINT32_BITS;
			}
			addInt(result, result, temp, resultLength);

			ib--;
		}
		delete[] temp;
	}

	UINT32 addInt(UINT32 * result, UINT32 * a, UINT32 * b, int length) {
		UINT64 acc = 0;
		for(int i = length - 1; i >= 0; i--) {
			acc += ((UINT64)a[i]) + ((UINT64)b[i]);
			result[i] = (UINT32)acc;
			acc >>= UINT32_BITS;
		}
		return (UINT32)acc;
	}

	UINT32 subInt(UINT32 * result, UINT32 * a, UINT32 * b, int length) {
		UINT64 acc = 0;
		for(int i = length - 1; i >= 0; i--) {
			acc = ((UINT64)a[i]) - (((UINT64)b[i]) + acc);
			result[i] = (UINT32)acc;
			acc = (acc >> UINT32_BITS) ? 1 : 0;
		}
		return (UINT32)acc;
	}

	void decSetZero(BYTE * dec, int decLength) {
		while(decLength--) *(dec++) = 0;
	}

	UINT32 decMulTwo(BYTE * dec, int decLength) {
		dec += decLength - 1;
		BYTE acc = 0;
		while(decLength--) {
			acc = ((*dec) << 1) + acc;
			*dec = acc % 10;
			acc /= 10;
			dec--;
		}
		return acc;
	}

	// val <= 2^32-10
	UINT32 decAddUint32(BYTE * dec, int decLength, UINT32 val) {
		dec += decLength - 1;
		while(decLength-- && val > 0) {
			val += *dec;
			*dec = val % 10;
			dec--;
			val /= 10;
		}
		return val;
	}

	void binIntToDec(BYTE * dec, int decLength, UINT32 * bin, int binLength) {
		decSetZero(dec, decLength);
		for(int i = 0; i < binLength; i++) {
			UINT32 acc = bin[i];
			for(int j = 0; j < sizeof(UINT32)*BITS_PR_BYTE; j++) {
				decMulTwo(dec, decLength);
				decAddUint32(dec, decLength, acc >> (UINT32_BITS - 1));
				acc <<= 1;
			}
		}
	}

	UINT32 binAddUint32(UINT32 * bin, int binLength, UINT32 val) {
		UINT64 acc = val;
		bin += binLength - 1;
		while(binLength--) {
			acc += *bin;
			*bin = (UINT32)acc;
			acc >>= UINT32_BITS;
			bin--;
		}
		return (UINT32)acc;
	}

	void binSetZero(UINT32 * bin, int binLength) {
		while(binLength--) *(bin++) = 0;
	}

	UINT32 binMulTen(UINT32 * bin, int binLength) {
		UINT64 acc = 0, ten = 10;
		bin += binLength - 1;
		while(binLength--) {
			acc += *bin * ten;
			*bin = (UINT32)acc;
			acc >>= UINT32_BITS;
			bin--;
		}
		return (UINT32)acc;
	}

	void decIntToBin(BYTE * dec, int decLength, UINT32 * bin, int binLength) {
		binSetZero(bin, binLength);
		while(decLength--) {
			binMulTen(bin, binLength);
			binAddUint32(bin, binLength, ((*dec++) & 0x0F) % 10);
		}
	}

	void reciproc() {

	}
	void calcPiFourths() {
		//https://www.ndl.go.jp/math/e/s1/c4_2.html
		//https://members.loria.fr/PZimmermann/talks/gauss.pdf
		// == 12 * arctan(1/18) + 8 * arctan(1/57) - 5 * arctan(1/239)
	}

	/*
	void decToAscii(BYTE * dec, int decLength) {
		while(decLength--) {
			*dec += '0';
			dec++;
		}
	}*/

	void decPrint(BYTE * dec, int decLength, HDC hdc, int lin = 0) {
		WCHAR * wbuf = new WCHAR[decLength];
		for(int i = 0; i < decLength; i++) wbuf[i] = dec[i] + '0';
		TextOut(hdc, 10, 20 * lin, wbuf, decLength);
		delete[]wbuf;
	}

public:

	RatherLongFloat() {
		mantisse = new UINT32[MANTISSE_UINT32_BLOCKS];
		exponent = 0;
		signum = 0;
		meta = Zero;
	}

	~RatherLongFloat() {
		delete[] mantisse;
	}

	void test(HDC hdc) {
		int lin = 0;
		COLORREF colour = 0xc0ffc0;
		SetBkColor(hdc, colour);

		UINT32 yff = MSB_MASK;

		UINT32 c[2], a[2], b[2], r[4];
		UINT32 val = 0xFFffFFff;
		//a[0] = val;
		a[0] = 478888;
		//a[1] = val;
		a[1] = 5211;
		//b[0] = val;
		b[0] = 654;
		//b[1] = val-1;
		b[1] = 987;

		mulInt(r, 4, a, 2, b, 2);
		/*
		for(int i = 0; i < 8; i++) {
			mulInt(r, 4, a, 2, b, 2);
			a[0] = r[2]; a[1] = r[3];
		}*/

		/*		yff = lsh(a, 2, 28);
				yff = lsh(a, 2, 4);
				yff = lsh(a, 2, 4);
				yff = lsh(a, 2, 2);*/

		int x = sizeof(UINT32)*BITS_PR_BYTE;

		int binLength = 2;
		int decLength = 40;
		BYTE *dec = new BYTE[decLength];

		binIntToDec(dec, decLength, a, binLength);
		decPrint(dec, decLength, hdc, 0);

		binIntToDec(dec, decLength, b, binLength);
		decPrint(dec, decLength, hdc, 1);

		binIntToDec(dec, decLength, r, 4);
		decPrint(dec, decLength, hdc, 2);


		/*
		UINT32 cy=subInt(c, a, b, binLength);

		binIntToDec(dec, decLength, c, binLength);
		//decToAscii(dec, decLength);
		decPrint(dec, decLength, hdc);

		WCHAR wcy=cy + '0';

		TextOut(hdc, 0, 20*lin, &wcy, 1);

		for(int i = 0; i < 5; i++) {
			binMulTen(c, binLength);
			binIntToDec(dec, decLength, c, binLength);
			decPrint(dec, decLength, hdc, i+1);

		}


		decSetZero(dec, decLength);
		for(int i = 0; i < decLength; i++) dec[i] = '0';
		for(int i = 0; i < 15; i++) {
			dec[decLength - 1 - i] = (i%10)+'0';
		}

		decIntToBin(dec, decLength, c, binLength);
		binIntToDec(dec, decLength, c, binLength);
		decPrint(dec, decLength, hdc, 7);

		//addInt(c, a, b, binLength);

		*/

		delete[]dec;
	}
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.


	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MAGNACALC, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if(!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAGNACALC));

	MSG msg;

	// Main message loop:
	while(GetMessage(&msg, nullptr, 0, 0)) {
		if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAGNACALC));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MAGNACALC);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if(!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


#include <winsock2.h>
#include <ws2tcpip.h>

void listen() {
	//https://learn.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-server
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch(wmId) {
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...


			RatherLongFloat * ratherLongFloat = new RatherLongFloat();
			ratherLongFloat->test(hdc);
			delete ratherLongFloat;



			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch(message) {
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
