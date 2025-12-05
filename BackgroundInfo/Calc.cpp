#include "windows.h"
#include "Calc.rc"


#define CE_CTRL 1
#define CE_SHIFT 2
#define CE_ALT 4


HINSTANCE	hInst;
char 		ApplNavn[]="CT-Kalkulator";
HWND 		Ramme;

/*
int fcreat=0;

void Msg(char * txt, unsigned val)
{	char buf[100];
	wsprintf(buf, "%d %s unsigned %u", fcreat, txt, val);
    SetWindowText(Ramme, buf);

}

void Msg(char * txt, void * val)
{	char buf[100];
	wsprintf(buf, "%d %s void ptr %p", fcreat, txt, val);
    SetWindowText(Ramme, buf);

} */



/////////////////////////////////


//	ombyt hvis a>b --> a<=b
template <class T> void Order(T & a, T & b)
{	T t;
	if(a>b)
	{	t=a;
		a=b;
		b=t;
	}
}

//	a<l --> a=l   a>h --> a=h  | l<=h
template <class T> void Limit(T & a, T & l, T & h)
{	if(a<l) a=l;
	else if(a>h) a=h;
}



unsigned long GetTextExtent(HDC hdc, char * cp, int len)
{	SIZE sz;

    GetTextExtentPoint(hdc, cp, len, &sz);

    return (sz.cy<<16) | (sz.cx&0xFFFF);
}

////////////////////////////////////////////////////////


HFONT hfontOld, hfont, hfontBold, hfontItal, hfontHead;
int	FaceSize=12;
int	LineSpace, LineSpace1, LineSpacePercent=110;
int	HeadHeight, HeadAscent;
int	Ascent, Descent;


COLORREF RGBdimmed, RGBnormal, RGBerror, RGBcomment, RGBliteral;
COLORREF RGBred, RGBblue;


char	FaceName[100]="Arial";


#define SF_NORM 0
#define SF_BOLD 1
#define SF_ITAL 2
#define SF_HEAD 3



void SelectFont(HDC hdc, int type=0)
{	SelectObject(hdc, hfontOld);
	switch(type)
	{	case SF_BOLD:
			SelectObject(hdc, hfontBold);
			break;
		case SF_ITAL:
			SelectObject(hdc, hfontItal);
			break;
		case SF_HEAD:
			SelectObject(hdc, hfontHead);
			break;
		default:
			SelectObject(hdc, hfont);
			break;
	}
}




