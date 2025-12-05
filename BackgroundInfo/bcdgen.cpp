#define BCD_TEST

#include "stdio.h"
#include "string.h"

template <class T> void swap(T & a, T & b)
{	T c;
	c=a;
    a=b;
    b=c;
}

typedef unsigned char UC;


#ifdef BCD_TEST
	#define BCDDIG   3000
	#define BCDROUND 1500
#else
	#define BCDDIG   400
	#define BCDROUND 200
#endif

#define BCD_NULL  0x0001
#define BCD_PLUS  0x0002
#define BCD_MINUS 0x0004
#define BCD_INF   0x0008
#define BCD_NAN   0x0010
#define BCD_OVF   0x0020

#define BCD_NTZ	0x0001


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

        bool isNumber(void);

	protected:
        short sw;		//	kombinationer af BCD_MINUS|BCD_INF|BCD_NAN
        UC val[BCDDIG];	//	0.dddddd....
        int expo;

    public:
    	bool ToText(char * cp, unsigned flag=0);
        bool FromText(char * cp);

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

        BCD RelRound(int dig);
        BCD Round(int dig);
        BCD Int(void);
		int Integer(void);
        BCD Frac(void);

        BCD Pow(int potens);
        BCD Ln1px(void);

        int Expo(void) { return expo; }

        BCD(int b);
        BCD(char * cp);
        BCD(BCD & b);
    	BCD(void);
        ~BCD(void);


        BCD atan(void);
        BCD atan1(void);
};


BCD BCD::atan(void)
{	BCD x(*this), x2, xn(*this), k, f, acc(*this), t;
	int i=3;
	x2=x*x;
    xn=xn*x2;


    do
    {	f=acc;
	    k.SetVal(i);
    	t=xn/k;
        acc=acc-t;
        i+=2;
        xn=xn*x2;

        k.SetVal(i);
    	t=xn/k;
        acc=acc+t;
        i+=2;
        xn=xn*x2;


        printf("%8d\r", i);


    } while( f!=acc);

    return acc;
}



BCD BCD::atan1(void)
{	BCD x(1), k(1), f, acc(1), t, to(2);
	int i=0;

    do
    {	f=acc;
	    k=k+to;
    	t=x/k;
        acc=acc-t;
        i+=2;

        k=k+to;
    	t=x/k;
        acc=acc+t;
        i+=2;

        printf("%8d\r", ++i);


    } while( f!=acc);

    return acc;
}



void BCD::Add(UC *v1, int & x1, UC *v2, int x2)
{	UC vt1[BCDDIG+2], vt2[BCDDIG+2];

	if(x1 > x2+BCDDIG) return;
	if(x2 > x1+BCDDIG)
    {	x1=x2;
    	memmove(v1, v2, BCDDIG);
        return;
    }


    memset(vt1, 0, BCDDIG+2);
    memset(vt2, 0, BCDDIG+2);
    int xd=x1-x2;


    if(xd>=0)
    {	memmove(vt1+1, v1, BCDDIG);
    	if(xd>0) memmove(vt2+1+xd, v2, BCDDIG+1-xd);
        else memmove(vt2+1+xd, v2, BCDDIG);
    }
    else
    {	xd=-xd;
    	memmove(vt1+1, v2, BCDDIG);
    	memmove(vt2+1+xd, v1, BCDDIG+1-xd);
        x1=x2;
    }

    vt1[0]=(UC)DigAdd(vt1+1, vt2+1, BCDDIG+1);
    DigRound(vt1, BCDDIG+2);

    if(*vt1)
    {	x1++;
	    memmove(v1, vt1, BCDDIG);
    }
    else memmove(v1, vt1+1, BCDDIG);
}

