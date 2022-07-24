//---------------------------------------------------------------------------
#include <vcl.h>
#include <dir.h>
#include <stdio.h>
#pragma hdrstop

#include "Emu.h"
#include "calchw.h"
#include "rom.h"
#define NOTYPEREDEF
#include "Z80.h"
#include "Debug.h"

extern Z80_Regs R;
static int keymap83[]={VK_ESCAPE,k_esc,'1',k_1,'2',k_2,
    '3',k_3,'4',k_4,'5',k_5,'6',k_6,'7',k_7,
    '8',k_8,'9',k_9,'0',k_0,0xbd,k_neg,
    0xbb,k_equ,VK_BACK,k_left,VK_TAB,k_sto,'Q',
    k_q,'W',k_w,'E',k_e,'R',k_r,'T',k_t,'Y',k_y,
    'U',k_u,'I',k_i,'O',k_o,'P',k_p,0xdb,
    k_lparan,0xdd,k_rparan,VK_RETURN,k_enter1,
    'A',k_a,'S',k_s,'D',k_d,
    'F',k_f,'G',k_g,'H',k_h,'J',k_j,'K',k_k,
    'L',k_l,0xc0,k_xvar,VK_SHIFT,k_shift,
    0xdc,k_ln,'Z',k_z,'X',k_x,'C',k_c,
    'V',k_v,'B',k_b,'N',k_n,'M',k_m,0xbc,
    k_comma,0xbe,k_dot,0xbf,k_div,VK_MULTIPLY,
    k_mult,VK_MENU,k_2nd,VK_SPACE,k_space,VK_CAPITAL,
    k_hand,VK_F1,k_f1,VK_F2,k_f2,VK_F3,k_f3,VK_F4,
    k_f4,VK_F5,k_f5,VK_F6,k_graph,VK_F7,k_table,VK_F8,
    k_prgm,VK_F9,k_custom,VK_NUMLOCK,k_power,VK_SCROLL,
    k_on,VK_NUMPAD7,k_7,VK_NUMPAD8,k_8,VK_NUMPAD9,k_9,
    VK_SUBTRACT,k_minus,VK_NUMPAD4,k_4,VK_NUMPAD5,k_5,
    VK_NUMPAD6,k_6,VK_ADD,k_plus,VK_NUMPAD1,k_1,
    VK_NUMPAD2,k_2,VK_NUMPAD3,k_3,VK_NUMPAD0,k_0,
    VK_DECIMAL,k_dot,VK_SEPARATOR,k_enter2,VK_DIVIDE,
    k_div,VK_HOME,k_cos,VK_UP,k_up,VK_PRIOR,k_tan,
    VK_LEFT,k_left,VK_RIGHT,k_right,VK_DOWN,k_down,
    VK_INSERT,k_sin,VK_DELETE,k_bs,0xba,
    k_square,0xde,k_log,VK_END,k_ee,
    VK_NEXT,k_clear,-1,-1};