void MakeFont(HDC hdc)
{	LOGFONT lf;
	TEXTMETRIC tm;


	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = -MulDiv(FaceSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	strcpy(lf.lfFaceName, FaceName);

	hfont = CreateFontIndirect(&lf);

	hfontOld = SelectObject(hdc, hfont);
	LineSpace1=HIWORD(GetTextExtent(hdc, "A",1));
	LineSpace=MulDiv(LineSpace1, LineSpacePercent, 100);
	GetTextMetrics(hdc, &tm);
	Ascent=tm.tmAscent;
	Descent=tm.tmDescent;
	SelectObject(hdc, hfontOld);



	lf.lfWeight=700;
	hfontBold = CreateFontIndirect(&lf);

	lf.lfWeight=0;
	lf.lfItalic=1;
	hfontItal = CreateFontIndirect(&lf);


	lf.lfWeight=700;
	lf.lfItalic=0;
	lf.lfHeight = -MulDiv(MulDiv(FaceSize, 150, 100), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	hfontHead = CreateFontIndirect(&lf);

	hfontOld = SelectObject(hdc, hfontHead);
	GetTextMetrics(hdc, &tm);
	HeadAscent=tm.tmAscent;
	HeadHeight=tm.tmHeight;
	SelectObject(hdc, hfontOld);




	RGBdimmed=RGB(192,192,192);
	RGBnormal=RGB(0,0,0);
	RGBerror=RGB(255,0,0);
	RGBcomment=RGB(0,128,0);
	RGBliteral=RGB(0,0,255);
	RGBred=RGB(255,0,0);
	RGBblue=RGB(0,0,255);
}

void RemoveFont(HDC hdc)
{	SelectObject(hdc, hfontOld);
	DeleteObject(hfont);
	DeleteObject(hfontBold);
	DeleteObject(hfontItal);
	DeleteObject(hfontHead);
}



////////////////////////////////////////////////////////


#define CE_MAXTEXTLEN 1024

class CellEdit
{	private:
		char txt[CE_MAXTEXTLEN];

		int CurX;			//	antal tegn fra første i buffer
		int OldCurX;
		int OffX;			//	afst. fra start af tekst til clip.left

		int OffY;			//	baselines placering fra toppen af clip rect
		int CurOffY;		//	markørs pla. fra top af clip rect

		RECT clip;
//		HFONT MyFont;

		int MarkOn, MarkFrom;

		void SetMark(HDC hdc);
		void ViewMark(HDC hdc, RECT * rcPaint);
		void HideMark(HDC hdc);


		void JustClip(void);

		void CreateCursor(HDC hdc, HWND hWnd);
		void SetCursorPosition(HDC hdc);
		int GetCursorPosition(HDC hdc);

		int NextWord(void);
		int PrevWord(void);

		void BlockDelete(HWND hWnd, HDC hdc);
		int MusCharIdx(HDC hdc, int x, int y);

	public:
		void Print(HDC hdc, RECT * rcPaint);
		int Cmd(unsigned key, unsigned mod, HWND hWnd);
		int Char(WPARAM wP, HWND hWnd);
		void SetClip(RECT * cell);
		void LoadText(char * text);
		void GetText(char * text);

		void KillFocus(void);
		//void SetFocus(HWND hWnd, HDC hdc, HFONT hFont);
        void SetFocus(HWND hWnd, HDC hdc);

		void Mus(HWND hWnd, int x, int y, int mod);

		//CellEdit(HWND hWnd, HFONT hFont, RECT * cell);
        CellEdit(HWND hWnd, RECT * cell);
		~CellEdit(void);
};



//	opretter ny markering mellem OldCurX (--> MarkFrom) og CurX eller udvider
void CellEdit::SetMark(HDC hdc)
{	RECT r;
	if(!MarkOn)
	{	MarkOn=1;
		MarkFrom=OldCurX;
	}
	r=clip;
	r.left=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, OldCurX));
	Limit(r.left, clip.left, clip.right);
	r.right=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, CurX));
	Limit(r.right, clip.left, clip.right);

	Order(r.left, r.right);
	if(r.left<r.right) InvertRect(hdc, &r);

}

//	visuelle aspekter af en markering, der skal opdateres
void CellEdit::ViewMark(HDC hdc, RECT * rcPaint)
{	RECT r;
	r=clip;

	r.left=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, MarkFrom));
	r.right=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, CurX));

	Order(r.left, r.right);
	Limit(r.left, clip.left, clip.right);
	Limit(r.right, clip.left, clip.right);

	IntersectRect(&r, &r, rcPaint);

	if(r.top<r.bottom && r.left<r.right) InvertRect(hdc, &r);
}


//	fjerner alle dele af en markering
void CellEdit::HideMark(HDC hdc)
{	RECT r;
	r=clip;

	r.left=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, MarkFrom));
	r.right=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, CurX));

	Order(r.left, r.right);
	Limit(r.left, clip.left, clip.right);
	Limit(r.right, clip.left, clip.right);

	MarkOn=0;
	if(r.left<r.right) InvertRect(hdc, &r);
}


void CellEdit::JustClip(void)
{	Order(clip.left, clip.right);
	Order(clip.top, clip.bottom);
}




int CellEdit::GetCursorPosition(HDC hdc)
{	return clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, CurX));
}

void CellEdit::SetCursorPosition(HDC hdc)
{	SetCaretPos(clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, CurX)), clip.top+CurOffY);
}