//	<0 chg sign, ==0 BCD_NULL, >0 Ok
int BCD::Sub(UC *v1, int & x1, UC *v2, int x2)
{	UC vt1[BCDDIG+1], vt2[BCDDIG+1];

	if(x1 > x2+BCDDIG) return +1;
	if(x2 > x1+BCDDIG)
    {	x1=x2;
    	memmove(v1, v2, BCDDIG);
        return -1;
    }


    memset(vt1, 0, BCDDIG+1);
    memset(vt2, 0, BCDDIG+1);
    int xd=x1-x2, cs=+1;

    if(xd==0) cs=DigCmp(v1, v2, BCDDIG);

    if(xd>=0 && cs>=0)
    {	memmove(vt1, v1, BCDDIG);
    	if(xd>0) memmove(vt2+xd, v2, BCDDIG+1-xd);
        else memmove(vt2+xd, v2, BCDDIG);
    }
    else
    {	xd=-xd;
    	memmove(vt1, v2, BCDDIG);
    	if(xd>0) memmove(vt2+xd, v1, BCDDIG+1-xd);
        else memmove(vt2+xd, v1, BCDDIG);
        x1=x2;
        cs=-1;
    }

	DigSub(vt1, vt2, BCDDIG+1);
    DigRound(vt1, BCDDIG+1);
    vt1[BCDDIG]=0;

    if(DigIsNull(vt1, BCDDIG))	return 0;

    while(*vt1 == 0)
    {	x1--;
    	memmove(vt1, vt1+1, BCDDIG);
        vt1[BCDDIG]=0;
    }

    memmove(v1, vt1, BCDDIG);

    return cs;
}


//	<0 v1<v2    ==0 v1==v2   >0 v1>v2
/*
int BCD::DigCmp(UC *v1, UC *v2, int dig)
{	while(dig--)
	{	if(*v1 != *v2) return ((int)(*v1) - (int)(*v2));
        v1++;
        v2++;
    }
    return 0;
}
*/

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



bool BCD::DigIsNull(UC * v1, int dig)
{	while(dig--) if(*(v1++)) return false;
	return true;
}




inline bool BCD::isNumber(void)
{	return (sw&BCD_MINUS || sw&BCD_PLUS || sw&BCD_NULL)? true : false;
}


bool BCD::ToText(char * cp, unsigned flag)
{	if(sw&(BCD_INF|BCD_NAN))
	{	if(sw&BCD_NAN) strcpy(cp, "BCD_NAN");
    	else
        {	if(sw&BCD_MINUS) strcpy(cp, "-Inf");
        	else strcpy(cp, "+Inf");
		}
	    return false;
    }
    if(sw&BCD_MINUS) *(cp++)='-';
    else *(cp++)='+';

    if(sw&BCD_NULL)
    {	memset(val, 0, BCDDIG);
    	expo=0;
    }

    UC Val[BCDDIG];
    int tx=expo;
    memmove(Val, val, BCDDIG);
    if(DigRound(Val, BCDDIG))
    {	Val[0]=1;
    	tx++;
    }

    *(cp++)=(UC)((Val[0]&0x0F)+'0');
    *(cp++)=',';

    for(int d=1; d<BCDDIG-1; d++)
    {	*(cp++)=(UC)((Val[d]&0x0F)+'0');
    }

    if(flag&BCD_NTZ)
    {	cp--;
    	while(*cp=='0') cp--;
    	if(*cp==',') cp--;
        cp++;
    }

    *(cp++)='E';
    sprintf(cp,"%+05d", tx-1);
    return true;

}

//	[+|-] [d...] , [d...] [e[+|-]d...]
// false hvis der er ukonverterede tegn
bool BCD::FromText(char * cp)
{	int tx=0, x=0, idx=0, se=+1;
	bool komma=false, round;

	memset(val,0, BCDDIG);
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

    while(*cp && idx<BCDDIG)
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
    {	if(DigInc(val, BCDDIG))
    	{	tx++;
        	val[0]=1;
        }
    }

    expo=tx;
    if(expo!=tx) sw|=BCD_OVF;

    return (*cp)?false:true;
}

