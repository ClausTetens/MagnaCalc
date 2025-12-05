#include "stdio.h"
#include "string.h"
#include "windows.h"

template <class T> void swap(T & a, T & b)
{	T c;
	c=a;
    a=b;
    b=c;
}

template <class T> T min(T & a, T & b)
{	return (a<b)? a : b;
}


bool IsDig(char ch)
{	return (ch>='0' && ch<='9') ? true : false;
}
bool IsDig(char * cp)
{	return IsDig(*cp);
}

bool IsLU(char ch)
{	return
		(ch>='a' && ch<='z' || ch>='A' && ch<='Z' || ch=='_' ||
         ch=='æ' || ch=='ø' || ch=='å' || ch=='Æ' || ch=='Ø' || ch=='Å')
        ? true : false;
}

bool IsLU(char * cp)
{	return IsLU(*cp);
}


bool IsLDU(char ch)
{	return
		(ch>='0' && ch<='9' || ch>='a' && ch<='z' || ch>='A' && ch<='Z' ||
        ch=='_' || ch=='æ' || ch=='ø' || ch=='å' || ch=='Æ' || ch=='Ø' || ch=='Å')
        ? true : false;
}

bool IsLDU(char * cp)
{	return IsLDU(*cp);
}


typedef unsigned char UC;
int BcdDig=300;
int DispDig=300;
#define BCDDIG 1000
#define BCDROUND 1000

#define BCD_NULL  0x0001
#define BCD_PLUS  0x0002
#define BCD_MINUS 0x0004
#define BCD_INF   0x0008
#define BCD_NAN   0x0010
#define BCD_OVF   0x0020


#define BCD_TEXT_NTZ	0x0001
#define BCD_TEXT_EXP	0x0002
#define BCD_TEXT_FIX	0x0004
#define BCD_TEXT_SEP	0x0008
#define BCD_TEXT_UFP	0x0010
#define BCD_TEXT_RND	0x0020

#define BCD_TEXT_LEN	(BCDDIG*2+25)

#define TRIG_SIN 1
#define TRIG_COS 2
#define TRIG_TAN 4


class BCD
{	private:
		//	uden fortegn
		void Add(UC *v1, int & x1, UC *v2, int x2);	//	v1,x1 += v2,x2
        int DigCmp(UC *v1, UC *v2, int dig); // ciffervis sammenligning
        int DigAdd(UC *v1, UC *v2, int dig);
        int Sub(UC *v1, int & x1, UC *v2, int x2);
        int DigSub(UC *v1, UC *v2, int dig);
        int DigRound(UC *v1, int dig);
        int DigInc(UC *v1, int dig);

        bool DigIsNull(UC * v1, int dig);

        int Eq(BCD & a, BCD & b);

        BCD LnFrac(void);
        BCD ExpFrac(void);

        //	kalkulerer s=k*sin(x), c=k*cos(x) (div. m. vektorlængden for sin/cos)
        //	x = [0; pi/2[
        void SinCos(BCD & s, BCD & c);
        bool ArcSin(BCD & as);

        bool TrigSCT(BCD & s, BCD & c, BCD & t, int calcflag);

	protected:
        short sw;		//	kombinationer af BCD_MINUS|BCD_INF|BCD_NAN
        UC val[BCDDIG];	//	0.dddddd....
        int expo;

    public:

    	void Print(unsigned flag=BCD_TEXT_NTZ)
        {	char buf[BCD_TEXT_LEN];
        	//ToText(buf, BCD_TEXT_LEN, BCDROUND, flag);
            ToText(buf, BCD_TEXT_LEN, DispDig, flag);
            puts(buf);
        }

    	bool ToText(char * cp, int buflen, int dig, unsigned flag);

        void FixPrint(int dig, unsigned flag)
        {	char buf[BCD_TEXT_LEN];
        	ToFixText(buf, BCD_TEXT_LEN, dig, flag);
            dig=72-strlen(buf);
            if(dig>0) printf("%*c", dig, ' '); // alignment
            puts(buf);
        }

        bool ToFixText(char * cp, int buflen, int dig, unsigned flag);

		bool IsNumber(void);

        bool SetVal(char * cp);
        void SetVal(int b);

    	BCD operator +(BCD & b);
        BCD operator -(BCD & b);
        BCD operator *(BCD & b);
        BCD operator /(BCD & b);

        bool operator ==(BCD & b);
        bool operator !=(BCD & b);

        bool operator >(BCD & b);
        bool operator <=(BCD & b);

        bool operator <(BCD & b);
        bool operator >=(BCD & b);

        BCD RelRound(int dig);	//	dig cifre ialt
        BCD Round(int dig);		//	dig cifre efter komma
        BCD Int(void);
        bool IntegerOk(void);	//	tester med BcdMax
		int Integer(void);
        BCD Frac(void);
        BCD Abs(void);
        BCD Neg(void);

        BCD MulAE(int a, int e=0);

        BCD Sqrt(void);
        BCD Pow(int potens);
        BCD Pow(BCD & potens);
        BCD Fakultet(void);
        BCD ExpXm1(void);
        BCD Exp(void);
        BCD Ln1px(void);
        BCD Ln(void);


        BCD Sin(void);
        BCD Cos(void);
        BCD Tan(void);
        BCD ASin(void);
        BCD ACos(void);
        BCD ATan(void);


        BCD TaylorSin(void);
        BCD TaylorCos(void);
        BCD TaylorArcTan(void);
        BCD SlowSin(void);
        BCD SlowCos(void);

        int Expo(void) { return expo; }

		static void KonstInit(int digs);

        BCD(int b);
        BCD(char * cp);
        BCD(BCD & b);
    	BCD(void);
        ~BCD(void);
};

//#include "bcdinit.cpp"

#include "bcdinit2.cpp"

BCD BCD_e;
BCD BCD_ln10;
BCD BCD_pi4, BCD_pi2, BCD_pi, BCD_2pi;
BCD BCD_ln1_n[100];
BCD BCD_atan[100];

BCD IntMax(2147483647);


void BCD::KonstInit(int digs)
{
	if(digs<50) BcdDig=50;
    else if(digs>BCDDIG) BcdDig=BCDDIG;
    else BcdDig=digs;

    BCD_e.SetVal(BCD_TEXT_e);
    BCD_ln10.SetVal(BCD_TEXT_ln10);
    BCD_pi4.SetVal(BCD_TEXT_pi4);
    BCD_pi2.SetVal(BCD_TEXT_pi2);
    BCD_pi.SetVal(BCD_TEXT_pi);
    BCD_2pi.SetVal(BCD_TEXT_2pi);

    for(int i=0; i<100; i++)
    {	BCD_ln1_n[i].SetVal(BCD_TEXT_ln1_n[i]);
	    BCD_atan[i].SetVal(BCD_TEXT_atan[i]);
    }
}




void BCD::Add(UC *v1, int & x1, UC *v2, int x2)
{	UC vt1[BCDDIG+2], vt2[BCDDIG+2];

	if(x1 > x2+BcdDig) return;
	if(x2 > x1+BcdDig)
    {	x1=x2;
    	memmove(v1, v2, BcdDig);
        return;
    }


    memset(vt1, 0, BcdDig+2);
    memset(vt2, 0, BcdDig+2);
    int xd=x1-x2;


    if(xd>=0)
    {	memmove(vt1+1, v1, BcdDig);
    	if(xd>0) memmove(vt2+1+xd, v2, BcdDig+1-xd);
        else memmove(vt2+1+xd, v2, BcdDig);
    }
    else
    {	xd=-xd;
    	memmove(vt1+1, v2, BcdDig);
    	memmove(vt2+1+xd, v1, BcdDig+1-xd);
        x1=x2;
    }

    vt1[0]=(UC)DigAdd(vt1+1, vt2+1, BcdDig+1);
    DigRound(vt1, BcdDig+2);

    if(*vt1)
    {	x1++;
	    memmove(v1, vt1, BcdDig);
    }
    else memmove(v1, vt1+1, BcdDig);
}