void CellEdit::Print(HDC hdc, RECT * rcPaint)
{	HFONT hfontOld;
	RECT upd;

	IntersectRect(&upd, &clip, rcPaint);
	if(upd.top>=upd.bottom || upd.left>=upd.right) return;

	//hfontOld = SelectObject(hdc, MyFont);
    hfontOld = SelectObject(hdc, hfont);
	SetTextAlign(hdc, TA_BASELINE);
	SetBkMode(hdc, TRANSPARENT);

	SetCursorPosition(hdc);

	ExtTextOut(	hdc, clip.left-OffX, clip.top+OffY, ETO_CLIPPED|ETO_OPAQUE,
					&upd, txt, strlen(txt), NULL);

	if(MarkOn) ViewMark(hdc, rcPaint);

	SelectObject(hdc, hfontOld);
}


void CellEdit::CreateCursor(HWND hWnd, HDC hdc)
{	TEXTMETRIC tm;
	HFONT hFontOld;

	//hFontOld=SelectObject(hdc, MyFont);
    hFontOld=SelectObject(hdc, hfont);

	GetTextMetrics(hdc, &tm);

	CreateCaret(hWnd, NULL, 2, tm.tmHeight);

	CurOffY=clip.bottom-clip.top-tm.tmHeight;

	OffY=clip.bottom-clip.top-tm.tmDescent;

	SetCaretPos(clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, CurX)), clip.top+CurOffY);
	ShowCaret(hWnd);

	SelectObject(hdc, hFontOld);
}

int CellEdit::NextWord(void)
{	int t=CurX;
	while(IsCharAlphaNumeric(*(txt+CurX))) CurX++;
	while(!IsCharAlphaNumeric(*(txt+CurX)) && *(txt+CurX)) CurX++;
	return CurX-t;
}

int CellEdit::PrevWord(void)
{	int t=CurX;
	if(!CurX) return 0;

	//	dette og forgående tegn er AlfaNum - gå til starten af ordet
	if(IsCharAlphaNumeric(*(txt+CurX)) && IsCharAlphaNumeric(*(txt+CurX-1)))
	{	while(CurX && IsCharAlphaNumeric(*(txt+CurX-1))) CurX--;
	}
	else	// er vi i starten af et ord eller mellem to ord
	{	CurX--;
		while(CurX && !IsCharAlphaNumeric(*(txt+CurX))) CurX--;
		while(CurX && IsCharAlphaNumeric(*(txt+CurX-1))) CurX--;
	}
	return t-CurX;
}


void CellEdit::BlockDelete(HWND hWnd, HDC hdc)
{	int t, x, c4;
	c4=(clip.right-clip.left)/4;
	if(MarkOn)HideMark(hdc);
	Order(CurX, MarkFrom);
	t=MarkFrom-CurX;
	memmove(txt+CurX, txt+CurX+t, strlen(txt)+1-MarkFrom);
	x=GetCursorPosition(hdc);
	while(x<clip.left)
	{	OffX-=c4;
		x+=c4;
	}
	InvalidateRect(hWnd, &clip, FALSE);
}