BCD BCD::operator +(BCD & b)
{	BCD a(*this);

	if(a.isNumber() && b.isNumber())
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

	if(!isNumber() || !r.isNumber())
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

    memset(acc, 0, BCDDIG*2);

    val09[1][0]=0;
    val09[1][1]=0;
    memmove(val09[1]+2, val, BCDDIG);
    for(i=2; i<10; i++) memmove(val09[i], val09[i-1], BCDDIG+2);
    for(i=2; i<10; i++) DigAdd(val09[i], val09[i-1],  BCDDIG+2);

    for(i=BCDDIG-1; i>=0; i--)
    {	//	acc/=10;
        memmove(acc+1, acc, BCDDIG*2-1);
        acc[0]=0;
       	//	acc+=val*d
        if(r.val[i]>0) DigAdd(acc, val09[r.val[i]],  BCDDIG+2);
    }

	if(DigIsNull(acc, BCDDIG*2))	//	burde være overflødig...
    {	r.sw=BCD_NULL;
    	return r;
    }

    i=expo+r.expo+1;

    while(*acc==0)
    {	memmove(acc, acc+1, BCDDIG*2-1);
    	i--;
    }

	if(DigRound(acc, BCDDIG+1))
    {	*acc=1;
    	i++;
    }

    r.expo=i;

    if(r.expo!=i)
    {	r.sw|=BCD_OVF;
    	return r;
    }

    memmove(r.val, acc, BCDDIG);
    return r;
}


BCD BCD::operator /(BCD & b)
{	UC dvd[BCDDIG+1], res[BCDDIG+1];
	UC	dvsr1[BCDDIG+1], dvsr2[BCDDIG+1], dvsr3[BCDDIG+1], dvsr4[BCDDIG+1],
    	dvsr5[BCDDIG+1], dvsr6[BCDDIG+1], dvsr7[BCDDIG+1], dvsr8[BCDDIG+1],
    	dvsr9[BCDDIG+1];
	BCD r;
    int i, tx;
    if(!isNumber() || !b.isNumber() || b.sw&BCD_NULL)
    {	r.sw=BCD_NAN;
    	return r;
    }


    memmove(dvd+1,val, BCDDIG);
    dvd[0]=0;

    memmove(dvsr1+1,b.val, BCDDIG);
    dvsr1[0]=0;
	memmove(dvsr2+1,b.val, BCDDIG);
    dvsr2[0]=0;
	memmove(dvsr3+1,b.val, BCDDIG);
    dvsr3[0]=0;
	memmove(dvsr4+1,b.val, BCDDIG);
    dvsr4[0]=0;
	memmove(dvsr5+1,b.val, BCDDIG);
    dvsr5[0]=0;
	memmove(dvsr6+1,b.val, BCDDIG);
    dvsr6[0]=0;
	memmove(dvsr7+1,b.val, BCDDIG);
    dvsr7[0]=0;
	memmove(dvsr8+1,b.val, BCDDIG);
    dvsr8[0]=0;
	memmove(dvsr9+1,b.val, BCDDIG);
    dvsr9[0]=0;

    DigAdd(dvsr2, dvsr1, BCDDIG+1);
    DigAdd(dvsr3, dvsr2, BCDDIG+1);
    DigAdd(dvsr4, dvsr3, BCDDIG+1);
    DigAdd(dvsr5, dvsr4, BCDDIG+1);
    DigAdd(dvsr6, dvsr5, BCDDIG+1);
    DigAdd(dvsr7, dvsr6, BCDDIG+1);
    DigAdd(dvsr8, dvsr7, BCDDIG+1);
    DigAdd(dvsr9, dvsr8, BCDDIG+1);


    tx=expo-b.expo;

    if(DigCmp(dvd, dvsr1, BCDDIG+1)<=0) memmove(dvd, dvd+1, BCDDIG);
    else tx++;

    for(i=0; i<=BCDDIG; i++)
    {	memmove(res, res+1, BCDDIG);	//	res*=10

	    if(DigCmp(dvd, dvsr5, BCDDIG+1)>=0)
        {
        	if(DigCmp(dvd, dvsr7, BCDDIG+1)>=0)
            {
                if(DigCmp(dvd, dvsr9, BCDDIG+1)>=0)	//	A>=b? A-=b, res+=..
                {	res[BCDDIG]=9;
                    DigSub(dvd, dvsr9, BCDDIG+1);
                }
                else if(DigCmp(dvd, dvsr8, BCDDIG+1)>=0)
                {	res[BCDDIG]=8;
                    DigSub(dvd, dvsr8, BCDDIG+1);
                }
                else
                {	res[BCDDIG]=7;
                    DigSub(dvd, dvsr7, BCDDIG+1);
                }
            }
            else
            {
            	if(DigCmp(dvd, dvsr6, BCDDIG+1)>=0)
                {	res[BCDDIG]=6;
                    DigSub(dvd, dvsr6, BCDDIG+1);
                }
                else
                {	res[BCDDIG]=5;
                    DigSub(dvd, dvsr5, BCDDIG+1);
                }
            }
        }
        else
        {
        	if(DigCmp(dvd, dvsr3, BCDDIG+1)>=0)
            {
                if(DigCmp(dvd, dvsr4, BCDDIG+1)>=0)
                {	res[BCDDIG]=4;
                    DigSub(dvd, dvsr4, BCDDIG+1);
                }
                else
                {	res[BCDDIG]=3;
                    DigSub(dvd, dvsr3, BCDDIG+1);
                }
            }
        	else
            {
                if(DigCmp(dvd, dvsr2, BCDDIG+1)>=0)
                {	res[BCDDIG]=2;
                    DigSub(dvd, dvsr2, BCDDIG+1);
                }
                else if(DigCmp(dvd, dvsr1, BCDDIG+1)>=0)
                {	res[BCDDIG]=1;
                    DigSub(dvd, dvsr1, BCDDIG+1);
                }
                else res[BCDDIG]=0;
            }
        }

        memmove(dvd, dvd+1, BCDDIG);	//	dvd*=10
    }


    if(DigRound(res, BCDDIG+1))
    {	tx++;
    	res[0]=1;
    }

    memmove(r.val, res, BCDDIG);

    if(sw&BCD_PLUS && b.sw&BCD_PLUS || sw&BCD_MINUS && b.sw&BCD_MINUS) r.sw=BCD_PLUS;
    else if(sw&BCD_MINUS && b.sw&BCD_PLUS || sw&BCD_PLUS && b.sw&BCD_MINUS) r.sw=BCD_MINUS;

    r.expo=tx;
    if(r.expo!=tx) r.sw=BCD_OVF;

    return r;
}