//	<0 chg sign, ==0 BCD_NULL, >0 Ok
int BCD::Sub(UC *v1, int & x1, UC *v2, int x2)
{	UC vt1[BCDDIG+1], vt2[BCDDIG+1];

	if(x1 > x2+BcdDig) return +1;
	if(x2 > x1+BcdDig)
    {	x1=x2;
    	memmove(v1, v2, BcdDig);
        return -1;
    }


    memset(vt1, 0, BcdDig+1);
    memset(vt2, 0, BcdDig+1);
    int xd=x1-x2, cs=+1;

    if(xd==0) cs=DigCmp(v1, v2, BcdDig);

    if(xd>=0 && cs>0)
    {	memmove(vt1, v1, BcdDig);
    	if(xd>0) memmove(vt2+xd, v2, BcdDig+1-xd);
        else memmove(vt2+xd, v2, BcdDig);
    }
    else if(cs<0 || xd<0)
    {	xd=-xd;
    	memmove(vt1, v2, BcdDig);
    	if(xd>0) memmove(vt2+xd, v1, BcdDig+1-xd);
        else memmove(vt2+xd, v1, BcdDig);
        x1=x2;
        cs=-1;
    }
    else
    {   memset(v1, 0, BcdDig);
        x1=0;
        return 0;
    }

	DigSub(vt1, vt2, BcdDig+1);
    DigRound(vt1, BcdDig+1);
    vt1[BcdDig]=0;


    if(DigIsNull(vt1, BcdDig))
    {	memset(v1, 0, BcdDig);
    	return 0;
    }


    while(*vt1 == 0)
    {	x1--;
    	memmove(vt1, vt1+1, BcdDig);
        vt1[BcdDig]=0;
    }

    memmove(v1, vt1, BcdDig);

    return cs;
}


//	<0 v1<v2    ==0 v1==v2   >0 v1>v2
int BCD::DigCmp(UC *v1, UC *v2, int dig)
{	for( ; dig>0 && *v1==*v2; dig--)
	{	v1++;
    	v2++;
    }

	return ((int)(*v1) - (int)(*v2));
}


int BCD::DigAdd(UC *v1, UC *v2, int dig)
{	int cy=0, i;
    for(i=dig-1; i>=0; i--)
    {	cy+=v1[i]+v2[i];
        v1[i]=(UC)(cy%10);
        cy/=10;
    }

	return cy;
}

int BCD::DigSub(UC *v1, UC *v2, int dig)
{	int cy=0, i;
    for(i=dig-1; i>=0; i--)
    {	cy=v1[i]-v2[i]-cy;
        if(cy<0)
        {	v1[i]=(UC)(cy+10);
        	cy=1;
        }
        else
        {	v1[i]=(UC)cy;
        	cy=0;
        }
    }

	return cy;
}


int BCD::DigRound(UC *v1, int dig)
{	int cy=5, i;
    for(i=dig-1; i>=0 && cy; i--)
    {	cy+=v1[i];
        v1[i]=(UC)(cy%10);
        cy/=10;
    }

    return cy;
}


int BCD::DigInc(UC *v1, int dig)
{	int cy=1, i;
    for(i=dig-1; i>=0 && cy; i--)
    {	cy+=v1[i];
        v1[i]=(UC)(cy%10);
        cy/=10;
    }

    return cy;
}



BCD BCD::MulAE(int a, int e)
{	UC v[BCDDIG+20];
	BCD t(*this);
	int cy=0, i;
    bool sgn=false;

    if(a<0)
    {	sgn=true;
    	a=-a;
    }

    while(a>0 && a%10 == 0)
    {	a/=10;
    	e++;
    }

    if(a==0 || t.sw&BCD_NULL)
    {	t.SetVal(0);
    	return t;
    }

    if(!t.IsNumber())
    {	t.sw=BCD_NAN;
    	return t;
    }

    if(a>1)
    {	for(i=0; i<20; i++) v[i]=0;

        for(i=BcdDig-1; i>=0; i--)
        {	cy+=t.val[i]*a;
            v[i+20]=(UC)(cy%10);
            cy/=10;

        }

        i=19;
        while(cy)
        {	v[i]=(UC)(cy%10);
            i--;
            cy/=10;
        }


        while(!v[0])
        {	memmove(v, v+1, BcdDig-1);
            e--;
        }

        if(DigRound(v, BcdDig+1))
        {	e++;
            v[0]=1;
        }

        memmove(t.val, v, BcdDig);
        e+=20;
    }

    e+=t.expo;
    t.expo=e;

    if(sgn)
    {	if(t.sw&BCD_PLUS) t.sw=BCD_MINUS;
	    else if(t.sw&BCD_MINUS) t.sw=BCD_PLUS;
    }

    if(t.expo!=e) sw=BCD_OVF;

    return t;
}





bool BCD::DigIsNull(UC * v1, int dig)
{	while(dig--) if(*(v1++)) return false;
	return true;
}




inline bool BCD::IsNumber(void)
{	return (sw& ~(BCD_MINUS | BCD_PLUS | BCD_NULL))? false : true;
}


//	-1: meget forskellige (expo, sign); >=0: antal ens cifre
int BCD::Eq(BCD & a, BCD & b)
{	if(a.sw != b.sw || a.expo != b.expo) return -1;
	int i;
    for(i=0; i<BcdDig; i++) if(a.val[i] != b.val[i]) return i;

    return BcdDig;
}

bool BCD::ToText(char * cp, int buflen, int dig, unsigned flag)
{	BCD t(*this);
	char * cp0=cp;

	memset(cp, '\0', buflen);

	t=t.RelRound(dig);

	if(t.sw&(BCD_INF|BCD_NAN))
	{	if(t.sw&BCD_NAN) strcpy(cp, "NaN");
    	else
        {	if(t.sw&BCD_MINUS) strcpy(cp, "-Inf");
        	else strcpy(cp, "+Inf");
		}
	    return false;
    }
    if(t.sw&BCD_MINUS) *(cp++)='-';
    else
    {	*(cp++)='+';
    	buflen--;
    }

    if(t.sw&BCD_NULL)
    {	memset(val, 0, BcdDig);
    	expo=0;
    }

    UC Val[BCDDIG];
    int tx=t.expo, d=0;

    memmove(Val, t.val, BcdDig);
/*    if(DigRound(Val, BcdDig))
    {	Val[0]=1;
    	tx++;
    }
*/


    bool komma=false;

    if(flag&BCD_TEXT_FIX)
    {	if(tx<=0)
    	{	*(cp++)='0';
	        *(cp++)=',';
            buflen-=2;
            komma=true;

            while(tx<0 && buflen)
        	{	*(cp++)='0';
            	buflen--;
	           	tx++;
            }
        }

    	else
        {	while(tx && (d<BcdDig-1) && buflen)
        	{	*(cp++)=(UC)((Val[d++]&0x0F)+'0');
	           	tx--;
                buflen--;
                if(flag&BCD_TEXT_SEP && tx>0 && tx%3==0)
                {	*(cp++)='.';
                	buflen--;
                }

            }
            while(tx && buflen)
        	{	*(cp++)='0';
            	buflen--;
	           	tx--;
                if(flag&BCD_TEXT_SEP && tx>0 && tx%3==0)
                {	*(cp++)='.';
                	buflen--;
                }
            }
        }

        if(buflen<=0)
        {	strcpy(cp0, "+Inf");
        	return false;
        }
    }

    else if(flag&BCD_TEXT_EXP)
	{	do
        {	*(cp++)=(UC)((Val[d++]&0x0F)+'0');
           	tx--;
    	} while(tx%3);
    }

    else
    {	*(cp++)=(UC)((Val[d++]&0x0F)+'0');
    }

    if(!komma) *(cp++)=',';

    for( ; d<BcdDig-1; d++)
    {	*(cp++)=(UC)((Val[d]&0x0F)+'0');
    }

    if(flag&BCD_TEXT_NTZ)
    {	cp--;
    	while(*cp=='0') cp--;
    	if(*cp==',') cp--;
        cp++;
    }

    if(flag&BCD_TEXT_FIX)
    {	*cp='\0';
    }

    else
    {	*(cp++)='E';
	    sprintf(cp,"%+05d", (t.sw&BCD_NULL)?0:((flag&BCD_TEXT_EXP)?tx:tx-1));
    }

    return true;
}


