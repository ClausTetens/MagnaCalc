// 1000Calc.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"
#include "framework.h"
#include "MagnaCalc.h"

#include <Windows.h>
#include "winnt.h"

#define BITS_PR_BYTE 8
#define UINT32_BITS 32
#define MSB_MASK 0x80000000U
#define MANTISSE_BITS 64
#define MANTISSE_UINT32_BLOCKS (MANTISSE_BITS/UINT32_BITS)
#define LONGMANTISSE_UINT32_BLOCKS (2*MANTISSE_UINT32_BLOCKS)


enum FloatInfo { FP_OK, FP_DENORMALIZED, FP_INFINITE, FP_NAN };

#define MESSAGELENGTH 137
class Exception {
private:
	WCHAR message[MESSAGELENGTH];
public:
	Exception(WCHAR* message) {
		wcscpy_s(this->message, message);
	}
};

class LongMantisse {
#define NUMBER_OF_LONGMANTISSES 16
private:
	UINT32* mantisse[NUMBER_OF_LONGMANTISSES];
	bool inUse[NUMBER_OF_LONGMANTISSES];
public:
	LongMantisse() {
		for (int i = 0; i < NUMBER_OF_LONGMANTISSES; i++) {
			//mantisse[i] = new UINT32[LONGMANTISSE_UINT32_BLOCKS];
			mantisse[i] = nullptr;
			inUse[i] = false;
		}
	}

	~LongMantisse() {
		for (int i = 0; i < NUMBER_OF_LONGMANTISSES; i++) {
			if (mantisse[i]) delete[] mantisse[i];
		}
	}

	UINT32* getMantisse() {
		for (int i = 0; i < NUMBER_OF_LONGMANTISSES; i++) {
			if (!inUse[i]) {
				inUse[i] = true;
				if (!mantisse[i]) {
					mantisse[i] = new UINT32[LONGMANTISSE_UINT32_BLOCKS];
				}
				return mantisse[i];
			}
		}
		WCHAR msg[MESSAGELENGTH] = L"LongMantisse::getMantisse no free mantisse available";
		throw new Exception(msg);
		return nullptr;
	}

	void releaseMantisse(UINT32* m) {
		for (int i = 0; i < NUMBER_OF_LONGMANTISSES; i++) {
			if (mantisse[i] == m) {
				inUse[i] = false;
				return;
			}
		}
		//throw new Exception(L"LongMantisse::releaseMantisse mantisse not found");
	}

	/*
	UINT32* getMantisse(int index) {
		if(index < 0 || index >= NUMBER_OF_LONGMANTISSES) {
			WCHAR msg[MESSAGELENGTH];
			wsprintfW(msg, L"LongMantisse::getMantisse index out of range: %d", index);
			throw new Exception(msg);
		}
		if(!mantisse[index]) {
			mantisse[index] = new UINT32[LONGMANTISSE_UINT32_BLOCKS];
		}
		return mantisse[index];
	}*/
};


LongMantisse longMantissePool;




class RatherLongFloat {
private:
	UINT32* mantisse;
	INT32 exponent;
	signed __int8 signum;
	FloatInfo meta;

	int getMsb(UINT32* a) {
		return(*a & MSB_MASK) ? 1 : 0;
	}

	void setMsb(UINT32* a) {
		*a |= MSB_MASK;
	}

	void resetMsb(UINT32* a) {
		*a &= ~MSB_MASK;
	}

	bool isAllZero(UINT32* a, int length) {
		while (length--) {
			if (*a++) return false;
		}
		return true;
	}

	// return value is only correct with count<=32
	UINT32 lsh(UINT32* a, int length, int count) {
		UINT64 acc = 0;
		if (count <= 0) return 0;
		// start of 32 bit shift
		if (count >= length * UINT32_BITS) {
			binSetZero(a, length);
			return 0; // not entirely right - ought to be the bits shifted out
		}
		int dst = 0, src = count / UINT32_BITS;
		count %= UINT32_BITS;
		if (dst < src) {
			acc = a[src - 1];
			while (src < length) {
				a[dst++] = a[src++];
			}
			while (dst < length) {
				a[dst++] = 0;
			}
		}
		if (count == 0) return (UINT32)acc;
		// end of 32 bit shift
		acc = 0;
		a += length - 1;
		while (length--) {
			acc |= ((UINT64)*a) << count;
			*a = (UINT32)acc;
			acc >>= UINT32_BITS;
			a--;
		}
		return (UINT32)acc;
	}