int CellEdit::Cmd(unsigned key, unsigned mod, HWND hWnd)
{	int x, t, k, c4, used=0, mark;
	RECT r;
	HDC hdc;
	HFONT hFontOld;

	OldCurX=CurX;

	hdc=GetDC(hWnd);
	//hFontOld=SelectObject(hdc, MyFont);
    hFontOld=SelectObject(hdc, hfont);

	c4=(clip.right-clip.left)/4;

	mark=mod&CE_SHIFT;
	mod&=~CE_SHIFT;



	if(mod==CE_CTRL && (key=='X' || key=='C' || key=='V'))
	{	HANDLE clp;
		char FAR* buf;
		char temp[CE_MAXTEXTLEN];
		if(OpenClipboard(hWnd))
		{	if(key=='V')
			{	if(MarkOn)		//	split buffer i pre- og postindsæt
				{	Order(CurX, MarkFrom);
					strcpy(temp, txt+MarkFrom);
					MarkOn=0;
				} else strcpy(temp, txt+CurX);
				*(txt+CurX)='\0';

				clp=GetClipboardData(CF_TEXT);
				buf = (char FAR*)GlobalLock(clp);

				//	indsæt clipboard
				while(*buf && CurX<CE_MAXTEXTLEN) *(txt+CurX++)=*(buf++);

				//	indsæt post
				strncpy(txt+CurX, temp, CE_MAXTEXTLEN-CurX);
				*(txt+CE_MAXTEXTLEN-1)='\0';
				GlobalUnlock(clp);
			}	//	paste

			else if((key=='C' || key=='X') && MarkOn)
			{	int i, n;
				if(CurX<MarkFrom)
				{	i=CurX;
					n=MarkFrom-CurX;
				} else
				{	i=MarkFrom;
					n=CurX-MarkFrom;
				}
				clp=GlobalAlloc(GPTR, CE_MAXTEXTLEN);
				buf = (char FAR*)GlobalLock(clp);
				strncpy(buf, txt+i, n);
				GlobalUnlock(clp);
				/*clp=*/SetClipboardData(CF_TEXT, clp);

				HideMark(hdc);

				if(key=='X')
				{	memmove(txt+i, txt+i+n, strlen(txt)+1-i-n);
					Order(CurX, MarkFrom);
				} //	cut

			} //	copy & cut

			CloseClipboard();
		}

		if(key=='V' || key=='X')
		{	x=GetCursorPosition(hdc);
			while(x<clip.left)
			{	OffX-=c4;
				x+=c4;
			}
			while(x>clip.right-3)
			{	OffX+=c4;
				x-=c4;
			}
			InvalidateRect(hWnd, &clip, FALSE);
		}
		used=1;
	}


	else if(MarkOn && (key==VK_DELETE || key==VK_BACK) && !(mod&CE_ALT))
	{	BlockDelete(hWnd, hdc);
		used=1;
	}

	else if(mod==0)
	{	switch(key)
		{	case VK_HOME:
				if(MarkOn && !mark) HideMark(hdc);
				if(CurX)
				{	x=OffX;
					CurX=OffX=0;
					if(x) InvalidateRect(hWnd, &clip, FALSE);
					else SetCursorPosition(hdc);
				}
				if(mark) SetMark(hdc);
				used=1;
				break;

			case VK_END:
				if(MarkOn && !mark) HideMark(hdc);
				if(CurX!=(int)strlen(txt))
				{	CurX=strlen(txt);
					x=GetCursorPosition(hdc);
					if(x<clip.right) SetCursorPosition(hdc);
					else
					{	x-=clip.right;
						k=x/c4;
						x-=k*c4;
						OffX+=k*c4;
						if(x>=0) OffX+=c4;
						InvalidateRect(hWnd, &clip, FALSE);
					}
				}
				if(mark) SetMark(hdc);
				used=1;
				break;

			case VK_LEFT:
				if(MarkOn && !mark) HideMark(hdc);
				if(CurX>0)
				{	CurX--;
					x=GetCursorPosition(hdc);
					for(t=0; x+t < clip.left; t+=c4);
					if(t)
					{	OffX-=t;
						ScrollWindow(hWnd, t, 0, NULL, &clip);
					}
					else SetCursorPosition(hdc);
				}

				if(mark) SetMark(hdc);
				used=1;
				break;

			case VK_RIGHT:
				if(MarkOn && !mark) HideMark(hdc);
				if(CurX<(int)strlen(txt))
				{	CurX++;
					x=GetCursorPosition(hdc);

					for(t=0; x-t >= clip.right-3; t+=c4);

					if(t)
					{	OffX+=t;
						ScrollWindow(hWnd, -t, 0, NULL, &clip);
					}
					else SetCursorPosition(hdc);
				}
				if(mark) SetMark(hdc);
				used=1;
				break;

			case VK_BACK:
				if(CurX>0)
				{	x=GetCursorPosition(hdc);
					CurX--;
					r=clip;
					r.left=GetCursorPosition(hdc);
					memmove(txt+CurX, txt+CurX+1, strlen(txt)-CurX);

					if(r.left>=clip.left) ScrollWindow(hWnd, r.left-x, 0, NULL, &r);
					else
					{	for(t=0; r.left+t < clip.left; t+=c4);
						if(t)
						{	OffX-=t;
							InvalidateRect(hWnd, &clip, FALSE);
						}
					}
				}
				used=1;
				break;

			case VK_DELETE:
				if(CurX<(int)strlen(txt))
				{	CurX++;
					x=GetCursorPosition(hdc);
					CurX--;

					memmove(txt+CurX, txt+CurX+1, strlen(txt)-CurX);

					r=clip;
					r.left=GetCursorPosition(hdc);

					ScrollWindow(hWnd, r.left-x, 0, NULL, &r);
				}
				used=1;
				break;

		}
	}	//	mod == 0

	else if(mod==CE_CTRL)
	{	switch(key)
		{

			case VK_LEFT:
				if(MarkOn && !mark) HideMark(hdc);
				if(CurX>0)
				{	PrevWord();
					x=GetCursorPosition(hdc);
					for(t=0; x+t < clip.left; t+=c4);
					if(t)
					{	OffX-=t;
						ScrollWindow(hWnd, t, 0, NULL, &clip);
					}
					else SetCursorPosition(hdc);
				}
				if(mark) SetMark(hdc);
				used=1;
				break;


			case VK_RIGHT:
				if(MarkOn && !mark) HideMark(hdc);
				if(CurX<(int)strlen(txt))
				{	NextWord();

					x=GetCursorPosition(hdc);

					for(t=0; x-t >= clip.right-3; t+=c4);

					if(t)
					{	OffX+=t;
						ScrollWindow(hWnd, -t, 0, NULL, &clip);
					}
					else SetCursorPosition(hdc);
				}
				if(mark) SetMark(hdc);
				used=1;
				break;

			case VK_BACK:
				if(CurX>0)
				{	x=GetCursorPosition(hdc);
					t=PrevWord();
					r=clip;
					r.left=GetCursorPosition(hdc);
					memmove(txt+CurX, txt+CurX+t, strlen(txt)+1-CurX-t);

					if(r.left>=clip.left) ScrollWindow(hWnd, r.left-x, 0, NULL, &r);
					else
					{	for(t=0; r.left+t < clip.left; t+=c4);
						if(t)
						{	OffX-=t;
							InvalidateRect(hWnd, &clip, FALSE);
						}
					}
				}
				used=1;
				break;

			case VK_DELETE:
				if(CurX<(int)strlen(txt))
				{	r=clip;
					r.left=GetCursorPosition(hdc);
					if((t=NextWord())!=0)
					{	x=GetCursorPosition(hdc);
						memmove(txt+CurX-t, txt+CurX, strlen(txt)+1-CurX);
						CurX-=t;
						if(x>r.right) InvalidateRect(hWnd, &r, FALSE);
						else ScrollWindow(hWnd, r.left-x, 0, NULL, &r);
					}
				}
				used=1;
				break;
		}
	}

	SelectObject(hdc, hFontOld);
	ReleaseDC(hWnd, hdc);
	return used;
}