bool BCD::ToFixText(char * cp, int buflen, int dig, unsigned flag)
{	BCD t(*this);
	char * cp0=cp;

	memset(cp, '0', buflen);
    *(cp+buflen-1)='\0';

	t=t.Round(dig);

	if(t.sw&(BCD_INF|BCD_NAN) || expo>buflen/3)
	{	if(t.sw&BCD_NAN) strcpy(cp, "NaN");
    	else
        {	if(t.sw&BCD_MINUS) strcpy(cp, "-Inf");
        	else strcpy(cp, "+Inf");
		}
	    return false;
    }

    if(t.sw&BCD_MINUS) *(cp++)='-';
    else
    {	*(cp++)='+';
    	buflen--;
    }

    if(t.sw&BCD_NULL)
    {	memset(t.val, 0, BcdDig);
    	t.expo=0;
    }

    UC Val[BCDDIG];
    int tx=t.expo, d=0;

    memmove(Val, t.val, BcdDig);

    char * komma=NULL;	//	første pos efter komma

    if(tx<=0)
    {	*(cp++)='0';
        *(cp++)=',';
        buflen-=2;
        komma=cp;

        while(tx<0 && buflen)
        {	*(cp++)='0';
            buflen--;
            tx++;
        }
    }

    else
    {	while(tx && (d<BcdDig-1) && buflen)
        {	*(cp++)=(UC)((Val[d++]&0x0F)+'0');
            tx--;
            buflen--;
            if(flag&BCD_TEXT_SEP && tx>0 && tx%3==0)
            {	*(cp++)='.';
                buflen--;
            }

        }
        while(tx && buflen)
        {	*(cp++)='0';
            buflen--;
            tx--;
            if(flag&BCD_TEXT_SEP && tx>0 && tx%3==0)
            {	*(cp++)='.';
                buflen--;
            }
        }
    }


    if(!komma && buflen>0)
    {	*(cp++)=',';
    	komma=cp;
        buflen--;
    }

    for( ; d<BcdDig-1 && buflen>0; d++)
    {	*(cp++)=(UC)((Val[d]&0x0F)+'0');
    	buflen--;
    }


    if(dig==0) *(komma-1)='\0';
    else
    {	if(cp+buflen-1 >= komma+dig) *(komma+dig)='\0';
	}

    if(buflen<=0)
    {	strcpy(cp0, (t.sw&BCD_MINUS) ? "-Inf" : "+Inf");
        return false;
    }


    return true;
}



//	[+|-] [d...] , [d...] [e[+|-]d...]
// false hvis der er ukonverterede tegn
bool BCD::SetVal(char * cp)
{	int tx=0, x=0, idx=0, se=+1;
	bool komma=false, round;

	memset(val,0, BcdDig);
    while(*cp==' ' || *cp=='\t') cp++;

	sw=BCD_PLUS;
	if(*cp=='-')
    {	sw=BCD_MINUS;
    	cp++;
    }
    if(*cp=='+') cp++;

    //	fjern leading zeroes and other stuff
    while(*cp && !(*cp>='1' && *cp<='9'))
    {	if(*cp==' ' || *cp=='\t' || *cp=='.') cp++;
    	else if(*cp==',')
        {	komma=true;
        	cp++;
        }
        else if(*cp=='0')
        {	if(komma) tx--;
        	cp++;
        }
    }

    while(*cp && idx<BcdDig)
    {	if(*cp>='0' && *cp<='9')
    	{	val[idx++]=(UC)(*(cp++)-'0');
        	if(!komma) tx++;
        }
    	else if(*cp=='.') cp++;
        else if(*cp==',')
        {	komma=true;
        	cp++;
        }
        else if(*cp==' ' || *cp=='\t') cp++;
        else break;
    }

    round = *cp>='5' && *cp<='9';

    //	Videre! Der kunne jo være en 'E+251' tilsidst!
    while(*cp)
    {	if(*cp>='0' && *cp<='9')
    	{	if(!komma) tx++;
        	cp++;
        }
    	else if(*cp=='.') cp++;
        else if(*cp==',')
        {	komma=true;
        	cp++;
        }
        else if(*cp==' ' || *cp=='\t') cp++;
        else break;
    }


    if(*cp=='e' || *cp=='E')
    {	cp++;
    	while(*cp==' ' || *cp=='\t') cp++;
    	if(*cp=='-')
        {	se=-1;
            cp++;
        }
        if(*cp=='+') cp++;

        while(*cp==' ' || *cp=='\t') cp++;

	    while(*cp)
	    {	if(*cp>='0' && *cp<='9')
        	{	x*=10;
            	x+=*cp-'0';
                cp++;
            }
        }
    }


    tx += se*x;

    if(round)
    {	if(DigInc(val, BcdDig))
    	{	tx++;
        	val[0]=1;
        }
    }

    expo=tx;
    if(expo!=tx) sw|=BCD_OVF;

    if(DigIsNull(val, BcdDig))
    {	sw=BCD_NULL;
    	expo=0;
    }

    return (*cp)?false:true;
}

BCD BCD::operator +(BCD & b)
{	BCD a(*this);

	if(a.IsNumber() && b.IsNumber())
	{	if(b.sw&BCD_NULL) return a;
    	if(a.sw&BCD_NULL) return b;

    	if(a.sw&BCD_MINUS && b.sw&BCD_MINUS || a.sw&BCD_PLUS && b.sw&BCD_PLUS)
    	{	Add(a.val, a.expo, b.val, b.expo);
		}
        else
        {	int r=Sub(a.val, a.expo, b.val, b.expo);
        	if(r<0)
            {	if(a.sw&BCD_PLUS) a.sw=BCD_MINUS;
            	else a.sw=BCD_PLUS;
            }
            else if(r==0) a.sw=BCD_NULL;
        }
    }

    else a.sw=BCD_NAN;

    return a;
}



BCD BCD::operator -(BCD & b)
{	BCD bb(b);
	if(bb.sw&BCD_MINUS) bb.sw=BCD_PLUS;
    else if(bb.sw&BCD_PLUS) bb.sw=BCD_MINUS;

	return *this + bb;
}

BCD BCD::operator *(BCD & b)
{	BCD r(b);

    if(sw&BCD_PLUS && r.sw&BCD_PLUS || sw&BCD_MINUS && r.sw&BCD_MINUS) r.sw=BCD_PLUS;
    else if(sw&BCD_MINUS && r.sw&BCD_PLUS || sw&BCD_PLUS && r.sw&BCD_MINUS) r.sw=BCD_MINUS;

	if(!IsNumber() || !r.IsNumber())
    {	r.sw=BCD_NAN;
    	return r;
    }

    if(sw&BCD_NULL || r.sw&BCD_NULL)
    {	r.sw=BCD_NULL;
    	return r;
    }


    typedef UC lbcd [BCDDIG+2];
    lbcd val09[10];	//	[0] bruges ikke ...
    UC acc [BCDDIG*2];
    int i;

    memset(acc, 0, BcdDig*2);

    val09[1][0]=0;
    val09[1][1]=0;
    memmove(val09[1]+2, val, BcdDig);
    for(i=2; i<10; i++) memmove(val09[i], val09[i-1], BcdDig+2);
    for(i=2; i<10; i++) DigAdd(val09[i], val09[i-1],  BcdDig+2);

    for(i=BcdDig-1; i>=0; i--)
    {	//	acc/=10;
        memmove(acc+1, acc, BcdDig*2-1);
        acc[0]=0;
       	//	acc+=val*d
        if(r.val[i]>0) DigAdd(acc, val09[r.val[i]],  BcdDig+2);
    }

	if(DigIsNull(acc, BcdDig*2))	//	burde være overflødig...
    {	r.sw=BCD_NULL;
    	return r;
    }

    i=expo+r.expo+1;

    if(expo>0 && r.expo>0 && i<=0 || expo<0 && r.expo<0 && i>=0)
    {	r.sw|=BCD_OVF;
    	return r;
    }


    while(*acc==0)
    {	memmove(acc, acc+1, BcdDig*2-1);
    	i--;
    }

	if(DigRound(acc, BcdDig+1))
    {	*acc=1;
    	i++;
    }

    r.expo=i;

    if(r.expo!=i)
    {	r.sw|=BCD_OVF;
    	return r;
    }

    memmove(r.val, acc, BcdDig);
    return r;
}