bool BCD::operator ==(BCD & b)
{	if(!isNumber() || !b.isNumber()) return false;
	if(sw&BCD_NULL || b.sw&BCD_NULL)
    {	return (sw&BCD_NULL && b.sw&BCD_NULL)? true : false;
    }

    if(sw&BCD_MINUS && b.sw&BCD_PLUS || b.sw&BCD_MINUS && sw&BCD_PLUS) return false;

    if(expo != b.expo) return false;

    for(int i=0; i<BCDDIG; i++)
    {	if(val[i] != b.val[i]) return false;
    }

    return true;
}

bool BCD::operator !=(BCD & b)
{	return !(*this == b);
}


bool BCD::operator <(BCD & b)
{	if(!isNumber() || !b.isNumber()) return false;
	if(sw&BCD_NULL && b.sw&BCD_NULL) return false;

    if(sw&BCD_NULL) return (b.sw&BCD_PLUS)? true : false;
    if(b.sw&BCD_NULL) return (sw&BCD_MINUS)? true : false;

    bool lt;

    lt=(expo == b.expo) ?
    	((DigCmp(val, b.val, BCDDIG)<0)?true:false) :
	    ((expo < b.expo)?true:false) ;

    return (sw&BCD_PLUS)?lt:!lt;
}

bool BCD::operator >=(BCD & b)
{	return !(*this < b);
}

bool BCD::operator >(BCD & b)
{	if(!isNumber() || !b.isNumber()) return false;
	if(sw&BCD_NULL && b.sw&BCD_NULL) return false;

    if(sw&BCD_NULL) return (b.sw&BCD_MINUS)? true : false;
    if(b.sw&BCD_NULL) return (sw&BCD_PLUS)? true : false;

    bool lt;

    lt=(expo == b.expo) ?
    	((DigCmp(val, b.val, BCDDIG)>0)?true:false) :
	    ((expo > b.expo)?true:false) ;

    return (sw&BCD_PLUS)?lt:!lt;
}

bool BCD::operator <=(BCD & b)
{	return !(*this > b);
}