static int ti83kmat[][8]=
    {kn,kn,kn,kn,k_up,k_right,k_left,k_down,
     kn,k_clear,k_power,k_div,k_mult,k_minus,k_plus,k_enter1,
     kn,k_custom,k_tan,k_rparan,k_9,k_6,k_3,k_neg,
     k_equ,k_prgm,k_cos,k_lparan,k_8,k_5,k_2,k_dot,
     k_xvar,k_table,k_sin,k_comma,k_7,k_4,k_1,k_0,
     k_hand,k_graph,k_ee,k_square,k_log,k_ln,k_sto,kn,
     k_bs,k_esc,k_2nd,k_f1,k_f2,k_f3,k_f4,k_f5,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int ti83alphakmat[][8]=
    {kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,k_h,k_m,k_r,k_w,kn,kn,
     kn,kn,k_g,k_l,k_q,k_v,kn,kn,
     kn,k_c,k_f,k_k,k_p,k_u,k_z,kn,
     kn,k_b,k_e,k_j,k_o,k_t,k_y,k_space,
     kn,k_a,k_d,k_i,k_n,k_s,k_x,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int key[]=
    {k_f1,k_f2,k_f3,k_f4,k_f5,
     k_2nd,k_esc,k_bs,k_left,k_right,
     k_up,k_down,k_hand,k_xvar,k_equ,
     k_graph,k_table,k_prgm,k_custom,k_clear,
     k_ee,k_sin,k_cos,k_tan,k_power,
     k_square,k_comma,k_lparan,k_rparan,k_div,
     k_log,k_7,k_8,k_9,k_mult,
     k_ln,k_4,k_5,k_6,k_minus,
     k_sto,k_1,k_2,k_3,k_plus,
     k_on,k_0,k_dot,k_neg,k_enter1};

char* defExt83[0x20]={"83n","83l","83m","83y","83p",
    "83p","83p","83i","83d","83p","83p","83w","83z",
    "83t","83p","83b"};

ROMFunc romFuncs83[]=
    {{0x4755,"_CLRLCDFULL"},{0x475D,"_clrScrnFull"},
     {0x4765,"_CLRTXTSHD"},{0x47F1,"_dispDone"},
     {0x4709,"_dispHL"},{0x51D4,"_DISPOP1A"},
     {0x476D,"_eraseEOL"},{0x4D2E,"_formDCplx"},
     {0x4D32,"_formReal"},{0x4B20,"_GDISPTOKEN"},
     {0x4775,"_homeUp"},{0x4066,"_lcd_busy"},
     {0x474D,"_moveup"},{0x473D,"_newline"},
     {0x5106,"_outputExpr"},{0x4705,"_putc"},
     {0x4701,"_putmap"},{0x4715,"_putps"},
     {0x470D,"_puts"},{0x4795,"_runIndicOff"},
     {0x4791,"_runIndicOn"},{0x4751,"_scrollDown"},
     {0x4745,"_scrollUp"},{0x51B9,"_setPenT"},
     {0x51A7,"_setPenX"},{0x51B0,"_setPenY"},
     {0x4A6C,"_SFONT_LEN"},{0x4C53,"_VPUTBLANK"},
     {0x477D,"_vputmap"},{0x4781,"_vputs"},
     {0x4785,"_vputsn"},{0x519E,"_Axes"},
     {0x4A84,"_CLine"},{0x4B00,"_CPointS"},
     {0x515B,"_GRBUFCLR"},{0x5164,"_GRBUFCPY_V"},
     {0x4BEF,"_HORIZCMD"},{0x4AE4,"_ILINE"},
     {0x4AE8,"_IPOINT"},{0x4741,"_moveDown"},
     {0x4BEB,"_PDspGrph"},{0x4BE4,"_PUTXY"},
     {0x4BCC,"_REGRAPH"},{0x4BF3,"_VERTCMD"},
     {0x4A18,"_GetK"},{0x4CFE,"_getkey"},
     {0x489D,"_ADRLELE"},{0x48A9,"_GETLTOOP1"},
     {0x48BD,"_PUTTOL"},{0x486D,"_ADRMELE"},
     {0x487D,"_GETMTOOP1"},{0x4889,"_PUTTOMAT"},
     {0x4122,"_ACOS"},{0x413A,"_ACOSH"},
     {0x4152,"_ANGLE"},{0x412A,"_ASIN"},
     {0x4136,"_ASINH"},{0x4126,"_ATAN"},
     {0x4132,"_ATANH"},{0x4BE0,"_Ceiling"},
     {0x40FA,"_COS"},{0x410A,"_COSH"},
     {0x409E,"_CUBE"},{0x4096,"_DTOR"},
     {0x40EA,"_ETOX"},{0x50C6,"_FACTORIAL"},
     {0x4092,"_FPADD"},{0x40C6,"_FPDIV"},
     {0x40AA,"_FPMULT"},{0x40C2,"_FPRECIP"},
     {0x40A6,"_FPSQUARE"},{0x408E,"_FPSUB"},
     {0x40BE,"_FRAC"},{0x40D6,"_INT"},
     {0x4076,"_INTGR"},{0x40B2,"_INVOP1SC"},
     {0x407E,"_INVSUB"},{0x40DE,"_LNX"},
     {0x40E2,"_LOGX"},{0x406E,"_MAX"},
     {0x406A,"_MIN"},{0x408A,"_MINUS1"},
     {0x4086,"_PLUS1"},{0x413E,"_PTOR"},
     {0x50B6,"_RANDOM"},{0x40CE,"_RNDGUARD"},
     {0x40DA,"_ROUND"},{0x409A,"_RTOD"},
     {0x4142,"_RTOP"},{0x40F6,"_SIN"},
     {0x410E,"_SINH"},{0x40CA,"_SQROOT"},
     {0x50BA,"_STORAND"},{0x40FE,"_TAN"},
     {0x4106,"_TANH"},{0x40EE,"_TENX"},
     {0x4082,"_TIMES2"},{0x40A2,"_TIMESPT5"},
     {0x407A,"_TRUNC"},{0x4A8C,"_XROOTY"},
     {0x4A90,"_YTOX"},{0x442A,"_CHKFINDSYM"},
     {0x44AA,"_DELVAR"},{0x4EF0,"_RCLSYSTOK"},
     {0x4EEC,"_RCLVARSYM"},{0x4EE8,"_RCLX"},
     {0x4EE0,"_RCLY"},{0x47A1,"_RSTRSHADOW"},
     {0x479D,"_SAVESHADOW"},{0x4ED8,"_STOOTHER"},
     {0x4EB8,"_STOSYSTOK"},{0x4ED4,"_STOX"},
     {0x4EC8,"_STOY"},{0x4322,"_CKINT"},
     {0x4326,"_CKODD"},{0x4312,"_CKOP1FP0"},
     {0x435A,"_CKOP1POS"},{0x414E,"_CKOP1REAL"},
     {0x4316,"_CKOP2FP0"},{0x4356,"_CKOP2POS"},
     {0x4412,"_CKOP2REAL"},{0x431E,"_CKPOSINT"},
     {0x4362,"_CLROP1S"},{0x435E,"_CLROP2S"},
     {0x4EFC,"_CONVOP1"},{0x4004,"_cphlde"},
     {0x4166,"_CPOP1OP2"},{0x400C,"_divHLbyA"},
     {0x4382,"_HTIMESL"},{0x40B6,"_INVOP1S"},
     {0x40BA,"_INVOP2S"},{0x4000,"_ldHLind"},
     {0x4212,"_MOVFROP1"},{0x4306,"_OP1EXOP2"},
     {0x42FE,"_OP1EXOP3"},{0x4302,"_OP1EXOP4"},
     {0x42EE,"_OP1EXOP5"},{0x42F2,"_OP1EXOP6"},
     {0x4286,"_OP1SET0"},{0x4232,"_OP1SET1"},
     {0x4242,"_OP1SET2"},{0x423A,"_OP1SET3"},
     {0x4236,"_OP1SET4"},{0x418E,"_OP1TOOP2"},
     {0x417E,"_OP1TOOP3"},{0x416E,"_OP1TOOP4"},
     {0x41BE,"_OP1TOOP5"},{0x41BA,"_OP1TOOP6"},
     {0x42F6,"_OP2EXOP4"},{0x42FA,"_OP2EXOP5"},
     {0x42E6,"_OP2EXOP6"},{0x4282,"_OP2SET0"},
     {0x4252,"_OP2SET1"},{0x4246,"_OP2SET2"},
     {0x422E,"_OP2SET3"},{0x422A,"_OP2SET4"},
     {0x4222,"_OP2SET5"},{0x4EA8,"_OP2SET60"},
     {0x421E,"_OP2SET8"},{0x4226,"_OP2SETA"},
     {0x41C2,"_OP2TOOP1"},{0x41F6,"_OP2TOOP3"},
     {0x4172,"_OP2TOOP4"},{0x41B2,"_OP2TOOP5"},
     {0x41B6,"_OP2TOOP6"},{0x427E,"_OP3SET0"},
     {0x421A,"_OP3SET1"},{0x423E,"_OP3SET2"},
     {0x41A2,"_OP3TOOP1"},{0x417A,"_OP3TOOP2"},
     {0x416A,"_OP3TOOP4"},{0x41AE,"_OP3TOOP5"},
     {0x427A,"_OP4SET0"},{0x4216,"_OP4SET1"},
     {0x419A,"_OP4TOOP1"},{0x4176,"_OP4TOOP2"},
     {0x41FA,"_OP4TOOP3"},{0x41AA,"_OP4TOOP5"},
     {0x4202,"_OP4TOOP6"},{0x42EA,"_OP5EXOP6"},
     {0x4276,"_OP5SET0"},{0x419E,"_OP5TOOP1"},
     {0x4182,"_OP5TOOP2"},{0x41FE,"_OP5TOOP3"},
     {0x418A,"_OP5TOOP4"},{0x4186,"_OP5TOOP6"},
     {0x4196,"_OP6TOOP1"},{0x4192,"_OP6TOOP2"},
     {0x41A6,"_OP6TOOP5"},{0x4536,"_PUSHREALO1"},
     {0x4A74,"_SETXXOP1"},{0x4A78,"_SETXXOP2"},
     {0x4A7C,"_SETXXXXOP2"},{0x428E,"_ZEROOOP1"},
     {0x8044,"OP2"},{0x8045,"OP2EXP"},
     {0x8046,"OP2M"},{0x803B,"OP1M"},
     {0x803A,"OP1EXP"},{0x8039,"OP1"},
     {0x804F,"OP3"},{0x8050,"OP3EXP"},
     {0x8058,"OP3EXT"},{0x8051,"OP3M"},
     {0x805A,"OP4"},{0x805B,"OP4EXP"},
     {0x8063,"OP4EXT"},{0x805C,"OP4M"},
     {0x8065,"OP5"},{0x8066,"OP5EXP"},
     {0x806E,"OP5EXT"},{0x8067,"OP5M"},
     {0x8070,"OP6"},{0x8071,"OP6EXP"},
     {0x8079,"OP6EXT"},{0x8072,"OP6M"},
     {0x8094,"IMATHPTR1"},{0x8096,"IMATHPTR2"},
     {0x8098,"IMATHPTR3"},{0x809A,"IMATHPTR4"},
     {0x809C,"IMATHPTR5"},{0x9319,"PROGPTR"},
     {0x80C9,"TEXTSHADOW"},{0x9157,"CMDSHADOW"},
     {0x8E29,"PLOTSSCREEN"},{0x8265,"SAVESSCREEN"},
     {0x858F,"STATVARS"},{0xFE6E,"SYMTABLE"},
     {0x800C,"CURROW"},{0x800D,"CURCOL"},
     {0x8252,"PENCOL"},{0x8253,"PENROW"},
     {0x886D,"CURGY"},{0x886E,"CURGX"},
     {0x80C8,"ASM_IND_CALL"},{0,NULL}};

TI83::TI83()
{
    romFuncs=romFuncs83;
    asm_readmem=debug_readmem;
    asm_readmem_word=debug_readmem_word;
    asm_readmem_dword=debug_readmem_dword;
    asm_writemem=debug_writemem;
    asm_writemem_word=debug_writemem_word;
    asm_writemem_dword=debug_writemem_dword;
}

void TI83::EnableDebug()
{
}

void TI83::DisableDebug()
{
}

int* TI83::GetKeyMap()
{
    return keymap83;
}

int TI83::GetKeyMat(int r,int c)
{
    return ti83kmat[r][c];
}

int TI83::GetKeyAlphaMat(int r,int c)
{
    return ti83alphakmat[r][c];
}

void TI83::Reset()
{
    chdir(initPath);
	LoadROM(currentROM);
    int i;
    for (i=0;i<0x40000;i++)
        ram[i]=0;
	for (i=0;i<256;i++)
	{
		if (initialPC<0x400000)
			ram[i]=intRom[i+initIntTabOfs];
		else
			ram[i]=extRom[i+initIntTabOfs];
	}
    for (i=0;i<768;i++)
        vidMem[i]=0;
	Z80_Reset();
    R.PC.D=initialPC;
	contrast=16;
	kbmask=0xff;
    for (i=0;i<7;i++) kbmatrix[i]=0xff;
	onheld=1; lcdBase=VIDMEM_ADDR;
    for (i=0;i<6;i++) oldScreenBase[i]=lcdBase;
	lcdRefresh=0;
    cpuCompleteStop=0; lcdOff=0;
    pageA=rom; pageAType=0; pageANum=0;
    port3Left=100000;
    curCol=0; curRow=0; dir=0; justSet=0; mode=0;
    redWire=1; whiteWire=1;
    outRed=0; outWhite=0; getError=0;
    intr=0; timerSpeed=55000;
    maskTimer=0; maskOn=0;
    port2=0;
    for (int i=0;romFuncs[i].name;i++)
        romFuncAddr[i]=romFuncs[i].num;
}

int TI83::getmem(int addr)
{
    if ((addr>>20)==6)
        return readport(addr);
    if ((addr&0xffff8000)==VIDMEM_ADDR)
        return vidMem[addr&0xfff];
	if ((word)addr<0x4000)
		return (rom[addr&0x3fff])&0xff;
	else if ((word)addr<0x8000)
		return (pageA[addr&0x3fff])&0xff;
	else
		return (ram[addr&0x7fff])&0xff;
}

int TI83::getmem_word(int addr)
{
	return ((getmem(addr+1)<<8)|getmem(addr))&0xffff;
}

int TI83::getmem_dword(int addr)
{
	return (getmem(addr+3)<<24)|
	       (getmem(addr+2)<<16)|
	       (getmem(addr+1)<<8)|
	       getmem(addr);
}

void TI83::setmem(int addr,int v)
{
    addr&=0xffffff;
	if ((word)addr>=0x8000)
		ram[addr&0x7fff]=v;
}

void TI83::setmem_word(int addr,int v)
{
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

void TI83::setmem_dword(int addr,int v)
{
	setmem(addr+3,(v>>24)&0xff);
	setmem(addr+2,(v>>16)&0xff);
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

int TI83::readmem(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr&0xffff8000)==VIDMEM_ADDR)
        return vidMem[addr&0xfff];
	if ((word)addr<0x4000)
		return (rom[addr&0x3fff])&0xff;
	else if ((word)addr<0x8000)
		return (pageA[addr&0x3fff])&0xff;
	else
		return (ram[addr&0x7fff])&0xff;
}

int TI83::readmem_word(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_READ,0))
            DataBreak();
    }
	return ((getmem(addr+1)<<8)|getmem(addr))&0xffff;
}