BCD BCD::operator /(BCD & b)
{	UC dvd[BCDDIG+1], res[BCDDIG+1];
	UC	dvsr1[BCDDIG+1], dvsr2[BCDDIG+1], dvsr3[BCDDIG+1], dvsr4[BCDDIG+1],
    	dvsr5[BCDDIG+1], dvsr6[BCDDIG+1], dvsr7[BCDDIG+1], dvsr8[BCDDIG+1],
    	dvsr9[BCDDIG+1];
	BCD r;
    int i, tx;
    if(!IsNumber() || !b.IsNumber() || b.sw&BCD_NULL)
    {	r.sw=BCD_NAN;
    	return r;
    }


    memmove(dvd+1,val, BcdDig);
    dvd[0]=0;

    memmove(dvsr1+1,b.val, BcdDig);
    dvsr1[0]=0;
	memmove(dvsr2+1,b.val, BcdDig);
    dvsr2[0]=0;
	memmove(dvsr3+1,b.val, BcdDig);
    dvsr3[0]=0;
	memmove(dvsr4+1,b.val, BcdDig);
    dvsr4[0]=0;
	memmove(dvsr5+1,b.val, BcdDig);
    dvsr5[0]=0;
	memmove(dvsr6+1,b.val, BcdDig);
    dvsr6[0]=0;
	memmove(dvsr7+1,b.val, BcdDig);
    dvsr7[0]=0;
	memmove(dvsr8+1,b.val, BcdDig);
    dvsr8[0]=0;
	memmove(dvsr9+1,b.val, BcdDig);
    dvsr9[0]=0;

    DigAdd(dvsr2, dvsr1, BcdDig+1);
    DigAdd(dvsr3, dvsr2, BcdDig+1);
    DigAdd(dvsr4, dvsr3, BcdDig+1);
    DigAdd(dvsr5, dvsr4, BcdDig+1);
    DigAdd(dvsr6, dvsr5, BcdDig+1);
    DigAdd(dvsr7, dvsr6, BcdDig+1);
    DigAdd(dvsr8, dvsr7, BcdDig+1);
    DigAdd(dvsr9, dvsr8, BcdDig+1);


    tx=expo-b.expo;

    if(DigCmp(dvd, dvsr1, BcdDig+1)<0) memmove(dvd, dvd+1, BcdDig);
    else tx++;

    for(i=0; i<=BcdDig; i++)
    {	memmove(res, res+1, BcdDig);	//	res*=10

	    if(DigCmp(dvd, dvsr5, BcdDig+1)>=0)
        {
        	if(DigCmp(dvd, dvsr7, BcdDig+1)>=0)
            {
                if(DigCmp(dvd, dvsr9, BcdDig+1)>=0)	//	A>=b? A-=b, res+=..
                {	res[BcdDig]=9;
                    DigSub(dvd, dvsr9, BcdDig+1);
                }
                else if(DigCmp(dvd, dvsr8, BcdDig+1)>=0)
                {	res[BcdDig]=8;
                    DigSub(dvd, dvsr8, BcdDig+1);
                }
                else
                {	res[BcdDig]=7;
                    DigSub(dvd, dvsr7, BcdDig+1);
                }
            }
            else
            {
            	if(DigCmp(dvd, dvsr6, BcdDig+1)>=0)
                {	res[BcdDig]=6;
                    DigSub(dvd, dvsr6, BcdDig+1);
                }
                else
                {	res[BcdDig]=5;
                    DigSub(dvd, dvsr5, BcdDig+1);
                }
            }
        }
        else
        {
        	if(DigCmp(dvd, dvsr3, BcdDig+1)>=0)
            {
                if(DigCmp(dvd, dvsr4, BcdDig+1)>=0)
                {	res[BcdDig]=4;
                    DigSub(dvd, dvsr4, BcdDig+1);
                }
                else
                {	res[BcdDig]=3;
                    DigSub(dvd, dvsr3, BcdDig+1);
                }
            }
        	else
            {
                if(DigCmp(dvd, dvsr2, BcdDig+1)>=0)
                {	res[BcdDig]=2;
                    DigSub(dvd, dvsr2, BcdDig+1);
                }
                else if(DigCmp(dvd, dvsr1, BcdDig+1)>=0)
                {	res[BcdDig]=1;
                    DigSub(dvd, dvsr1, BcdDig+1);
                }
                else res[BcdDig]=0;
            }
        }

        memmove(dvd, dvd+1, BcdDig);	//	dvd*=10
    }


    if(DigRound(res, BcdDig+1))
    {	tx++;
    	res[0]=1;
    }

    memmove(r.val, res, BcdDig);

    if(sw&BCD_PLUS && b.sw&BCD_PLUS || sw&BCD_MINUS && b.sw&BCD_MINUS) r.sw=BCD_PLUS;
    else if(sw&BCD_MINUS && b.sw&BCD_PLUS || sw&BCD_PLUS && b.sw&BCD_MINUS) r.sw=BCD_MINUS;

    r.expo=tx;
    if(r.expo!=tx) r.sw=BCD_OVF;

    return r;
}



bool BCD::operator ==(BCD & b)
{	if(!IsNumber() || !b.IsNumber()) return false;
	if(sw&BCD_NULL || b.sw&BCD_NULL)
    {	return (sw&BCD_NULL && b.sw&BCD_NULL)? true : false;
    }

    if(sw&BCD_MINUS && b.sw&BCD_PLUS || b.sw&BCD_MINUS && sw&BCD_PLUS) return false;

    if(expo != b.expo) return false;

    for(int i=0; i<BcdDig; i++)
    {	if(val[i] != b.val[i]) return false;
    }

    return true;
}

bool BCD::operator !=(BCD & b)
{	return !(*this == b);
}


bool BCD::operator <(BCD & b)
{	if(!IsNumber() || !b.IsNumber()) return false;
	if(sw&BCD_NULL && b.sw&BCD_NULL) return false;

    if(sw&BCD_NULL) return (b.sw&BCD_PLUS)? true : false;
    if(b.sw&BCD_NULL) return (sw&BCD_MINUS)? true : false;

    if(sw&BCD_MINUS && b.sw&BCD_PLUS) return true;
    if(b.sw&BCD_MINUS && sw&BCD_PLUS) return false;

    bool lt;

    lt=(expo == b.expo) ?
    	((DigCmp(val, b.val, BcdDig)<0)?true:false) :
	    ((expo < b.expo)?true:false) ;

    return (sw&BCD_PLUS)?lt:!lt;
}

bool BCD::operator >=(BCD & b)
{	return !(*this < b);
}

bool BCD::operator >(BCD & b)
{	if(!IsNumber() || !b.IsNumber()) return false;
	if(sw&BCD_NULL && b.sw&BCD_NULL) return false;

    if(sw&BCD_NULL) return (b.sw&BCD_MINUS)? true : false;
    if(b.sw&BCD_NULL) return (sw&BCD_PLUS)? true : false;

    if(sw&BCD_MINUS && b.sw&BCD_PLUS) return false;
    if(b.sw&BCD_MINUS && sw&BCD_PLUS) return true;


    bool lt;

    lt=(expo == b.expo) ?
    	((DigCmp(val, b.val, BcdDig)>0)?true:false) :
	    ((expo > b.expo)?true:false) ;

    return (sw&BCD_PLUS)?lt:!lt;
}

bool BCD::operator <=(BCD & b)
{	return !(*this > b);
}




BCD BCD::RelRound(int dig)
{	BCD a(*this);
	if(dig>0 && dig<BcdDig && IsNumber() && !(sw&BCD_NULL))
	{	if(DigRound(a.val, dig+1))
        {	int tx=a.expo;
        	tx++;
            a.val[0]=1;
            a.expo=tx;
            if(a.expo!=tx) a.sw=BCD_OVF;
            dig=1;
        }

    	for(int i=dig; i<BcdDig; i++) a.val[i]=0;
    }
    return a;
}


BCD BCD::Round(int dig)
{	BCD a(*this);

	if(!IsNumber() || sw&BCD_NULL || expo>=BcdDig || (BcdDig-expo-dig)<=0) return a;

	if(dig>0 && dig<BcdDig && IsNumber() && !(sw&BCD_NULL))
	{	if(DigRound(a.val, a.expo+dig+1))
        {	int tx=a.expo;
        	tx++;
            a.val[0]=1;
            a.expo=tx;
            if(a.expo!=tx) a.sw=BCD_OVF;
            for(int i=1; i<BcdDig; i++) a.val[i]=0;
        }

        else for(int i=dig+a.expo; i<BcdDig; i++) a.val[i]=0;
    }

    return a;
}