BCD BCD::RelRound(int dig)
{	BCD a(*this);
	if(dig>0 && dig<BCDDIG && isNumber() && !(sw&BCD_NULL))
	{	if(DigRound(a.val, dig+1))
        {	int tx=a.expo;
        	tx++;
            a.val[0]=1;
            a.expo=tx;
            if(a.expo!=tx) a.sw=BCD_OVF;
            dig=1;
        }

    	for(int i=dig; i<BCDDIG; i++) a.val[i]=0;
    }
    return a;
}


BCD BCD::Round(int dig)
{	BCD a(*this);

	if(!isNumber() || sw&BCD_NULL || expo>=BCDDIG || (BCDDIG-expo-dig)<=0) return a;

	if(dig>0 && dig<BCDDIG && isNumber() && !(sw&BCD_NULL))
	{	if(DigRound(a.val, a.expo+dig+1))
        {	int tx=a.expo;
        	tx++;
            a.val[0]=1;
            a.expo=tx;
            if(a.expo!=tx) a.sw=BCD_OVF;
            for(int i=1; i<BCDDIG; i++) a.val[i]=0;
        }

        else for(int i=dig+a.expo; i<BCDDIG; i++) a.val[i]=0;
    }

    return a;
}



BCD BCD::Int(void)
{	BCD a(*this);
	if(!isNumber() || sw&BCD_NULL || expo>=BCDDIG) return a;


    if(a.expo<=0) a.sw=BCD_NULL;
    else for(int i=a.expo; i<BCDDIG; i++) a.val[i]=0;
    return a;
}

// OBS! der er ingen overflowtest
int BCD::Integer(void)
{	int r=0, i;
	if(isNumber() && !(sw&BCD_NULL) && expo>0)
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
	if(!isNumber() || a.sw&BCD_NULL || a.expo<=0) return a;


    if(a.expo>=BCDDIG)
    {	a.sw=BCD_NULL;
    	return a;
    }

    memmove(a.val, a.val+a.expo, BCDDIG-a.expo);
    for(i=BCDDIG-a.expo; i<BCDDIG; i++) a.val[i]=0;
    a.expo=0;

    if(DigIsNull(a.val, BCDDIG)) a.sw=BCD_NULL;

    return a;
}


BCD BCD::Pow(int potens)
{	BCD a(*this), b;
	int pot=(potens>0)?potens:-potens;

	if(a.sw&BCD_NULL) return a;

    if(a.isNumber() && !potens)
    {	BCD en(1);
    	return en;
    }

    while(a.isNumber() && pot && !(pot&1))
    {	a=a*a;
    	pot>>=1;
    }

    b=a;
    pot>>=1;

    while(b.isNumber() && pot)
    {	a=a*a;
    	if(pot&1) b=b*a;
    	pot>>=1;
    }


    if(potens<0 && b.isNumber())
    {	BCD en(1);
    	return en/b;
    }

    return b;

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

    } while(t.expo >= expo - BCDDIG);

    return acc;
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

	for(i=0; i<BCDDIG; i++) val[i]=0;

    sprintf(buf, "%d", b);
    for(i=0; buf[i]!='\0'; i++) val[i]=(UC)(buf[i]&0x0F);
    expo=i;
}


BCD::BCD(int b)
{	SetVal(b);
//	puts("CI");

}

BCD::BCD(char * cp)
{	FromText(cp);
//	puts("Chr");
}

BCD::BCD(BCD & b)
{	sw=b.sw;
	expo=b.expo;
    memmove(val, b.val, BCDDIG);
//    puts("Cpy");
}

BCD::BCD(void)
{	sw=BCD_NULL;
//	puts("Con");
}

BCD::~BCD(void)
{//	puts("Des");
}



BCD GenE2rec(BCD & en, BCD & fak, int n)
{	BCD a(n), b;
	fak=fak*a;
    a=en/fak;

    printf("e: recursion #%d\r", n);

	if(a.Expo() < - BCDDIG) return a;
//    b=GenE2rec(en, fak, n+1);
//    return a+b;
	return GenE2rec(en, fak, n+1) + a;
}

void GenE2(BCD & r)
{	BCD en(1), tokomma5("2,5"), fak(2), b;
	b=GenE2rec(en, fak, 3);
	r = tokomma5 + b;
}