int TI83::readmem_dword(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_READ,0))
            DataBreak();
    }
	return (getmem(addr+3)<<24)|
	       (getmem(addr+2)<<16)|
	       (getmem(addr+1)<<8)|
	       getmem(addr);
}

void TI83::writemem(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_WRITE,v))
            DataBreak();
    }
    addr&=0xffffff;
	if ((word)addr>=0x8000)
		ram[addr&0x7fff]=v;
}

void TI83::writemem_word(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_WRITE,v))
            DataBreak();
    }
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

void TI83::writemem_dword(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_WRITE,v))
            DataBreak();
    }
	setmem(addr+3,(v>>24)&0xff);
	setmem(addr+2,(v>>16)&0xff);
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

int TI83::readport(int port)
{
    int i,v;
    static int onpressed=0;
    if ((port>>20)!=6)
    {
        if (CheckAddr((port&0xff)|0x600000,1,DATABREAK_READ,0))
            DataBreak();
    }
    switch(port&0xff)
    {
        case 0:
            return ((outWhite)<<1)|((outRed))|
                ((whiteWire&(1-outWhite))<<3)|
                ((redWire&(1-outRed))<<2)|
                ((pageANum&8)<<1);
        case 1:
            for (v=0xff,i=0;i<7;i++)
			{
			    if (!(kbmask&(1<<i)))
			   	    v&=kbmatrix[i];
            }
			return v;
        case 2:
            return port2;
        case 3:
            if (port3Left)
            {
                port3Left--;
                return 1;
            }
            if ((!onpressed)&&(!onheld))
            {
                onpressed=1;
                return (onheld<<3)|(R.IFF1?0:2)|1;
            }
            if ((onpressed)&&(onheld))
                onpressed=0;
            return (onheld<<3)|(R.IFF1?0:2);
        case 4:
            return 0xff;
        case 6:
            return pageBType|pageBNum;
        case 7:
            return 0xff;
        case 0x11:
            if (justSet)
            {
                justSet=0;
                return 0;
            }
            if (curRow<0) curRow=63;
            if (curRow>63) curRow=0;
            if (curCol<0) curCol=15;
            if (curCol>15) curCol=0;
            if (mode)
            {
                if (curCol>11) return 0;
                v=vidMem[(curRow*12)+curCol];
            }
            else
            {
                int col=6*curCol;
                int ofs=curRow*12+(col>>3);
                int shift=10-(col&7);
                v=((vidMem[ofs]<<8)|vidMem[ofs+1])>>shift;
            }
            switch (dir)
            {
                case 4: curRow--; break;
                case 5: curRow++; break;
                case 6: curCol--; break;
                case 7: curCol++; break;
            }
            return v;
        case 0x14:
            return 1;
    }
    return 0;
}