BCD BCD::Int(void)
{	BCD a(*this);
	if(!IsNumber() || sw&BCD_NULL || expo>=BcdDig) return a;


    if(a.expo<=0) a.sw=BCD_NULL;
    else for(int i=a.expo; i<BcdDig; i++) a.val[i]=0;
    return a;
}

bool BCD::IntegerOk(void)
{	BCD t(*this);
	if(!t.IsNumber()) return false;

    if(t.sw&BCD_MINUS) t.sw=BCD_PLUS;
    if(t<=IntMax) return true;
    return false;


}

// OBS! der er ingen overflowtest
int BCD::Integer(void)
{	int r=0, i;
	if(IsNumber() && !(sw&BCD_NULL) && expo>0)
    {	for(i=0; i<expo; i++)
    	{	r*=10;
        	r+=val[i];
        }
    }
    return (sw&BCD_PLUS)?r:-r;
}


BCD BCD::Frac(void)
{	BCD a(*this);
	int i;
	if(!IsNumber() || a.sw&BCD_NULL || a.expo<=0) return a;


    if(a.expo>=BcdDig)
    {	a.sw=BCD_NULL;
    	return a;
    }

    memmove(a.val, a.val+a.expo, BcdDig-a.expo);
    for(i=BcdDig-a.expo; i<BcdDig; i++) a.val[i]=0;
    a.expo=0;

    if(DigIsNull(a.val, BcdDig)) a.sw=BCD_NULL;

    return a;
}


BCD BCD::Abs(void)
{	BCD r(*this);
	if(r.IsNumber() && r.sw&BCD_MINUS) r.sw=BCD_PLUS;
    return r;
}

BCD BCD::Neg(void)
{	BCD r(*this);
	if(r.IsNumber())
    {	if(r.sw&BCD_MINUS) r.sw=BCD_PLUS;
    	else if(r.sw&BCD_PLUS) r.sw=BCD_MINUS;
    }
    return r;
}




#include "math.h"

BCD BCD::Sqrt(void)
{	BCD A(*this), t(*this), a;
	t.expo/=2;

    double d=0;
    for(int i=0; i<16; i++)
    {	d+=A.val[15-i];
        d/=10;

    }
    if(A.expo&1)
    {	if(A.expo<0) d/=10;
    	else d*=10;

    }

    d=sqrt(d);

    while(d>=1)
    {	d/=10;
    	t.expo++;
    }
    while(d<0.1)
    {	d*=10;
    	t.expo--;
    }

    for(int i=0; i<16; i++)
    {	d*=10;
    	t.val[i]=(UC)d;
        d-=t.val[i];
    }


    int Cmp=-1, lim=(BcdDig+1)/2;

    while(Cmp < lim)
    {	a=t;
    	t = A/a + a;
	    t = t.MulAE(5, -1);

        Cmp=Eq(t,a);
    }
//    } while (Eq(t,a) < (BcdDig+1)/2);

    t = A/a - a;
    t = t.MulAE(5, -1);
    a=a+t;

    return a;
}

BCD BCD::Pow(int potens)
{	BCD a(*this), b;
	int pot=(potens>0)?potens:-potens;

	if(a.sw&BCD_NULL) return a;

    if(a.IsNumber() && !potens)
    {	BCD en(1);
    	return en;
    }

    while(a.IsNumber() && pot && !(pot&1))
    {	a=a*a;
    	pot>>=1;
    }

    b=a;
    pot>>=1;

    while(b.IsNumber() && pot && a.IsNumber())
    {	a=a*a;
    	if(pot&1) b=b*a;
    	pot>>=1;
    }


    if(potens<0 && b.IsNumber())
    {	BCD en(1);
    	return en/b;
    }

    if(!b.IsNumber() || !a.IsNumber())
    {	b.SetVal(0);
    	b.sw=BCD_OVF;
    }

    return b;
}




BCD BCD::Pow(BCD & potens)
{	if(!potens.IntegerOk())
	{	BCD err(0);
    	err.sw=BCD_OVF;
        return err;
    }

	int n=potens.Integer();
	return Pow(n);
}

BCD BCD::Fakultet(void)
{	BCD acc(2), en(1);
	int i, n;

	if(!IsNumber() || (sw&BCD_MINUS))
    {	acc.sw=BCD_NAN;
    	return acc;
    }

    if(*this > IntMax)
    {	acc.sw=BCD_PLUS|BCD_OVF;
    	return acc;
    }
    if(*this <= en) return en;

    if(!IntegerOk())
	{	BCD err(0);
    	err.sw=BCD_OVF;
        return err;
    }

    n=Integer();

    for(i=3; i<=n && acc.IsNumber(); i++)
    {	acc=acc.MulAE(i);
    }

    return acc;

}


BCD BCD::ExpXm1(void)
{	BCD acc(*this), fak(2), xn, t, r, x(*this);
	int i=3;

    if(sw&BCD_NULL)
    {	acc.SetVal(0);
	    return acc;
    }

	xn = x*x;
    acc = xn / fak + acc;

    do
    {	r=acc;
    	xn = xn * x;
	    t.SetVal(i++);
    	fak = fak * t;
        acc = xn / fak + acc;

    } while(r != acc);

    return acc;

}

//	x >= 0
BCD BCD::ExpFrac(void)
{	BCD nul(0), acc(1), t, a(*this), b;

	if(*this == nul)
    {	acc.SetVal(1);
    	return acc;
    }

    for(int i=0; i<100; i++)
    {	do
    	{	b=a-BCD_ln1_n[i];

        	if(b>=nul)
        	{	a=b;
            	t=acc;
                t.expo-=i+1;
                acc = acc + t;
            }

        } while(b>nul);
    }

    if(a==nul) return acc;

    t=a.ExpXm1();
    b.SetVal(1);
    b=b+t;
    acc = acc * b;

    return acc;
}


void BCD::SinCos(BCD & s, BCD & c)
{	int delta[100], i, pi4=0;
	for(i=0; i<100; i++) delta[i]=0;
    BCD rest(*this), tc, tmp;

    if(rest>=BCD_pi4)
    {	pi4=1;
    	rest=rest-BCD_pi4;
    }

    // Reduce angle usign CORDIC
    for(i=0; i<100; i++)
    {	while(rest >= BCD_atan[i])
    	{	rest=rest-BCD_atan[i];
        	delta[i]++;
        }
    }

    s=rest.TaylorSin();
    c=rest.TaylorCos();

    for(i=100-1; i>=0; i--)
    {	for( ; delta[i]; delta[i]--)
    	{	tc=c;

        	tmp=s;
            tmp.expo-=i+1;
            c=c-tmp;

            tmp=tc;
            tmp.expo-=i+1;
            s=tmp+s;
        }
    }

    if(pi4)
    {	tc=c;
    	c=c-s;
        s=tc+s;
    }
}




//	kun positive tal
bool BCD::ArcSin(BCD & as)
{	BCD s(*this), c(1), ts, t;
	int i;

	if(s>c)
    {	as.sw=BCD_NAN;
    	return false;
    }

    if(s==c)
    {	as=BCD_pi2;
    	return true;
	}


    t=s*s;
    c=c-t;
    c=c.Sqrt();

    as.SetVal(0);

    if(s.sw&BCD_NULL) return true;

    for(i=0; i<100; i++)
    {	t=c;
        t.expo-=i+1;

        while(s>=t)
        {	ts=s;
        	s=s-t;
        	as=as+BCD_atan[i];

            t=ts;
            t.expo-=i+1;
            c=c+t;

            t=c;
            t.expo-=i+1;
        }
    }


    t=s/c;
    t=t.TaylorArcTan();

    as=as+t;


    return true;
}