void GenE(BCD & r)
{	BCD en(1), to(2), acc(",5"), tmp, fak(2), f;
	int i=3;

    do
    {	tmp=acc;
    	f.SetVal(i);
        fak = fak * f;
        r = en / fak;
	    acc = acc + r;
        i++;

        //printf("e: iteration #%d\r", i);

    } while(acc != tmp);

    r = to + acc;
}

//	Exp(x) - 1
void ExpXm1(BCD & r, BCD & x)
{	BCD acc(x), fak(2), xn, t;
	int i=3;

    BCD en(1), nul(0);
    if(x == nul)
    {	r=nul;
	    return;
    }

	xn = x*x;
    acc = xn / fak + acc;

    do
    {	r=acc;
    	xn = xn * x;
	    t.SetVal(i++);
    	fak = fak * t;
        acc = xn / fak + acc;
        //printf("Exp(x)-1: iteration #%d\r",i);

    } while(r != acc);
    //puts("");
}

BCD ValE;

void ExpX(BCD & r, BCD & x)
{	BCD f, en(1), e;
	int i;

    f=x.Frac();

    ExpXm1(r, f);
    r=r+en;

    //GenE(e);
    e=ValE;
    i=x.Integer();
    r = e.Pow(i) * r;
}

/*
void Exp(BCD & r, BCD & x)
{	BCD acc(x), fak(2), xn, t;
	int i=3;

    BCD en(1), nul(0);
    if(x == nul)
    {	r=nul;
	    return;
    }

    acc = acc + en;
	xn = x*x;
    acc = xn / fak + acc;

    do
    {	r=acc;
    	xn = xn * x;
	    t.SetVal(i++);
    	fak = fak * t;
        acc = xn / fak + acc;
        //printf("Exp(x)-1: iteration #%d\r",i);

    } while(r != acc);
    //puts("");
}
  */


//	ln(1+x) | |x| < 1


/*void GenLn10(BCD & r)
{
} */


void LnX(BCD & r, BCD & x)
{	BCD y("2,302585093"), t, tt(1), exp;

//	ln(frac) + expo*ln(10) --> y
	t.SetVal(x.Expo());
	y = y * t;

//    t=x.Frac() - tt;
//    Ln1px(tt, t);   	//	frac == 0 --> Ln1px(-1) --> loop
    y = y - tt;


	do
    {	t=y;
        ExpX(exp, y);
        tt=(x-exp)/exp;
        y=y+tt;

    } while (t != y);

    r=y;
}

void Pi4A(BCD & t)
{	BCD acc(t), xn(t), x2(t), fak(3), to(2), cmp, tt;

#ifdef BCD_TEST
	int i=0;
#endif
	x2=x2*x2;	//	t^2
    xn=xn*x2;	//	t^3

    do
    {	cmp=acc;
    	tt=xn/fak;
        acc = acc - tt;
        fak=fak+to;
        xn=xn*x2;
        acc = xn/fak + acc;
        fak=fak+to;
        xn=xn*x2;
#ifdef BCD_TEST
		i+=2;
		printf("Pi4A iter # %d\r",i);
#endif

    } while(cmp != acc);

    t=acc;

}

void Pi4(BCD & pi4)
{	BCD en(1), t, t2, acc;
#ifdef BCD_TEST
	puts("1/239");
#endif
	t.SetVal(239);
    t=en/t;
    Pi4A(t);

    acc=t;

#ifdef BCD_TEST
	puts("1/157");
#endif

    t.SetVal(57);
    t=en/t;
    Pi4A(t);
    t2.SetVal(2);
    t=t*t2;

    acc=acc+t;

#ifdef BCD_TEST
	puts("1/8");
#endif


    t.SetVal(8);
    t=en/t;
    Pi4A(t);
    t2.SetVal(6);
    t=t*t2;

    acc=acc+t;

    pi4=acc;
}