void TI83::writeport(int port,int v)
{
    if (CheckAddr((port&0xff)|0x600000,1,DATABREAK_WRITE,v)) DataBreak();
    v&=0xff;
    switch(port&0xff)
    {
        case 0:
            pageANum=(pageANum&7)|((v&16)>>1);
            pageA=&rom[pageANum<<14];
            outRed=v&1;
            outWhite=(v>>1)&1;
            if ((!sendInProgress)&&(!emuLink)&&
                ((outRed)||(outWhite))&&(!soundEnable))
                ReceiveFile();
            if ((!sendInProgress)&&(emuLink)&&
                ((outRed)||(outWhite))&&(!soundEnable))
            {
                sendInProgress=1;
                int v;
                if (GetByte(v))
                {
                    int timeout=10;
                    while (1)
                    {
                        if (!PutRecvReady())
                        {
                            Sleep(30);
                            timeout--;
                            if (timeout<=0) break;
                            continue;
                        }
                        PutRecvBuf(v);
                        break;
                    }
                }
                sendInProgress=0;
            }
            break;
        case 1:
            kbmask=v;
            break;
        case 2:
            port2=v;
            pageANum=(pageANum&8)|(v&0x7);
            pageA=&rom[pageANum<<14];
            break;
        case 3:
            maskOn=v&1;
            break;
        case 4:
            break;
        case 0x10:
            if ((v&0xc0)==0xc0)
                contrast=v&0x3f;
            else if ((v>=0x20)&&(v<=0x2f))
            {
                curCol=(v&0xff)-0x20;
                justSet=1;
            }
            else if ((v>=0x80)&&(v<=0xbf))
            {
                curRow=(v&0xff)-0x80;
                justSet=1;
            }
            else if ((v>=4)&&(v<=7))
                dir=v;
            else if (v==0)
                mode=0;
            else if (v==1)
                mode=1;
            else if (v==2)
                lcdOff=1;
            else if (v==3)
                lcdOff=0;
            break;
        case 0x11:
            if (curRow<0) curRow=63;
            if (curRow>63) curRow=0;
            if (curCol<0) curCol=15;
            if (curCol>15) curCol=0;
            if (mode)
            {
                if (curCol>11) return;
                vidMem[(curRow*12)+curCol]=v;
            }
            else
            {
                int col=6*curCol;
                int ofs=curRow*12+(col>>3);
                int shift=col&7;
                int mask;
                v<<=2;
                mask=~(252>>shift);
                vidMem[ofs]=vidMem[ofs]&mask|(v>>shift);
                if (shift>2)
                {
                    ofs++;
                    shift=8-shift;
                    mask=~(252<<shift);
                    vidMem[ofs]=vidMem[ofs]&mask|(v<<shift);
                }
            }
            switch (dir)
            {
                case 4: curRow--; break;
                case 5: curRow++; break;
                case 6: curCol--; break;
                case 7: curCol++; break;
            }
            break;
    }
}