	void normalize(UINT32* a, int length, int& expo, FloatInfo& meta) {
		if (isAllZero(a, length)) {
			signum = 0;
			//return true;
			return;
		}
		// lsh is able to cope with this - just make cnt reflect the amount of UINT32_BITS units
		while (*a == 0) {
			for (int i = 0; i < length - 1; i++) {
				a[i] = a[i + 1];
			}
			a[length - 1] = 0;
			expo -= UINT32_BITS;
		}
		UINT32 mask = MSB_MASK, msw = *a;
		int cnt = 0;
		while (!(msw & mask)) {
			mask >>= 1;
			cnt++;
		}
		lsh(a, length, cnt);
		expo -= cnt;

		//return true/false/...?;
	}



	void round(UINT32* a, int& expo, FloatInfo& meta, UINT32 nextA) {
		round(a, MANTISSE_UINT32_BLOCKS, expo, meta, nextA);
	}

	// number must be normalized when entering this function - MSB in nextA is the first bit to be rounded away
	void round(UINT32* a, int length, int& expo, FloatInfo& meta, UINT32 nextA) {
		if (getMsb(&nextA)) {
			// need to add one to a
			UINT32 carry = 1;
			for (int i = length - 1; i >= 0 && carry; i--) {
				UINT64 acc = (UINT64)a[i] + (UINT64)carry;
				a[i] = (UINT32)acc;
				carry = (UINT32)(acc >> UINT32_BITS);
			}
			if (carry) {
				/*
				// overflowed - need to rsh
				UINT32 temp = rsh(a, length, 1);
				expo += 1;
				if(temp != 0) {
					// should not happen - means we lost bits
					meta = FP_INFINITE;
				} */
				// if overflowed by adding 1, then all bits were 1 - now all bits must be 0 except MSB
				//binSetZero(a, length); - bits are already zeroed
				expo += 1;
				setMsb(a);
			}
		}
	}


	void divInt(UINT32* quotient, int quotientLength, UINT32* remainder, int remainderLength, UINT32* a, int aLength, UINT32* b, int bLength) {
		int tempLength = quotientLength + remainderLength;
		UINT32* temp = new UINT32[tempLength];
		binSetZero(temp, tempLength);

	}

	void mulIntII(UINT32* result, int resultLength, UINT32* a, int aLength, UINT32* b, int bLength) {
		if (resultLength < aLength + bLength || aLength < 1 || bLength < 1) {
			WCHAR msg[MESSAGELENGTH];
			wsprintfW(msg, L"mulInt length error: %d < %d + %d or no input", resultLength, aLength, bLength);
			throw new Exception(msg);
		}

		binSetZero(result, resultLength);
		UINT64 acc;

	}


	void copyMantisse(UINT32* dest, UINT32* src) {
		for (int i = 0; i < MANTISSE_UINT32_BLOCKS; i++) {
			dest[i] = src[i];
		}
	}


	//	this = a * b
	void mulFp(RatherLongFloat* a, RatherLongFloat* b) {
		// ToDo special cases not handled yet (like zero, infinite, nan)
		UINT32* result = longMantissePool.getMantisse();
		mulInt(result, LONGMANTISSE_UINT32_BLOCKS, a->mantisse, MANTISSE_UINT32_BLOCKS, b->mantisse, MANTISSE_UINT32_BLOCKS); //her går det vist galt
		this->exponent = a->exponent + b->exponent;
		normalize(result, LONGMANTISSE_UINT32_BLOCKS, this->exponent, this->meta);
		round(result, this->exponent, this->meta, result[MANTISSE_UINT32_BLOCKS]);
		copyMantisse(this->mantisse, result);
		longMantissePool.releaseMantisse(result);
	}



	void mulInt(UINT32* result, UINT32* a, UINT32* b) {
		mulInt(result, LONGMANTISSE_UINT32_BLOCKS, a, MANTISSE_UINT32_BLOCKS, b, MANTISSE_UINT32_BLOCKS);
	}