void GenFile(void)
{	BCD a, x;
	char buf[BCDDIG+25];
    int i;
    FILE * f;
    f=fopen("c:\\pi4.txt", "w");
    if(!f)
    {	puts("kan ikke åbne outputfilen");
    	return;
    }
          /*
    fprintf(f, "konstanter genereret vha Taylor & Newton med 1200 cifre - afrundet til 1000 cifre\n\n");

    puts("[e]");
    fprintf(f, "[e]\n" );
    GenE(x);
    ValE=x;
    x=x.RelRound(BCDROUND);
    x.ToText(buf, BCD_NTZ);
    puts(buf);
    fprintf(f, "%s\n",buf);

    puts("[ln(10)]");
    fprintf(f, "[ln(10)]\n");
    a.SetVal(10);
    LnX(x,a);
    x=x.RelRound(BCDROUND);
    x.ToText(buf, BCD_NTZ);
    puts(buf);
    fprintf(f, "%s\n",buf);


    for(i=1; i<=100; i++)
    {	printf("[ln(1 + 1E-%d)]\n", i);
	    fprintf(f,"[ln(1 + 1E-%d)]\n", i);
	    sprintf(buf, "1E-%d", i);
        x.FromText(buf);

        a=x.Ln1px();
        a=a.RelRound(BCDROUND);
	    a.ToText(buf, BCD_NTZ);
	    puts(buf);
        fprintf(f, "%s\n",buf);
    }

    fprintf(f, "\nBrug Taylor ln(1+x) hhv exp(x) til resten \n\n");

            */

    BCD pi4, fak(2), r;
	Pi4(pi4);
    r=pi4;
    r=r.RelRound(BCDROUND);
    r.ToText(buf, BCD_NTZ);

    puts("[Pi/4]");
    puts(buf);
    fprintf(f, "[Pi/4]\n");
    fprintf(f, "%s\n", buf);


    r=pi4*fak;
    r=r.RelRound(BCDROUND);
    r.ToText(buf, BCD_NTZ);

    puts("[Pi/2]");
    puts(buf);
    fprintf(f, "[Pi/2]\n");
    fprintf(f, "%s\n", buf);


    fak.SetVal(4);
    r=pi4*fak;
    r=r.RelRound(BCDROUND);
    r.ToText(buf, BCD_NTZ);

    puts("[Pi]");
    puts(buf);
    fprintf(f, "[Pi]\n");
    fprintf(f, "%s\n", buf);


    fak.SetVal(8);
    r=pi4*fak;
    r=r.RelRound(BCDROUND);
    r.ToText(buf, BCD_NTZ);

    puts("[2Pi]");
    puts(buf);
    fprintf(f, "[2Pi]\n");
    fprintf(f, "%s\n", buf);


	fprintf(f, "\n\
sin(x) = -sin(x-pi) = sin(pi-x) = cos(pi/2-x)\n\
            x>pi        x>pi/2      x>pi/4\n\
Reducér x til [0;pi/4] og beregn Taylor sin hhv cos\n\nCos(x) = sin(x+pi/2\n");


                    /*
    for(i=1; i<=100; i++)
    {	printf("[ArcTan(1E-%d)]\n", i);
	    fprintf(f,"[ArcTan(1E-%d)]\n", i);
	    sprintf(buf, "1E-%d", i);
        x.FromText(buf);

        a=x.atan();
        a=a.RelRound(BCDROUND);
	    a.ToText(buf, BCD_NTZ);
	    puts(buf);
        fprintf(f, "%s\n",buf);
    }

    */
               /*
    BCD b;

    x.SetVal(1);
    a=x.atan1();

    b=a.RelRound(BCDROUND);
    b.ToText(buf, BCD_NTZ);
    puts(buf);
    fprintf(f, "%s\n",buf);

    a=a+a;
    b=a.RelRound(BCDROUND);
    b.ToText(buf, BCD_NTZ);
    puts(buf);
    fprintf(f, "%s\n",buf);

    a=a+a;
    b=a.RelRound(BCDROUND);
    b.ToText(buf, BCD_NTZ);
    puts(buf);
    fprintf(f, "%s\n",buf);


    a=a+a;
    b=a.RelRound(BCDROUND);
    b.ToText(buf, BCD_NTZ);
    puts(buf);
    fprintf(f, "%s\n",buf);

*/








    fclose(f);
}

void main(void)
{	GenFile();




    getchar();
}