void TI83::Execute()
{
    if (!run) return;
    UpdateDebugCheckEnable();
    intr+=375;
    if (intr>=timerSpeed)
    {
        RefreshLCD();
        intr=0;
        if (!maskTimer)
            Z80_Cause_Interrupt(0);
    }
    static int leftOver=0;
	leftOver=Z80_Execute(375-leftOver)-(375-leftOver);
}

void TI83::OneInstruction()
{
    UpdateDebugCheckEnable();
    Z80_Execute(1);
}

void TI83::OnUp()
{
    onheld=1;
}

void TI83::OnDown()
{
    onheld=0;
}

void TI83::KeyUp(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]|=1<<c;
}

void TI83::KeyDown(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]&=~(1<<c);
}

void TI83::AlphaUp()
{
    KeyUp(5,7);
}

void TI83::AlphaDown()
{
    KeyDown(5,7);
}

void TI83::KBInt()
{
    for (int i=0;i<200;i++)
        Execute();
}

int TI83::GetKeyID(int i)
{
    return key[i];
}

void TI83::SaveState(char *fn)
{
    FILE *fp=fopen(fn,"wb");
    fprintf(fp,"VTIv2.1 ");
    fwrite(romImage[currentROM].name,56,1,fp);
    fwrite(&R,sizeof(Z80_Regs),1,fp);
    fwrite(ram,32,1024,fp); fwrite(vidMem,768,1,fp);
    fwrite(&kbmask,4,1,fp);
    fwrite(&pageAType,4,1,fp); fwrite(&pageANum,4,1,fp);
    fwrite(&port3Left,4,1,fp); fwrite(&intr,4,1,fp);
    fwrite(&curCol,4,1,fp); fwrite(&curRow,4,1,fp);
    fwrite(&dir,4,1,fp);
    fwrite(&justSet,4,1,fp); fwrite(&mode,4,1,fp);
    fwrite(&redWire,4,1,fp); fwrite(&whiteWire,4,1,fp);
    fwrite(&outRed,4,1,fp); fwrite(&outWhite,4,1,fp);
    fwrite(&timerSpeed,4,1,fp);
    fwrite(&maskTimer,4,1,fp); fwrite(&maskOn,4,1,fp);
    fwrite(&lcdBase,4,1,fp); fwrite(&lcdOff,4,1,fp);
    fwrite(&contrast,4,1,fp); fwrite(&port2,4,1,fp);
    fclose(fp);
}

void TI83::LoadState(char *fn)
{
    FILE *fp=fopen(fn,"rb");
    if (!fp) return;
    char str[9],romName[56];
    fread(str,8,1,fp);
    str[8]=0;
    int ver=200;
    if (!strcmp(str,"VTIv2.1 "))
        ver=210;
    else if (strcmp(str,"VTIv2.0 "))
        return;
    fread(romName,56,1,fp);
    if (strcmp(romName,romImage[currentROM].name))
        return;
    fread(&R,sizeof(Z80_Regs),1,fp);
    fread(ram,32,1024,fp); fread(vidMem,768,1,fp);
    fread(&kbmask,4,1,fp);
    fread(&pageAType,4,1,fp); fread(&pageANum,4,1,fp);
    pageA=&rom[pageANum<<14];
    fread(&port3Left,4,1,fp); fread(&intr,4,1,fp);
    fread(&curCol,4,1,fp); fread(&curRow,4,1,fp);
    fread(&dir,4,1,fp);
    fread(&justSet,4,1,fp); fread(&mode,4,1,fp);
    fread(&redWire,4,1,fp); fread(&whiteWire,4,1,fp);
    fread(&outRed,4,1,fp); fread(&outWhite,4,1,fp);
    fread(&timerSpeed,4,1,fp);
    fread(&maskTimer,4,1,fp); fread(&maskOn,4,1,fp);
    fread(&lcdBase,4,1,fp); fread(&lcdOff,4,1,fp);
    fread(&contrast,4,1,fp);
    if (ver>=210)
        fread(&port2,4,1,fp);
    fclose(fp);
}