bool BCD::TrigSCT(BCD & s, BCD & c, BCD & t, int calcflag)
{	BCD k, x(*this), nul(0), en(1);
	int kvadrant=1;

    if(!IsNumber()) return false;

	if(x>BCD_2pi)
    {	k=x/BCD_2pi;
    	k=k.Int();
        if(x.sw&BCD_MINUS) k=k+en;
        k=BCD_2pi * k;
        x=x-k;
    }

    if(x>=BCD_pi)
    {	x=x-BCD_pi;
    	kvadrant+=2;
    }

    if(x>=BCD_pi2)
    {	x=x-BCD_pi2;
    	kvadrant+=1;
    }


    if(x>=BCD_pi2)
    {	t.sw=(short)((kvadrant==1 || kvadrant==3)?BCD_PLUS:BCD_MINUS);
    	t.sw|=BCD_INF;
        switch(kvadrant)
        {	case 1:
        		s.SetVal(1);
                c.SetVal(0);
                break;
        	case 2:
            	s.SetVal(0);
                c.SetVal(-1);
            	break;
            case 3:
            	s.SetVal(-1);
                c.SetVal(0);
                break;
            case 4:
            	s.SetVal(0);
                c.SetVal(1);
                break;
        }
        return true;
	}

    x.SinCos(s,c);


    switch(kvadrant)
    {	case 2:
    		k=s;
            s=c;
            c=nul-k;
            break;
    	case 3:
        	s=nul-s;
            c=nul-c;
            break;
        case 4:
        	k=s;
            s=nul-c;
            c=k;
            break;
    }

	if(calcflag&TRIG_TAN) t=s/c;

	if(calcflag&(TRIG_SIN|TRIG_COS))
    {	k=s*s;
    	k=c*c+k;
    	k=k.Sqrt();
        if(calcflag&TRIG_SIN) s=s/k;
        if(calcflag&TRIG_COS) c=c/k;
    }

    return true;
}



BCD BCD::Exp(void)
{	BCD en(1), t(*this);
	int i;
    bool sgn=false;

    if(!IsNumber())
    {	t.sw=BCD_NAN;
    	return t;
    }

    if(t.sw&BCD_MINUS)
    {	sgn=true;
    	t.sw=BCD_PLUS;
    }

    if(!t.IntegerOk())
	{	BCD err(0);
    	err.sw=BCD_OVF;
        return err;
    }
    i=t.Integer();

    t=t.Frac();

//    t=f.ExpXm1();
//    t=t+en;

	t=t.ExpFrac();

    t = BCD_e.Pow(i) * t;
    if(sgn) t=en/t;

    return t;
}



BCD BCD::Ln1px(void)
{	int i=2;
	BCD dvsr, acc(*this), dvd(*this), t;

    do
    {	dvd=dvd * *this;
	    dvsr.SetVal(i++);
        t=dvd / dvsr;
        acc = acc - t;

        dvd=dvd * *this;
	    dvsr.SetVal(i++);
        t=dvd / dvsr;
        acc = acc + t;

    } while(t.expo >= expo - BcdDig);

    return acc;
}


BCD BCD::LnFrac(void)
{	BCD acc(0), t, a(*this), b, en(1);
	bool brk=false;

    for(int i=1; i<=100; i++)
    {	do
    	{	b = a;
            b.expo -= i;
            t = a + b;
            if(t==a) brk=true;	//	for det tilfælde, at der bruges < 100 cifre i beregningerne

            if(t<=en)
            {	acc = acc - BCD_ln1_n[i-1];
                a = t;
            }

        } while(t<en && !brk);

        if(t == en) return acc;
    }

    if(a < en)
    {	a = a - en;
    	a = a.Ln1px();
        return a + acc;
	}
    return acc;
}

BCD BCD::Ln(void)
{	BCD n, a(*this);
	if(!(IsNumber() && (sw&BCD_PLUS)))
    {	a.sw=BCD_NAN;
    	return a;
    }
    n.SetVal(expo);
    n = n * BCD_ln10;
    a.expo=0;
    a = a.LnFrac();
    return a+n;
}

void BCD::SetVal(int b)
{	char buf[100];
	int i;

	if(b<0)
	{	sw=BCD_MINUS;
    	b=-b;
    }
    else if(b>0) sw=BCD_PLUS;
	else sw=BCD_NULL;

	for(i=0; i<BcdDig; i++) val[i]=0;

    sprintf(buf, "%d", b);
    for(i=0; buf[i]!='\0'; i++) val[i]=(UC)(buf[i]&0x0F);
    expo=i;
}


BCD BCD::Sin(void)
{	BCD s, c, t;
    TrigSCT(s,c,t, TRIG_SIN);
    return s;
}

BCD BCD::Cos(void)
{	BCD s, c, t;
    TrigSCT(s,c,t, TRIG_COS);
    return c;
}

BCD BCD::Tan(void)
{	BCD s, c, t;
    TrigSCT(s,c,t, TRIG_TAN);
    return t;
}

BCD BCD::ASin(void)
{	BCD r(*this);
	bool sgn=false;

	if(r.IsNumber())
    {	if(r.sw&BCD_MINUS)
    	{	r.sw=BCD_PLUS;
        	sgn=true;
        }

		if(r.ArcSin(r))
        {	if(sgn)
        	{	if(r.sw&BCD_PLUS) r.sw=BCD_MINUS;
            	else if(r.sw&BCD_MINUS) r.sw=BCD_PLUS;
            }
        }
    }

    return r;
}

BCD BCD::ACos(void)
{	BCD r;
	r=ASin();
    if(IsNumber()) r=BCD_pi2-r;
    return r;
}

BCD BCD::ATan(void)
{	BCD r(*this), en(1), t;
	bool sgn=false;

	if(r.IsNumber())
    {	if(r.sw&BCD_MINUS)
    	{	r.sw=BCD_PLUS;
        	sgn=true;
        }

        r=r*r;
        en=en+r;
        r=r/en;
        r=r.Sqrt();

		if(r.ArcSin(r))
        {	if(sgn)
        	{	if(r.sw&BCD_PLUS) r.sw=BCD_MINUS;
            	else if(r.sw&BCD_MINUS) r.sw=BCD_PLUS;
            }
        }
    }

    return r;
}







BCD BCD::TaylorSin(void)
{	BCD x(*this), xn, x2, acc(*this), fak(6), t, tt;
	int i=4;
	x2=x*x;
	xn=x2*x;

    do
    {	t=acc;

    	tt=xn/fak;
    	acc=acc-tt;
        fak=fak.MulAE(i++);
        fak=fak.MulAE(i++);
        xn=xn*x2;

        acc=xn/fak + acc;
    	fak=fak.MulAE(i++);
        fak=fak.MulAE(i++);
        xn=xn*x2;

	} while(t!=acc);

    return acc;
}


/*
sin(x) = -sin(x-pi) = sin(pi-x) = cos(pi/2-x)
            x>pi        x>pi/2      x>pi/4
Reducér x til [0;pi/4] og beregn Taylor sin hhv cos

Cos(x) = sin(x+pi/2)
*/


BCD BCD::SlowSin(void)
{	bool sgn=false;
	BCD x(*this), k;
    int i;

    if(!x.IsNumber())
    {	x.sw=BCD_NAN;
    	return x;
    }

    if(x.sw&BCD_MINUS)
    {	x.sw=BCD_PLUS;
    	sgn=!sgn;
    }


    if(x>BCD_2pi)
    {	k=x/BCD_2pi;
        if(!k.IntegerOk())
        {	BCD err(0);
            err.sw=BCD_OVF;
            return err;
        }
    	i=k.Integer();
        k=BCD_2pi.MulAE(i);
        x=x-k;
    }

    if(x>BCD_pi)
    {	x=x-BCD_pi;
    	sgn=!sgn;
    }

    if(x>BCD_pi2)
    {	x=BCD_pi-x;
	}

    if(x>BCD_pi4)
    {	x=BCD_pi2-x;
    	x=x.TaylorCos();
    }
    else x=x.TaylorSin();

    if(sgn)
    {	if(x.sw&BCD_MINUS) x.sw=BCD_PLUS;
    	else if(x.sw&BCD_PLUS) x.sw=BCD_MINUS;
    }
    return x;
}




BCD BCD::TaylorCos(void)
{	BCD x(*this), xn, x2, acc(1), fak(2), t, tt;
	int i=3;
	x2=x*x;
	xn=x2;

    do
    {	t=acc;

    	tt=xn/fak;
    	acc=acc-tt;
        fak=fak.MulAE(i++);
        fak=fak.MulAE(i++);
        xn=xn*x2;

        acc=xn/fak + acc;
    	fak=fak.MulAE(i++);
        fak=fak.MulAE(i++);
        xn=xn*x2;

	} while(t!=acc);

    return acc;
}