	void mulInt(UINT32* result, int resultLength, UINT32* a, int aLength, UINT32* b, int bLength) {
		if (resultLength < aLength + bLength || aLength < 1 || bLength < 1) {
			WCHAR msg[MESSAGELENGTH];
			wsprintfW(msg, L"mulInt length error: %d < %d + %d or no input", resultLength, aLength, bLength);
			throw new Exception(msg);
		}

		binSetZero(result, resultLength);
		UINT64 acc;
		//UINT32* temp = new UINT32[resultLength];
		UINT32* temp = longMantissePool.getMantisse();
		int ia, ib = bLength - 1, ix, ixInit = resultLength - 1;
		while (ib >= 0) {
			binSetZero(temp, resultLength);
			ia = aLength - 1;
			ix = ixInit--;
			while (ia >= 0) {
				acc = UInt32x32To64(a[ia], b[ib]);
				ia -= 2;
				temp[ix--] = (UINT32)acc;
				temp[ix--] = (UINT32)(acc >> UINT32_BITS);
			}
			addInt(result, result, temp, resultLength);

			binSetZero(temp, resultLength);
			ia = aLength - 2;
			ix = ixInit;
			while (ia >= 0) {
				acc = UInt32x32To64(a[ia], b[ib]);
				ia -= 2;
				temp[ix--] = (UINT32)acc;
				temp[ix--] = acc >> UINT32_BITS;
			}
			addInt(result, result, temp, resultLength);

			ib--;
		}
		longMantissePool.releaseMantisse(temp);
		//delete[] temp;
	}

	UINT32 addInt(UINT32* result, UINT32* a, UINT32* b, int length) {
		UINT64 acc = 0;
		for (int i = length - 1; i >= 0; i--) {
			acc += ((UINT64)a[i]) + ((UINT64)b[i]);
			result[i] = (UINT32)acc;
			acc >>= UINT32_BITS;
		}
		return (UINT32)acc;
	}

	UINT32 subInt(UINT32* result, UINT32* a, UINT32* b, int length) {
		UINT64 acc = 0;
		for (int i = length - 1; i >= 0; i--) {
			acc = ((UINT64)a[i]) - (((UINT64)b[i]) + acc);
			result[i] = (UINT32)acc;
			acc = (acc >> UINT32_BITS) ? 1 : 0;
		}
		return (UINT32)acc;
	}

	void decSetZero(BYTE* dec, int decLength) {
		while (decLength--) *(dec++) = 0;
	}

	UINT32 decMulTwo(BYTE* dec, int decLength) {
		dec += decLength - 1;
		BYTE acc = 0;
		while (decLength--) {
			acc = ((*dec) << 1) + acc;
			*dec = acc % 10;
			acc /= 10;
			dec--;
		}
		return acc;
	}

	// decimal shift left by one digit
	UINT32 decMulTen(BYTE* dec, int decLength) {
		UINT32 acc = dec[0]	;
		for (int i = 0; i < decLength-1; i++) {
			dec[i] =dec[i+1];
		}
		dec[decLength - 1] = 0;
		return acc;
	}

	// val <= 2^32-10
	UINT32 decAddUint32(BYTE* dec, int decLength, UINT32 val) {
		dec += decLength - 1;
		while (decLength-- && val > 0) {
			val += *dec;
			*dec = val % 10;
			val /= 10;
			dec--;
		}
		return val;
	}

	void binIntToDec(BYTE* dec, int decLength, UINT32* bin, int binLength) {
		decSetZero(dec, decLength);
		for (int i = 0; i < binLength; i++) {
			UINT32 acc = bin[i];
			//for (int j = 0; j < sizeof(UINT32) * BITS_PR_BYTE; j++) {
			for (int j = 0; j < UINT32_BITS; j++) {
				decMulTwo(dec, decLength);
				//decAddUint32(dec, decLength, acc >> (UINT32_BITS - 1));
				//decAddUint32(dec, decLength, (acc & MSB_MASK) ? 1 : 0);
				if(acc & MSB_MASK) decAddUint32(dec, decLength, 1);
				acc <<= 1;
			}
		}
	}

	/* more like dec frac to bin frac
	void binFracToDec(BYTE* dec, int decLength, UINT32* bin, int binLength, int fracBits) {
		decSetZero(dec, decLength);
		for (int i = 0; i < fracBits; i++) {
			binMulTen(dec, decLength);
			// get bit i
			int bitIndex = i;
			int binIndex = bitIndex / UINT32_BITS;
			int bitInUint32 = bitIndex % UINT32_BITS;
			UINT32 bit = (binIndex < binLength && (bin[binIndex] & (MSB_MASK >> bitInUint32))) ? 1 : 0;
			decAddUint32(dec, decLength, bit);
		}
	}*/


	void binFracToDec(BYTE* dec, int decLength, UINT32* bin, int binLength) {
		UINT32 acc;
		decSetZero(dec, decLength);
		for (int i = 0; i < decLength; i++) {
			acc = binMulTen(bin, binLength);
			decMulTen(dec, decLength);
			decAddUint32(dec, decLength, acc);	
		}
	}