int TI83::SendByte(int c)
{
    int bit=1;
    int left;
    for (int i=0;i<8;i++,bit<<=1)
    {
        if (c&bit)
        {
            redWire=1;
            whiteWire=0;
            left=200000;
            while (((redWire&(1-outRed))||(whiteWire&
                (1-outWhite)))&&(--left))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=timerSpeed)
                {
                    RefreshLCD();
                    intr=0;
                    if (!maskTimer)
                        Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
            redWire=1;
            whiteWire=1;
            while (!((redWire&(1-outRed))&&
                (whiteWire&(1-outWhite)))&&(--left))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=timerSpeed)
                {
                    RefreshLCD();
                    intr=0;
                    if (!maskTimer)
                        Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
        }
        else
        {
            redWire=0;
            whiteWire=1;
            left=200000;
            while (((redWire&(1-outRed))||(whiteWire&
                (1-outWhite)))&&(--left))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=timerSpeed)
                {
                    RefreshLCD();
                    intr=0;
                    if (!maskTimer)
                        Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
            redWire=1;
            whiteWire=1;
            left=200000;
            while ((!((redWire&(1-outRed))&&
                (whiteWire&(1-outWhite))))&&(--left))
            {
                Z80_Execute(1);
                intr++;
                if (intr>=timerSpeed)
                {
                    RefreshLCD();
                    intr=0;
                    if (!maskTimer)
                        Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
        }
    }
    return 1;
}

int TI83::GetByte(int &c)
{
    int left=6000000;
    c=0;
    for (int i=0;i<8;i++)
    {
        while ((redWire&(1-outRed))&&(whiteWire&(1-outWhite))&&
            (--left))
        {
            Z80_Execute(1);
            intr++;
            if (intr>=timerSpeed)
            {
                RefreshLCD();
                intr=0;
                if (!maskTimer)
                    Z80_Cause_Interrupt(0);
            }
        }
        if (!left) { getError=1; return 0; }
        c|=outWhite<<i;
        whiteWire=outWhite; redWire=outRed;
        left=200000;
        while ((outWhite||outRed)&&(--left))
        {
            Z80_Execute(1);
            intr++;
            if (intr>=timerSpeed)
            {
                RefreshLCD();
                intr=0;
                if (!maskTimer)
                    Z80_Cause_Interrupt(0);
            }
        }
        if (!left) { getError=1; return 0; }
        whiteWire=1; redWire=1;
        left=200000;
    }
    return 1;
}

int TI83::SendReady()
{
    return 1;
}

int TI83::SendBuffer(int c)
{
    return SendByte(c);
}

int TI83::CheckBuffer()
{
    if ((redWire&(1-outRed))&&(whiteWire&(1-outWhite)))
        return 0;
    return 1;
}

int TI83::GetBuffer(int &c)
{
    return GetByte(c);
}

#define WaitForAck() \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }

int TI83::SendFile(char *fn,int more)
{
    int i,hdr,csum,c,len,len2,len3,type,nameLen;
    char name[8];

    sendInProgress=1;
    FILE *fp=fopen(fn,"rb");
    fseek(fp,0x37,SEEK_SET);
    while (!feof(fp))
    {
        hdr=0; fread(&hdr,2,1,fp);
        if ((hdr==0)||(hdr==0xffff)) break;
        len=0; fread(&len,2,1,fp);
        if ((len==0)||(len==0xffff)) break;
        type=0; fread(&type,1,1,fp);
        if (type==0xf)
        {
            len2=0; fread(&len2,2,1,fp);
            len3=0; fread(&len3,2,1,fp);
            fseek(fp,2,SEEK_CUR);
            whiteWire=1; redWire=1; KBInt();
            SendByte(0x83); SendByte(0x6);
            SendByte(9); SendByte(0);
            SendByte(len&0xff); SendByte(len>>8);
            SendByte(0xf);
            SendByte(len2&0xff); SendByte(len2>>8);
            SendByte(len3&0xff); SendByte(len3>>8);
            SendByte(0x24); SendByte(0x8d);
            csum=(len&0xff)+(len>>8)+0xf+(len2&0xff)+
                (len2>>8)+(len3&0xff)+(len3>>8)+0x24+0x8d;
            SendByte(csum&0xff); SendByte(csum>>8);
            WaitForAck();
            int left=1800000;
            while ((redWire&(1-outRed))&&(whiteWire&(1-outWhite))&&
                (--left))
            {
                Execute();
                Application->ProcessMessages();
                static int lcdRefresh=0; lcdRefresh++;
                if (lcdRefresh>320)
                {
                    lcdRefresh=0;
                    EmuWnd->LCD->Canvas->Draw(0,0,EmuWnd->bmp);
                }
            }
            if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }
            if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }
            if (c!=9) { GetByte(c); GetByte(c); fclose(fp); sendInProgress=0; return 0; }
            if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }
            if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }
            whiteWire=1; redWire=1; KBInt();
            SendByte(0x83); SendByte(0x56); SendByte(0); SendByte(0);
            SendByte(0x83); SendByte(0x15);
            SendByte(len&0xff); SendByte(len>>8);
            fseek(fp,2,SEEK_CUR);
            csum=0;
            for (i=0;i<len;i++)
            {
                c=fgetc(fp);
                if (!SendByte(c)) { fclose(fp); sendInProgress=0; return 0; }
                csum+=c;
            }
            SendByte(csum&0xff); SendByte(csum>>8);
            WaitForAck();
            whiteWire=1; redWire=1; KBInt();
            SendByte(0x83); SendByte(0x15);
            SendByte(len2&0xff); SendByte(len2>>8);
            fseek(fp,2,SEEK_CUR);
            csum=0;
            for (i=0;i<len2;i++)
            {
                c=fgetc(fp);
                if (!SendByte(c)) { fclose(fp); sendInProgress=0; return 0; }
                csum+=c;
            }
            SendByte(csum&0xff); SendByte(csum>>8);
            WaitForAck();
            whiteWire=1; redWire=1; KBInt();
            SendByte(0x83); SendByte(0x15);
            SendByte(len3&0xff); SendByte(len3>>8);
            fseek(fp,2,SEEK_CUR);
            csum=0;
            for (i=0;i<len3;i++)
            {
                c=fgetc(fp);
                if (!SendByte(c)) { fclose(fp); sendInProgress=0; return 0; }
                csum+=c;
            }
            SendByte(csum&0xff); SendByte(csum>>8);
            WaitForAck();
            fclose(fp);
            sendInProgress=0;
            return 1;
        }
        nameLen=8;
        for (i=0;i<8;i++) name[i]=0;
        fread(name,8,1,fp);
        fseek(fp,2,SEEK_CUR);
        whiteWire=1; redWire=1; KBInt();
        SendByte(0x3); SendByte(0xc9);
        SendByte(nameLen+3); SendByte(0);
        SendByte(len&0xff); SendByte(len>>8);
        SendByte(type);
        csum=(len&0xff)+(len>>8)+type;
        for (i=0;i<8;i++)
        {
            c=(int)((unsigned char)name[i]);
            SendByte(c);
            csum+=c;
        }
        SendByte(csum&0xff); SendByte((csum>>8)&0xff);
        WaitForAck(); WaitForAck();
        whiteWire=1; redWire=1; KBInt();
        SendByte(0x3); SendByte(0x56); SendByte(0); SendByte(0);
        SendByte(0x3); SendByte(0x15);
        SendByte(len&0xff); SendByte(len>>8);
        csum=0;
        for (i=0;i<len;i++)
        {
            c=fgetc(fp);
            if (!SendByte(c)) { fclose(fp); sendInProgress=0; return 0; }
            csum+=c;
        }
        SendByte(csum&0xff); SendByte((csum>>8)&0xff);
        WaitForAck();
        whiteWire=1; redWire=1;
        for (i=0;i<100000;i++)
            Execute();
    }
    fclose(fp);
    sendInProgress=0;
    return 1;
}