BCD BCD::TaylorArcTan(void)
{	BCD xn(*this), x2(*this), acc(*this), fak(3), t, tt, to(2);
	x2=x2*x2;
	xn=x2*xn;

    do
    {	t=acc;

    	tt=xn/fak;
    	acc=acc-tt;
        fak=fak+to;
        xn=xn*x2;

        acc=xn/fak + acc;
        fak=fak+to;
		xn=xn*x2;

	} while(t!=acc);

    return acc;
}

BCD BCD::SlowCos(void)
{	BCD t=*this+BCD_pi2;
	return t.Sin();
}


BCD::BCD(int b)
{	SetVal(b);
//	puts("CI");

}

BCD::BCD(char * cp)
{	SetVal(cp);
//	puts("Chr");
}

BCD::BCD(BCD & b)
{	sw=b.sw;
	expo=b.expo;
    memmove(val, b.val, BcdDig);
//    puts("Cpy");
}

BCD::BCD(void)
{	sw=BCD_NULL;
//	puts("Con");
}

BCD::~BCD(void)
{//	puts("Des");
}


#define BCDSTACK 100
class BcdStack
{	private:
		int sp;
        BCD stack[BCDSTACK];

	protected:

    public:
    	bool Push(BCD & a);
        bool Pop(BCD & a);

        bool GetEntry(int idx, BCD & a);	// idx==0 --> TOS

        bool Dup(void);
        bool DupN(int n);
        bool Drop(void);
        bool DropN(int n);
        void ClSt(void) {sp=0;}
        bool Swap(void);
        bool Rot(void);
        bool Over(void);
        bool Dover(void);

        int Count(void){ return sp;}
        int Free(void){ return BCDSTACK-sp;}

    	BcdStack(void);
        ~BcdStack(void);
};


bool BcdStack::Push(BCD & a)
{	if(sp<BCDSTACK)
	{	stack[sp++]=a;
    	return true;
    }

    return false;
}

bool BcdStack::Pop(BCD & a)
{	if(sp>0)
	{	a=stack[--sp];
    	return true;
    }

    return false;
}


bool BcdStack::GetEntry(int idx, BCD & a)
{	if(idx<0 || idx>=sp) return false;
	a=stack[sp-1-idx];
    return true;
}


bool BcdStack::Dup(void)
{	return DupN(0);
}

bool BcdStack::DupN(int n)
{	n=sp-1-n;
	if(n<0) return false;
    return Push(stack[n]);
}

bool BcdStack::Drop(void)
{	return DropN(1);
}

bool BcdStack::DropN(int n)
{	int i=min(sp,n);
	sp-=i;
    return (i<n) ? false : true;
}

bool BcdStack::Swap(void)
{	if(sp<2) return false;
	BCD x, y;
    Pop(x);
    Pop(y);
    Push(x);
    Push(y);
    return true;
}


bool BcdStack::Rot(void)
{	if(sp<3) return false;
	BCD x, y, z;
    Pop(x);
    Pop(y);
    Pop(z);
    Push(y);
    Push(x);
    Push(z);
    return true;
}

bool BcdStack::Over(void)
{	if(sp<2 || Free()<2) return false;
	BCD x, y;
    Pop(x);
    Pop(y);
    Push(y);
	Push(x);
    Push(y);
    return true;
}

bool BcdStack::Dover(void)
{	if(sp<2 || Free()<2) return false;
	BCD x, y;
    Pop(x);
    Pop(y);
    Push(y);
	Push(x);
    Push(y);
	Push(x);
    return true;
}


BcdStack::BcdStack(void)
{	sp=0;
}

BcdStack::~BcdStack(void)
{
}




enum
{	CALC_NOERR, CALC_TEXTLEN, CALC_KENDERIKKE, CALC_STACKOVF, CALC_STACKNDF,
	CALC_STACKLIM,
	CALC_DOMERR
};

char * CalcErrText[]=
{	"Ok",
	"Linien er for lang",
    "Kan ikke genkende udtrykket",
    "Ikke mere plads på stakken",
    "For få tal på stakken",
    "DU har et problem med stakken - JEG gad ikke undersøge DIT problem",
    "Domain error",
};



// funktionsnavne (i CalcFuncName) skal være mindre end FUNCLEN
#define FUNCLEN 100

enum {CALC_SIN, CALC_COS, CALC_TAN, CALC_ASIN, CALC_ACOS, CALC_ATAN,
CALC_LN, CALC_EXP, CALC_SQRT, CALC_POW, CALC_XIY, CALC_FAK,
CALC_ADD, CALC_SUB, CALC_MUL, CALC_DIV, CALC_FACTORIAL,
CALC_DUP, CALC_DROP, CALC_SWAP, CALC_CLST, CALC_SPF, CALC_FXD, CALC_PI,
CALC_HELP, CALC_QUIT, CALC_OVER, CALC_ROT, CALC_AMO, CALC_DOVER,  
CALC_FUNCCNT
};	//CALC_FUNCCNT er antal funktionsnavne i listen - skal altid stå til sidst



typedef struct tagCalcFuncInfo
{	int func;
	char * name;
	int args;		//	antal argumenter til funkktionerne
} CalcFuncInfo;


CalcFuncInfo CalcFuncInf[] =
{	{CALC_SIN, "sin", 1},	{CALC_COS, "cos", 1},	{CALC_TAN, "tan", 1},
    {CALC_ASIN, "asin", 1},	{CALC_ACOS, "acos", 1},	{CALC_ATAN, "atan", 1},
    {CALC_LN, "ln", 1},		{CALC_EXP, "exp", 1},	{CALC_SQRT, "sqrt", 1},
    {CALC_POW, "pow", 2},	{CALC_XIY, "xiy", 2},	{CALC_FAK, "fak", 1},
    {CALC_ADD, "+", 2},		{CALC_SUB, "-", 2},		{CALC_MUL, "*", 2},
    {CALC_DIV, "/", 2},		{CALC_FACTORIAL, "!", 1},{CALC_DUP, "dup", 0},
    {CALC_DROP, "drop", 0},	{CALC_SWAP, "swap", 0},	{CALC_CLST, "clst", 0},
    {CALC_SPF, "setprint",1}, {CALC_FXD, "cifre",1}, {CALC_PI, "pi", 0},
    {CALC_HELP, "?",0}, {CALC_QUIT, "quit",0}, {CALC_OVER, "over",0},
    {CALC_ROT, "rot",0}, {CALC_AMO, "amort", 2}, {CALC_OVER, "dover",0}
};


void DispHelp(void)
{	char ** txt, * cp;
	int i, s, j;
    bool swp=true;
	txt=new char * [CALC_FUNCCNT];

    for(i=0; i<CALC_FUNCCNT; i++) txt[i]=CalcFuncInf[i].name;

    for(i=CALC_FUNCCNT-1; i>0 && swp; i--)
    {	swp=false;
	    for(j=0; j<i; j++)
    	{
        	if(strcmp(txt[j], txt[j+1]) > 0)
        	{	swp=true;
            	cp=txt[j];
                txt[j]=txt[j+1];
                txt[j+1]=cp;
            }
        }
    }


    s=0;
    for(i=0; i<CALC_FUNCCNT; i++)
    {	if(s+strlen(txt[i]) < 78)
    	{	printf("%s ", txt[i]);
	    	s+=strlen(txt[i])+1;
        }

	    else
    	{	printf("\n%s ", txt[i]);
	    	s=strlen(txt[i])+1;
        }
    }
    puts("");

    delete [] txt;

}



class Calculator
{	private:
		BcdStack Stack;
        //BCD a, b;
        int WS(char * cp);
        int Num(char * cp);
        int Func(char * cp, int & fn);
        bool ExecFunc(int fn);

        int ErrNo;
        int CifreEfterKomma;
        bool UseFixPrint;
        unsigned PrintFlag;
        bool DoDispHelp;


	protected:
    public:
	    bool Done;

    	bool Calc(char * xpr);
        bool InError(void) { return (ErrNo==CALC_NOERR) ? false : true; }
        void Disp(void);
        void Init(void);
        void SetCifreEfterKomma(int d);
	    Calculator(int digs);
        ~Calculator(void);
};