int CellEdit::Char(WPARAM wP, HWND hWnd)
{	HDC hdc;
	int x0, x1, t, c4;
	RECT r;
	HFONT hFontOld;
	if(wP<32) return 0;

	OldCurX=CurX;
	hdc=GetDC(hWnd);

	//hFontOld=SelectObject(hdc, MyFont);
    hFontOld=SelectObject(hdc, hfont);

	c4=(clip.right-clip.left)/4;

	if(MarkOn)
	{	BlockDelete(hWnd, hdc);
		memmove(txt+CurX+1, txt+CurX, strlen(txt)+1-CurX);
		*(txt+CurX)=(char)wP;
		CurX++;
		x1=GetCursorPosition(hdc);
		while(x1>clip.right-3)
		{	OffX+=c4;
			x1-=c4;
		}

	}

	else if(strlen(txt)+1 < CE_MAXTEXTLEN)
	{	x0=GetCursorPosition(hdc);
		memmove(txt+CurX+1, txt+CurX, strlen(txt)+1-CurX);
		*(txt+CurX)=(char)wP;
		CurX++;
		x1=GetCursorPosition(hdc);

		for(t=0; x1-t >= clip.right-3; t+=c4);
		if(t)
		{	OffX+=t;
			InvalidateRect(hWnd, &clip, FALSE);
		}
		else
		{	r=clip;
			r.left=x0;
			ScrollWindow(hWnd, x1-x0, 0, NULL, &r);
		}
	}

	SelectObject(hdc, hFontOld);
	ReleaseDC(hWnd, hdc);
	return 1;
}