int TI83::ReceiveFile()
{
    int i,j,c,len,size,type,id,csum,bufLen,n;
    int curOfs,group;
    char name[9],*buf,*newBuf;

    for (i=0;i<7;i++) kbmatrix[i]=0xff;
    sendInProgress=1;
    getError=0;
    GetByte(c); id=c;
    GetByte(c); if (c!=6) { sendInProgress=0; return 0; }
    if (getError) { sendInProgress=0; getError=0; return 0; }
    Screen->Cursor=crHourGlass;
    buf=NULL; bufLen=0; n=0;
    while (1)
    {
        GetByte(c); len=c;
        GetByte(c); len|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); size=c;
        GetByte(c); size|=c<<8;
        GetByte(c); type=c;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        for (i=0;i<(len-3);i++)
        {
            GetByte(c);
            name[i]=c;
        }
        name[i]=0;
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        whiteWire=1; redWire=1; KBInt();
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        SendByte(id); SendByte(9); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c); GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c);
        GetByte(c);
        if (c!=0x15)
        {
            Screen->Cursor=crDefault;
            sendInProgress=0;
            if (buf) delete[] buf;
            return 0;
        }
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        newBuf=new char[bufLen+size+17];
        if (buf)
        {
            memcpy(newBuf,buf,bufLen);
            delete[] buf;
        }
        buf=newBuf;
        curOfs=bufLen+15;
        buf[bufLen]=0xb; buf[bufLen+1]=0;
        buf[bufLen+2]=size&0xff; buf[bufLen+3]=size>>8;
        buf[bufLen+4]=type;
        for (i=0;i<8;i++)
            buf[bufLen+5+i]=name[i];
        buf[bufLen+13]=size&0xff; buf[bufLen+14]=size>>8;
        for (i=0;i<size;i++)
        {
            GetByte(c);
            buf[curOfs+i]=c;
            if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        }
        bufLen+=size+15;
        GetByte(c); GetByte(c);
        whiteWire=1; redWire=1; KBInt();
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        if (!transferAsGroup)
        {
            TSaveDialog *dlog=new TSaveDialog(EmuWnd);
            dlog->DefaultExt=defExt83[type&0xf];
            if (name[0]==0x5c)
                sprintf(name,"%c",'A'+name[1]);
            else if (name[0]==0x5e)
            {
                if ((name[1]&0xf0)==0x10)
                    sprintf(name,"Y%d",(name[1]&0xf)+1);
                else if ((name[1]&0xf1)==0x20)
                    sprintf(name,"Xt%d",((name[1]&0xf)>>1)+1);
                else if ((name[1]&0xf1)==0x21)
                    sprintf(name,"Yt%d",((name[1]&0xf)>>1)+1);
                else if ((name[1]&0xf0)==0x40)
                    sprintf(name,"r%d",(name[1]&0xf)+1);
                else if ((name[1]&0xff)==0x80)
                    sprintf(name,"Un");
                else if ((name[1]&0xff)==0x81)
                    sprintf(name,"Vn");
                else if ((name[1]&0xff)==0x82)
                sprintf(name,"Wn");
            }
            else if (name[0]==0x5d)
                sprintf(name,"L%d",name[1]+1);
            for (int i=0;i<8;i++)
            {
                if ((name[i]>='A')&&(name[i]<='Z'))
                    name[i]+='a'-'A';
            }
            dlog->FileName=AnsiString(name)+AnsiString(".")+
                dlog->DefaultExt;
            dlog->FilterIndex=2;
            dlog->Options.Clear();
            dlog->Filter="TI-82 files (*.82?)|*.82?|TI-83 files (*.83?)|*.83?|TI-85 files (*.85?)|*.85?|TI-86 files (*.86?)|*.86?|TI-89 files (*.89?)|*.89?|TI-92 files (*.92?)|*.92?|TI-92 Plus files (*.9x?)|*.9x?";
            char cwd[256];
            getcwd(cwd,256);
            dlog->InitialDir=cwd;
            if (useInitPath)
            {
                useInitPath=0;
                dlog->InitialDir=initPath;
            }
            Screen->Cursor=crDefault;
            if (dlog->Execute())
            {
                csum=0;
                for (i=0;i<bufLen;i++)
                    csum+=buf[i]&0xff;
                buf[bufLen]=csum&0xff;
                buf[bufLen+1]=csum>>8;
                bufLen+=2;
                FILE *fp=fopen(dlog->FileName.c_str(),"wb");
                fprintf(fp,"**TI83**");
                fputc(0x1a,fp); fputc(0xa,fp);
                for (i=0;i<0x2b;i++)
                    fputc(0,fp);
                fputc((bufLen-2)&0xff,fp);
                fputc((bufLen-2)>>8,fp);
                for (i=0;i<bufLen;i++)
                    fputc(buf[i],fp);
                fclose(fp);
            }
            Screen->Cursor=crHourGlass;
            delete dlog;
            delete[] buf; buf=NULL; bufLen=0;
        }
        n++;
        if (c!=6) break;
    }
    GetByte(c); GetByte(c);
    whiteWire=1; redWire=1; KBInt();
    SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
    getError=0;
    Screen->Cursor=crDefault;
    if (transferAsGroup)
    {
        TSaveDialog *dlog=new TSaveDialog(EmuWnd);
        if (n==1)
        {
            dlog->DefaultExt=defExt83[type&0xf];
            group=0;
        }
        else
        {
            dlog->DefaultExt="83g";
            strcpy(name,"group");
            group=1;
        }
        if (name[0]==0x5c)
            sprintf(name,"%c",'A'+name[1]);
        else if (name[0]==0x5e)
        {
            if ((name[1]&0xf0)==0x10)
                sprintf(name,"Y%d",(name[1]&0xf)+1);
            else if ((name[1]&0xf1)==0x20)
                sprintf(name,"Xt%d",((name[1]&0xf)>>1)+1);
            else if ((name[1]&0xf1)==0x21)
                sprintf(name,"Yt%d",((name[1]&0xf)>>1)+1);
            else if ((name[1]&0xf0)==0x40)
                sprintf(name,"r%d",(name[1]&0xf)+1);
            else if ((name[1]&0xff)==0x80)
                sprintf(name,"Un");
            else if ((name[1]&0xff)==0x81)
                sprintf(name,"Vn");
            else if ((name[1]&0xff)==0x82)
            sprintf(name,"Wn");
        }
        else if (name[0]==0x5d)
            sprintf(name,"L%d",name[1]+1);
        for (int i=0;i<8;i++)
        {
            if ((name[i]>='A')&&(name[i]<='Z'))
                name[i]+='a'-'A';
        }
        dlog->FileName=AnsiString(name)+AnsiString(".")+
            dlog->DefaultExt;
        dlog->FilterIndex=2;
        dlog->Options.Clear();
        dlog->Options << ofHideReadOnly << ofOverwritePrompt << ofPathMustExist;
        dlog->Filter="TI-82 files (*.82?)|*.82?|TI-83 files (*.83?)|*.83?|TI-85 files (*.85?)|*.85?|TI-86 files (*.86?)|*.86?|TI-89 files (*.89?)|*.89?|TI-92 files (*.92?)|*.92?|TI-92 Plus files (*.9x?)|*.9x?";
        char cwd[256];
        getcwd(cwd,256);
        dlog->InitialDir=cwd;
        if (useInitPath)
        {
            useInitPath=0;
            dlog->InitialDir=initPath;
        }
        if (dlog->Execute())
        {
            csum=0;
            for (i=0;i<bufLen;i++)
                csum+=buf[i]&0xff;
            buf[bufLen]=csum&0xff;
            buf[bufLen+1]=csum>>8;
            bufLen+=2;
            FILE *fp=fopen(dlog->FileName.c_str(),"wb");
            fprintf(fp,"**TI83**");
            fputc(0x1a,fp); fputc(0xa,fp);
            for (i=0;i<0x2b;i++)
                fputc(0,fp);
            fputc((bufLen-2)&0xff,fp);
            fputc((bufLen-2)>>8,fp);
            for (i=0;i<bufLen;i++)
                fputc(buf[i],fp);
            fclose(fp);
        }
        delete dlog;
    }
    sendInProgress=0;
    if (buf) delete[] buf;
    return 1;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