int Calculator::WS(char * cp)
{	int len=0;
	while(cp[len]==' ' || cp[len]=='\t') len++;
    return len;
}

int Calculator::Num(char * cp)
{	char *org=cp;
	bool dig=false;

	while(IsDig(cp) || *cp=='.')
    {	if(IsDig(cp)) dig=true;
    	cp++;
    }

    if(*cp==',') cp++;

    while(IsDig(cp))
    {	dig=true;
    	cp++;
    }

    if(*cp =='e' || *cp =='E')
    {	bool edig=false;
    	cp++;
    	if(*cp =='+' || *cp =='-') cp++;
    	while(IsDig(cp))
        {	cp++;
        	edig=true;
        }

        //	det ku' være at, brugeren har tastet '1exp' for at beregne exp(1)...
        if(!edig) while(!(*cp =='e' || *cp =='E')) cp--;
        else dig=dig&&edig;
    }

    return (dig)?cp-org : 0;
}



int Calculator::Func(char * cp, int & fn)
{	char name[FUNCLEN];
	int len=0;

	if(IsLU(cp))
    {   while(IsLDU(cp) && len<FUNCLEN-1) name[len++]=*(cp++);
		name[len]='\0';
        CharLower(name);
    }
    else
    {	name[0]=*cp;
	    name[1]='\0';
        len=1;
    }

    for(fn=0; fn<CALC_FUNCCNT; fn++)
    {	if(strcmp(name, CalcFuncInf[fn].name) == 0) return len;
    }

    return 0;
}



bool Calculator::ExecFunc(int fn)
{	bool OK=false;
	BCD r, a, b, x, y;
    int i;

    if(fn<0 || fn>=CALC_FUNCCNT) return false; //	burde være fanget før ExecFunc

    if(CalcFuncInf[fn].args>0)
    {	if(!Stack.Pop(a))
        {	ErrNo=CALC_STACKNDF;
            return false;
        }

        if(CalcFuncInf[fn].args==2)
        {	if(!Stack.Pop(b))
            {	if(fn==CALC_ADD || fn==CALC_SUB)
                {	b.SetVal(0);
                }
                else
                {	ErrNo=CALC_STACKNDF;
                    Stack.Push(a);
                    return false;
                }
            }
        }
    }

	switch(fn)
    {	case CALC_SIN:
    		r=a.Sin();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

    	case CALC_COS:
    		r=a.Cos();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_TAN:
    		r=a.Tan();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_ASIN:
    		r=a.ASin();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_ACOS:
    		r=a.ACos();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_ATAN:
    		r=a.ATan();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_LN:
    		r=a.Ln();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_EXP:
    		r=a.Exp();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_SQRT:
        	r=a.Sqrt();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_POW:
        	r=b.Pow(a);
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_XIY:
        	r=b.Ln();
            r=r*a;
            r=r.Exp();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_FAK:
        	r=a.Fakultet();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_ADD:
        	r = b + a;
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_SUB:
        	r = b - a;
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_MUL:
        	r = b * a;
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_DIV:
        	r = b / a;
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_FACTORIAL:
        	r=a.Fakultet();
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_DUP:
        	if(Stack.Dup()) return true;
			else
            {	ErrNo=CALC_STACKLIM;
	            return false;
            }
            //break;

        case CALC_DROP:
        	if(Stack.Drop()) return true;
			else
            {	ErrNo=CALC_STACKNDF;
	            return false;
            }
            //break;

        case CALC_SWAP:
        	if(Stack.Swap()) return true;
			else
            {	ErrNo=CALC_STACKNDF;
	            return false;
            }
            //break;

        case CALC_CLST:
        	Stack.ClSt();
            return true;
            //break;

        case CALC_SPF:
            i=a.Integer();
        	UseFixPrint=(i&BCD_TEXT_UFP)?true:false;
            PrintFlag=i&~BCD_TEXT_UFP;
            return true;
            //break;

        case CALC_FXD:
	        i=a.Integer();
            SetCifreEfterKomma(i);
            return true;
            //break;

        case CALC_PI:
        	r=BCD_pi;
        	OK=true;
            break;

        case CALC_HELP:
        	DoDispHelp=true;
            return true;
            //break;

        case CALC_QUIT:
        	Done=true;
            return true;

        case CALC_OVER:
        	if(Stack.Over()) return true;
			else
            {	ErrNo=CALC_STACKNDF;
	            return false;
            }
            //break;

        case CALC_ROT:
	        if(Stack.Rot()) return true;
			else
            {	ErrNo=CALC_STACKNDF;
	            return false;
            }
            //break;

        case CALC_AMO:
        	y.SetVal(1);
            r=y+b;
            r=r.Pow(a);
            x=r;
            r=r*b;
            x=x-y;
            r=r/x;
            if(r.IsNumber()) OK=true;
            else ErrNo=CALC_DOMERR;
            break;

        case CALC_DOVER:
        	if(Stack.Dover()) return true;
			else
            {	ErrNo=CALC_STACKNDF;
	            return false;
            }
            //break;



    }

    if(OK)		//	OK res --> stack
    {	if(PrintFlag&BCD_TEXT_RND) r=r.Round(CifreEfterKomma);
		Stack.Push(r);
	}
    else		//	ellers genopret stack
    {	if(CalcFuncInf[fn].args==2) Stack.Push(b);
    	Stack.Push(a);
    }

    return OK;
}


#define CALCBUF 1024
bool Calculator::Calc(char * xpr)
{	char buf[CALCBUF];
	int len, fn;
    BCD a;

    do
    {	xpr+=WS(xpr);

        len=Num(xpr);
        if(len>0)
        {	if(len>=CALCBUF-1)
            {	ErrNo=CALC_TEXTLEN;
                return false;
            }
            memmove(buf, xpr, len);
            buf[len]='\0';
            xpr+=len;
            a.SetVal(buf);
            if(!Stack.Push(a))
            {	ErrNo=CALC_STACKOVF;
                return false;
            }
        	continue;
        }

        len=Func(xpr, fn);
        if(len>0)
        {	if(!ExecFunc(fn)) return false;
	        xpr+=len;
	        continue;
        }

    } while(*xpr != '\0' && len>0);

    if(*xpr != '\0')
    {	ErrNo=CALC_KENDERIKKE;
    	return false;
    }


    return true;
}


void Calculator::Disp(void)
{	if(Stack.Count()>0)
	{	BCD r;
    	int idx=Stack.Count();
        for(idx--; idx>=0; idx--)
        {	Stack.GetEntry(idx,r);
        	//r=r.RelRound(50);
	        printf("[%3d] ",idx);
            if(UseFixPrint)
            {	r=r.RelRound(DispDig);
            	r.FixPrint(CifreEfterKomma, PrintFlag);
            }

            else r.Print(PrintFlag);
        }
    }

    if(ErrNo==CALC_NOERR && Stack.Count()==0)
    {	puts("      (stakken er tom)");
    }

    if(ErrNo!=CALC_NOERR)
    {	puts(CalcErrText[ErrNo]);
    }

	if(DoDispHelp)
    {	DispHelp();
    	DoDispHelp=false;
    }
}

void Calculator::Init(void)
{	ErrNo=CALC_NOERR;
}

void Calculator::SetCifreEfterKomma(int d)
{	CifreEfterKomma= (d>=0 && d<BcdDig)? d : 5;
}

Calculator::Calculator(int digs)
{	BCD::KonstInit(digs);
	Init();
    SetCifreEfterKomma(10);
	PrintFlag=BCD_TEXT_SEP;
	UseFixPrint=true;
    DispDig=50;
    Done=false;
	DoDispHelp=false;
}


Calculator::~Calculator(void)
{
}


#include <conio.h>


void main(void)
{
	Calculator calc(75);

    char buf[1000];

    while(!calc.Done)
    {	calc.Init();
	    printf(": ");
	    gets(buf);
        if(*buf)
        {	/*bool OK=*/calc.Calc(buf);
        	if(!calc.Done)
            {	clrscr();
            	calc.Disp();
                //if(!OK && !calc.InError()) puts("Operandfejl");
            }
        }
    }


	//printf("Press Enter ");
    //getchar();

}