void CellEdit::SetClip(RECT * cell)
{	clip=*cell;
	JustClip();
}



void CellEdit::LoadText(char * text)
{	strncpy(txt, text, CE_MAXTEXTLEN);
	txt[CE_MAXTEXTLEN-1]='\0';
}

void CellEdit::GetText(char * text)
{	strcpy(text, txt);
}


void CellEdit::Mus(HWND hWnd, int x, int y, int mod)
{	int idx, mark;
	HDC hdc;
	HFONT hFontOld;

	hdc=GetDC(hWnd);
	//hFontOld=SelectObject(hdc, MyFont);
    hFontOld=SelectObject(hdc, hfont);

	mark=mod==(MK_SHIFT|MK_LBUTTON);

	if(y>=clip.top && y<=clip.bottom && x>=clip.left && x<=clip.right && *txt)
	{	OldCurX=CurX;
		idx = MusCharIdx(hdc, x, y);
		if(idx>=0)
		{	if(!mark && MarkOn) HideMark(hdc);
			CurX=idx;
			SetCursorPosition(hdc);
			if(mark) SetMark(hdc);
		}
	}
	SelectObject(hdc, hFontOld);
	ReleaseDC(hWnd, hdc);
}

//	Font skal være valgt
//	ret -1 hvis uden for clip
//	ret 0..n-1 = index til txt
int CellEdit::MusCharIdx(HDC hdc, int x, int y)
{	int l, m=-1, h, xl, xm, xh, n;

	n=strlen(txt);
	if(y>=clip.top && y<=clip.bottom && x>=clip.left && x<=clip.right && n)
	{	l=0;
		h=n;

		while(l<h)
		{	m=(l+h)/2;
			xm=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, m));
			if(xm<x) l=m+1;
			else h=m;
		}
		l=h-1;
		if(l<0) l=0;

		xl=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, l));
		xh=clip.left-OffX+LOWORD(GetTextExtent(hdc, txt, h));

		if((x-xl) < (xh-x)) m=l;
		else m=h;
		if(xl<clip.left) m=h;
		else if(xh>=clip.right-3) m=l;
	}

	return m;
}



void CellEdit::KillFocus(void)
{	DestroyCaret();
}

//void CellEdit::SetFocus(HWND hWnd, HDC hdc, HFONT hFont)
void CellEdit::SetFocus(HWND hWnd, HDC hdc)
{	//MyFont=hFont;
	CreateCursor(hWnd, hdc);
}

//CellEdit::CellEdit(HWND hWnd, HFONT hFont, RECT * cell)
CellEdit::CellEdit(HWND hWnd, RECT * cell)
{	HDC hdc;
	hdc=GetDC(hWnd);

	txt[0]='\0';
	OldCurX=MarkOn=CurX=OffX=OffY=0;

	//MyFont=hFont;

	SetClip(cell);
	if(clip.right-clip.left < 5) clip.right+=5;	//	ellers kan prog gå ned!!!!


	CreateCursor(hWnd, hdc);

	ReleaseDC(hWnd, hdc);

}