	/*
		dec frac to bin frac:
		multiply by 2
		add carry to bin
		continue for fracBits
		add next bit to round off
	*/

	UINT32 binAddUint32(UINT32* bin, int binLength, UINT32 val) {
		UINT64 acc = val;
		bin += binLength - 1;
		while (binLength--) {
			acc += *bin;
			*bin = (UINT32)acc;
			acc >>= UINT32_BITS;
			bin--;
		}
		return (UINT32)acc;
	}

	void binSetZero(UINT32* bin, int binLength) {
		while (binLength--) *(bin++) = 0;
	}

	UINT32 binMulTen(UINT32* bin, int binLength) {
		UINT64 acc = 0, ten = 10;
		bin += binLength - 1;
		while (binLength--) {
			acc += *bin * ten;
			*bin = (UINT32)acc;
			acc >>= UINT32_BITS;
			bin--;
		}
		return (UINT32)acc;
	}

	void decIntToBin(BYTE* dec, int decLength, UINT32* bin, int binLength) {
		binSetZero(bin, binLength);
		while (decLength--) {
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


	/*	sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ...
		cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + ...
		tan(x) = sin(x)/cos(x)
		atan(x) = x - x^3/3 + x^5/5 - x^7/7 + ... | for |x| <= 1
		exp(x) = 1 + x/1! + x^2/2! + x^3/3! + ...
		ln(1+x) = x - x^2/2 + x^3/3 - x^4/4 + ...

		//sin(x) = -sin(-x) = -sin(x-PI) = sin(x + 2kPI) = sin(x mod 2PI) = sin(x mod PI)
		sin(x) = -sin(-x) = sin(x mod 2PI) = -sin(x-PI) = sin(PI - x) | apply first for negative x; second for x > 2PI; third for x ]PI, 2PI]; fourth for x > PI/2
		cos(x) = sin(x + PI/2)


		sin(x) = -sin(x-pi) = sin(pi-x) = cos(pi/2-x)
				x>pi        x>pi/2      x>pi/4
		Reducér x til [0;pi/4] og beregn Taylor sin hhv cos

		cos(a-b) = cos(a)cos(b) + sin(a)sin(b); cos(PI/2) = 0; sin(PI/2) = 1; ==> cos(PI/2 - x) = sin(x)

	*/

	/*
	void decToAscii(BYTE * dec, int decLength) {
		while(decLength--) {
			*dec += '0';
			dec++;
		}
	}*/

	void decPrint(BYTE* dec, int decLength, HDC hdc, int lin = 0) {
		WCHAR* wbuf = new WCHAR[decLength];
		for (int i = 0; i < decLength; i++) wbuf[i] = dec[i] + '0';
		TextOut(hdc, 10, 20 * lin, wbuf, decLength);
		delete[]wbuf;
	}

	void printHexFp(HDC hdc, int lin = 0) {
		WCHAR wbuf[MESSAGELENGTH];
		int len = wsprintfW(wbuf, L"signum: %d, exponent: %d, mantisse: ", signum, exponent);
		for (int i = 0; i < MANTISSE_UINT32_BLOCKS; i++) {
			len += wsprintfW(wbuf + len, L"%08X ", mantisse[i]);
		}
		TextOut(hdc, 300, 20 * lin, wbuf, len);
	}


	void setToZeroFp() {
		binSetZero(mantisse, MANTISSE_UINT32_BLOCKS);
		exponent = 0;
		signum = 0;
	}

	void zeroFp() {
		signum = 0;
	}
	
	void absFp() {
		if (signum < 0)	signum = 1;
	}
	
	void changeSignFp() {
		if (signum > 0) signum = -1;
		else if (signum < 0) signum = 1;
	}

	void positiveFp() {
		signum = 1;
	}

	void negativeFp() {
		signum = -1;
	}

	void fromIntegerFp(INT32 val) {
		binSetZero(mantisse, MANTISSE_UINT32_BLOCKS);
		if (val < 0) {
			signum = -1;
			val = -val;
		}
		else if (val > 0) {
			signum = 1;
		}
		else {
			signum = 0;
		}

		if (signum == 0) {
			exponent = 0;
		}
		else {
			mantisse[0] = (UINT32)val;
			exponent = UINT32_BITS;
			normalize(mantisse, MANTISSE_UINT32_BLOCKS, exponent, meta);
		}
	}

	void fromUint32Fp(UINT32 val) {
		binSetZero(mantisse, MANTISSE_UINT32_BLOCKS);
		if (val > 0) {
			signum = 1;
			mantisse[0] = val;
			exponent = UINT32_BITS;
			normalize(mantisse, MANTISSE_UINT32_BLOCKS, exponent, meta);
		}
		else {
			signum = 0;
			exponent = 0;
		}
	}

	//	mantisse contains an unsigned integer and signum indicates the sign
	void makeFp() {
		if (signum == 0) {
			setToZeroFp();
		}
		else {
			exponent = MANTISSE_BITS;
			normalize(mantisse, MANTISSE_UINT32_BLOCKS, exponent, meta);
		}
	}


	/*
	void setToOne() {
		binSetZero(mantisse, MANTISSE_UINT32_BLOCKS);
		mantisse[MANTISSE_UINT32_BLOCKS - 1] = MSB_MASK;
		exponent = 0;
		signum = 1;
	}*/

	void setToOneFp() {
		binSetZero(mantisse, MANTISSE_UINT32_BLOCKS);
		mantisse[0] = MSB_MASK;
		exponent = 1;
		signum = 1;
	}

public:

	RatherLongFloat() {
		mantisse = new UINT32[MANTISSE_UINT32_BLOCKS];
		setToZeroFp();
	}

	~RatherLongFloat() {
		delete[] mantisse;
	}

	/*
	void setFromUint32(UINT32 val) {
		binSetZero(mantisse, MANTISSE_UINT32_BLOCKS);
		mantisse[MANTISSE_UINT32_BLOCKS - 1] = val; move left
		exponent = 0; much higher
		signum = 0;
		normalize(mantisse, MANTISSE_UINT32_BLOCKS, exponent, meta);
	}*/

	void test(HDC hdc) {
		int lin = 0;
		COLORREF colour = 0xc0ffc0;
		SetBkColor(hdc, colour);

		UINT32 yff = MSB_MASK;

		UINT32 a[2], b[2], r[4];
		UINT32 val = 0xFFffFFff;
		//a[0] = val;
		a[0] = 478888;
		//a[1] = val;
		a[1] = 5211;
		//b[0] = val;
		b[0] = 654;
		//b[1] = val-1;
		b[1] = 987;



		// 2056808298452059 * 2808908612571 = 5.777.386.543.929.492.332.648.233.689

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

		int x = sizeof(UINT32) * BITS_PR_BYTE;

		int binLength = 2;
		int decLength = 40;
		BYTE* dec = new BYTE[decLength];

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



		RatherLongFloat f1, f2, f3;	
		f1.mulFp(&f2, &f3);

		f1.copyMantisse(f1.mantisse,a);
		f1.signum = +1;
		
		f1.makeFp();
		f1.printHexFp(hdc, 4);

		f1.setToOneFp();
		f1.negativeFp();
		f1.printHexFp(hdc, 5);

		f1.fromUint32Fp(0xffffFFff);
		f1.printHexFp(hdc, 6);

		f1.fromUint32Fp(0xffff);
		f1.printHexFp(hdc, 7);

		f1.mulFp(&f1, &f1);	//	this = this * this	
		f1.printHexFp(hdc, 8);


		f1.setToOneFp();	// n!
		for (int i = 2; i < 8; i++) {
			f2.fromIntegerFp(i);
			f1.mulFp(&f1, &f2);
		}
		f1.printHexFp(hdc, 9);
		

		b[0] = 0xffFFffFF;
		b[1] = 0xffFFffFF;
		b[1] = 0xf0000000;
		b[1] = 0;

		binFracToDec(dec, decLength, b, 2);
		decPrint(dec, decLength, hdc, 10);

		delete[]dec;
	}
};




void dummy(void) {
	//DWORD64 a = 0x1234567890ABCDEF;
	BYTE a[] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF };
	DWORD64 b = 0x0FEDCBA098765432;
	DWORD64 c = 0;
	WORD w = 17;


	__asm {
		mov ax, w
		; add rax, b
		; mov c, rax
	}

}











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
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAGNACALC));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
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

	if (!hWnd) {
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

	UINT32 proc = 0;

	switch (message) {
	case WM_CREATE:
		dummy();

#ifdef _AMD64_
		proc = 1;
#endif
#ifdef _X86_
		proc = 2;	//	the chosen one
#endif
#ifdef _IA64_
		proc = 3;
#endif
#ifdef _ARM64_
		proc = 4;
#endif
#ifdef _ARM_
		proc = 5;
#endif
		break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
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


		RatherLongFloat* ratherLongFloat = new RatherLongFloat();
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
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