CellEdit::~CellEdit(void)
{	DestroyCaret();
	//DeleteObject(MyFont);
}



////////////////////////////////////////////////////////////









CellEdit * ce;


int RowX0=10, RowX1=790, RowY0=10, RowNbr=0;


LRESULT FAR PASCAL _export RammeProc(HWND hWnd, UINT Msg, WPARAM wP, LPARAM lP)
{	PAINTSTRUCT ps;
	RECT r;
	HDC hdc;
	unsigned mod;
    int i;

    char txt[CE_MAXTEXTLEN];
    char buf[100];

	switch(Msg)
	{	case WM_CREATE:
    		SendMessage(hWnd, WM_KEYDOWN, VK_F2, 0);
			break;

		case WM_DESTROY:
			if(ce)
			{	delete ce;
				ce=NULL;
			}

			PostQuitMessage(0);
			break;

		case WM_PAINT:
			hdc=BeginPaint(hWnd, &ps);

            MakeFont(hdc);

			//SetBkColor(ps.hdc, RGB(192,192,192));
			//SetTextColor(ps.hdc, RGB(0,0,255));
			//SetTextAlign(ps.hdc, TA_BASELINE);	//	TA_UPDATECP

			//SetBkMode(ps.hdc, TRANSPARENT);

			if(ce)
            {	ce->GetText(txt);
	            for(i=0; i<RowNbr; i++)
            	{	SetRect(&r, RowX0, RowY0+LineSpace*i, RowX1, RowY0+LineSpace*(i+1));
	            	ce->SetClip(&r);
                    wsprintf(buf, "%d", i+1);
                    ce->LoadText(buf);
                    ce->Print(hdc, &ps.rcPaint);
                    //ce->Print(hdc, &r);
                }

                SetRect(&r, RowX0, RowY0+LineSpace*RowNbr, RowX1, RowY0+LineSpace*(RowNbr+1));
                ce->SetClip(&r);
            	ce->LoadText(txt);
                ce->Print(hdc, &ps.rcPaint);
                //ce->Print(hdc, &r);
            }


            GetClientRect(hWnd, &r);
            MoveToEx(hdc, 0, r.bottom-2, NULL);
            LineTo(hdc, r.right, r.bottom-2);

            MoveToEx(hdc, 0, r.bottom-5, NULL);
            LineTo(hdc, r.right, r.bottom-5);

            MoveToEx(hdc, 0, r.bottom-10, NULL);
            LineTo(hdc, r.right, r.bottom-10);

            RemoveFont(hdc);
			EndPaint(hWnd, &ps);
			break;

		case WM_LBUTTONDOWN:
            if(ce)
			{	hdc=GetDC(hWnd);
				MakeFont(hdc);
                ce->Mus(hWnd, LOWORD(lP), HIWORD(lP), wP);
                RemoveFont(hdc);
				ReleaseDC(hWnd, hdc);
			}
			break;

		case WM_SETFOCUS:
			if(ce)
			{	hdc=GetDC(hWnd);
				MakeFont(hdc);
                //ce->SetFocus(hWnd, hdc, hfont);
                ce->SetFocus(hWnd, hdc);
                RemoveFont(hdc);
				ReleaseDC(hWnd, hdc);
			}
			break;

		case WM_KILLFOCUS:
			if(ce)
			{	ce->KillFocus();
			}
			break;

		case WM_CHAR:
			if(GetKeyState(VK_CONTROL)<0) break;
			if(GetKeyState(VK_MENU)<0) break;
			if(wP<0x20) break;
			if(ce)
            {	hdc=GetDC(hWnd);
	            MakeFont(hdc);
            	ce->Char(wP, hWnd);
    			RemoveFont(hdc);
				ReleaseDC(hWnd, hdc);
            }
			break;

		case WM_KEYDOWN:
			if(wP==VK_CONTROL || wP==VK_MENU || wP==VK_SHIFT) break;
			mod=(GetKeyState(VK_CONTROL)<0) ? CE_CTRL:0;
			mod|=(GetKeyState(VK_MENU)<0) ? CE_ALT:0;
			mod|=(GetKeyState(VK_SHIFT)<0) ? CE_SHIFT:0;

			if(ce)
            {	if(wP==VK_RETURN)
            	{	RowNbr++;
	                SetRect(&r, RowX0, RowY0+LineSpace*RowNbr, RowX1, RowY0+LineSpace*(RowNbr+1));
                    ce->SetClip(&r);
                    //InvalidateRect(hWnd, &r, FALSE);
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                else if(wP==VK_UP)
            	{	SetRect(&r, RowX0, RowY0+LineSpace*RowNbr, RowX1, RowY0+LineSpace*(RowNbr+1));
                    InvalidateRect(hWnd, &r, TRUE);
                    RowNbr--;
                    SetRect(&r, RowX0, RowY0+LineSpace*RowNbr, RowX1, RowY0+LineSpace*(RowNbr+1));
                    ce->SetClip(&r);
                    InvalidateRect(hWnd, &r, FALSE);	//	SetCaretPos...
                }
	            else
                {	hdc=GetDC(hWnd);
					MakeFont(hdc);
					ce->Cmd(wP, mod, hWnd);
                    RemoveFont(hdc);
                    ReleaseDC(hWnd, hdc);
                }
            }
			else
			{	if(!mod && wP==VK_F2)
				{	hdc=GetDC(hWnd);
					MakeFont(hdc);
					SetRect(&r, RowX0, RowY0+LineSpace*RowNbr, RowX1, RowY0+LineSpace*(RowNbr+1));
					//ce=new CellEdit(hWnd, hfont/*Head*/, &r);
                    ce=new CellEdit(hWnd, &r);
                    RemoveFont(hdc);
                    ReleaseDC(hWnd, hdc);
					InvalidateRect(hWnd, &r, FALSE);
				}
			}
			break;

		case WM_COMMAND:
			switch(wP)
			{	case CM_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				case CM_HELP:
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, Msg, wP, lP);
	} return 0;
}





int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int /*show*/)
{	MSG Msg;
	hInst=hInstance;
	WNDCLASS WndClass;
	memset(&WndClass, 0, sizeof(WndClass));

	WndClass.style			=CS_HREDRAW|CS_VREDRAW|CS_BYTEALIGNWINDOW;
	WndClass.lpfnWndProc	=RammeProc;
	WndClass.hInstance		=hInst;
	WndClass.hIcon			=LoadIcon(hInst, MAKEINTRESOURCE(ICONEN));
	WndClass.hCursor		=LoadCursor(NULL, IDC_ARROW);
//	WndClass.hbrBackground	=GetStockObject(WHITE_BRUSH);//GetStockObject(LTGRAY_BRUSH);
	WndClass.hbrBackground	=GetStockObject(LTGRAY_BRUSH);
	WndClass.lpszMenuName	=MAKEINTRESOURCE(MENUEN);
	WndClass.lpszClassName	=ApplNavn;

	RegisterClass(&WndClass);

	Ramme=CreateWindow(ApplNavn,ApplNavn,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|/*WS_MAXIMIZEBOX|*/WS_MINIMIZEBOX,100,100,400,300,NULL,NULL,hInst,NULL);
    RECT r;
    //GetClientRect(NULL, &r);
    GetWindowRect(HWND_DESKTOP, &r);
    r.left=0;
    r.top=0;
    r.right=800;
    r.bottom=600;

    //Ramme=CreateWindow(ApplNavn,ApplNavn,WS_OVERLAPPEDWINDOW, r.left+10,r.top+10,r.right-20,r.bottom-50,NULL,NULL,hInst,NULL);
    ShowWindow(Ramme,SW_SHOWMAXIMIZED);
	//ShowWindow(Ramme,show);




	while(GetMessage(&Msg, NULL, 0,0))
	{	TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}


	UnregisterClass(ApplNavn, hInst);

	return Msg.wParam;
}


