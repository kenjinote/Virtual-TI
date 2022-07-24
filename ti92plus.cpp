//---------------------------------------------------------------------------
#include <vcl.h>
#include <dir.h>
#include <stdio.h>
#pragma hdrstop

#include "calchw.h"
#include "rom.h"
#define NOTYPEREDEF
#include "m68000.h"
#include "Debug.h"
#include "Log.h"
#include "Emu.h"

static int keymap92[]={VK_ESCAPE,k_esc,'1',k_1,'2',k_2,
    '3',k_3,'4',k_4,'5',k_5,'6',k_6,'7',k_7,
    '8',k_8,'9',k_9,'0',k_0,0xbd,k_neg,
    0xbb,k_equ,VK_BACK,k_bs,VK_TAB,k_sto,'Q',
    k_q,'W',k_w,'E',k_e,'R',k_r,'T',k_t,'Y',k_y,
    'U',k_u,'I',k_i,'O',k_o,'P',k_p,0xdb,
    k_lparan,0xdd,k_rparan,VK_RETURN,k_enter1,
    VK_CONTROL,k_dia,'A',k_a,'S',k_s,'D',k_d,
    'F',k_f,'G',k_g,'H',k_h,'J',k_j,'K',k_k,
    'L',k_l,0xc0,k_mode,VK_SHIFT,k_shift,
    0xdc,k_ln,'Z',k_z,'X',k_x,'C',k_c,
    'V',k_v,'B',k_b,'N',k_n,'M',k_m,0xbc,
    k_comma,0xbe,k_dot,0xbf,k_div,VK_MULTIPLY,
    k_mult,VK_MENU,k_2nd,VK_SPACE,k_space,VK_CAPITAL,
    k_hand,VK_F1,k_f1,VK_F2,k_f2,VK_F3,k_f3,VK_F4,
    k_f4,VK_F5,k_f5,VK_F6,k_f6,VK_F7,k_f7,VK_F8,
    k_f8,VK_F9,k_apps,VK_PRIOR,k_tan,VK_NEXT,k_power,VK_SCROLL,
    k_on,VK_NUMPAD7,k_7,VK_NUMPAD8,k_8,VK_NUMPAD9,k_9,
    VK_SUBTRACT,k_minus,VK_NUMPAD4,k_4,VK_NUMPAD5,k_5,
    VK_NUMPAD6,k_6,VK_ADD,k_plus,VK_NUMPAD1,k_1,
    VK_NUMPAD2,k_2,VK_NUMPAD3,k_3,VK_NUMPAD0,k_0,
    VK_DECIMAL,k_dot,VK_SEPARATOR,k_enter2,VK_DIVIDE,
    k_div,VK_HOME,k_cos,VK_UP,k_up,
    VK_LEFT,k_left,VK_RIGHT,k_right,VK_DOWN,k_down,
    VK_INSERT,k_sin,VK_DELETE,k_clear,0xba,
    k_theta,-1,-1};

static int ti92kmat[][8]=
    {k_down,k_right,k_up,k_left,k_hand,k_shift,k_dia,k_2nd,
     k_3,k_2,k_1,k_f8,k_w,k_s,k_z,kn,
     k_6,k_5,k_4,k_f3,k_e,k_d,k_x,kn,
     k_9,k_8,k_7,k_f7,k_r,k_f,k_c,k_sto,
     k_comma,k_rparan,k_lparan,k_f2,k_t,k_g,k_v,k_space,
     k_tan,k_cos,k_sin,k_f6,k_y,k_h,k_b,k_div,
     k_p,k_enter2,k_ln,k_f1,k_u,k_j,k_n,k_power,
     k_mult,k_apps,k_clear,k_f5,k_i,k_k,k_m,k_equ,
     kn,k_esc,k_mode,k_plus,k_o,k_l,k_theta,k_bs,
     k_neg,k_dot,k_0,k_f4,k_q,k_a,k_enter1,k_minus};

static int key[]=
    {k_hand,k_f1,k_f2,k_f3,k_f4,k_f5,k_f6,k_f7,k_f8,
     k_q,k_w,k_e,k_r,k_t,k_y,k_u,k_i,k_o,k_p,
     k_a,k_s,k_d,k_f,k_g,k_h,k_j,k_k,k_l,
     k_z,k_x,k_c,k_v,k_b,k_n,k_m,k_theta,
     k_shift,k_on,k_dia,k_2nd,k_sto,k_space,k_equ,
     k_bs,k_enter1,k_2nd,k_esc,k_mode,k_clear,k_ln,
     k_apps,k_enter2,k_left,k_right,k_up,k_down,
     k_sin,k_cos,k_tan,k_power,k_lparan,k_rparan,k_comma,
     k_div,k_7,k_8,k_9,k_mult,k_4,k_5,k_6,k_minus,
     k_1,k_2,k_3,k_plus,k_0,k_dot,k_neg,k_enter1};

extern char* defExt92[0x20];
char* defExt92p[0x20]={"9xe","9xz","9xz","9xz","9xl","9xz",
    "9xm","9xz","9xz","9xz","9xc","9xt","9xs","9xd",
    "9xa","9xz","9xi","9xz","9xp","9xf","9xx","9xz",
    "9xz","9xz","9xz","9xz","9xz","9xz","9xz","9xz",
    "9xz","9xz"};

extern ROMFunc romFuncs89[];
extern ROMFunc romFuncs92[];

static DWORD funcAddr[0x42]={0x4d1204,0x4d12dc,0x4620fe,
    0x46217c,0x400ed6,0x400efc,0x4023a8,0x40241a,0x402472,
    0x4024ac,0x402530,0x4620e4,0x4d11c8,0x400eac,0x4622d6,
    0x483706,0x45885c,0x4591a8,0x4572c8,0x457394,0x4573d2,
    0x4573e0,0x457404,0x459d66,0x45a102,0x45a88e,0x45b0a6,
    0x79a8,0x4720,0x7a60,0x45e23c,0x45e424,0x45e5e2,
    0x45b578,0x4622a6,0x4d1c70,0x4645be,0x400000,0x45731a,
    0x482c90,0x4d1b80,0x4d1b98,0x4d1bec,0x4d1c20,0x4d1c44,
    0x42fd64,0x4d00d0,0x4c912a,0x430378,0x4d1ea8,0x4d1eb4,
    0x4d1f8c,0x4d1fd4,0x4d2108,0x4d2160,0x4d1d78,0x4d1dba,
    0x4d1c94,0x4d1cbe,0x4d1d54,0x4d1d64,0x4d1dd0,0x4d1df0,
    0x4d1e44,0x4d1e52,0x463af8};

void IRQ(int level)
{
    if (cpuCompleteStop&&(level==6))
    {
        cpuCompleteStop=0;
        return;
    }
    MC68000_Cause_Interrupt(level);
//	pending_interrupts|=1<<(level+24);
}

TI92Plus::TI92Plus()
{
    romFuncs=plusMod?romFuncs89:romFuncs92;
    asm_readmem=asm_89_readmem;
    asm_readmem_word=asm_89_readmem_word;
    asm_readmem_dword=asm_89_readmem_dword;
    asm_writemem=asm_89_writemem;
    asm_writemem_word=asm_89_writemem_word;
    asm_writemem_dword=asm_89_writemem_dword;
}

void TI92Plus::EnableDebug()
{
    asm_readmem=debug_readmem;
    asm_readmem_word=debug_readmem_word;
    asm_readmem_dword=debug_readmem_dword;
    asm_writemem=debug_writemem;
    asm_writemem_word=debug_writemem_word;
    asm_writemem_dword=debug_writemem_dword;
}

void TI92Plus::DisableDebug()
{
    asm_readmem=asm_89_readmem;
    asm_readmem_word=asm_89_readmem_word;
    asm_readmem_dword=asm_89_readmem_dword;
    asm_writemem=asm_89_writemem;
    asm_writemem_word=asm_89_writemem_word;
    asm_writemem_dword=asm_89_writemem_dword;
}

int* TI92Plus::GetKeyMap()
{
    return keymap92;
}

int TI92Plus::GetKeyMat(int r,int c)
{
    return ti92kmat[r][c];
}

int TI92Plus::GetKeyAlphaMat(int r,int c)
{
    return kn;
}

void TI92Plus::Reset()
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
    for (i=0;i<32;i++)
        mem[i]=&ram[(i&3)<<16];
    for (i=0;i<32;i++)
        mem[i+32]=&intRom[i<<16];
    for (i=0;i<32;i++)
        mem[i+64]=&extRom[i<<16];
    for (i=96;i<256;i++)
        mem[i]=garbageMem;
    for (i=0;i<0x10000;i++)
        garbageMem[i]=0x14;
	MC68000_Reset();
	regs.pc=initialPC;
	contrast=16;
	ram128=0;
	memprotect=0;
	kbmask=0xff;
    for (i=0;i<10;i++) kbmatrix[i]=0xff;
	ramWrap=0; memAnd=0x3ffff;
	onheld=1; lcdBase=((calc==89)||plusMod)?0x4c00:0x4440;
    for (i=0;i<6;i++) oldScreenBase[i]=lcdBase;
	timer=0; timerInit=0xb2; int0Count=0;
	transflag=0; recvflag=0;
	transnotready=0;
	lcdRefresh=0;
    io0Bit2=1; io0Bit7=1;
    romWriteReady=0; romWritePhase=0; romRetOr=0;
    cpuCompleteStop=0; lcdOff=0;
    romErase=0;
    directAccess=0; outRed=1; outWhite=1; getError=0;
    for (i=0;i<32;i++) romChanged[i]=0;
    memset(romFuncAddr,0,sizeof(int)*0x800);
    if (plusMod)
    {
        for (int i=0;(romFuncs[i].name)&&(i<(getmem_dword(getmem_dword(0xc8)-4))-1);i++)
            romFuncAddr[i]=getmem_dword(getmem_dword(0xc8)+(i<<2));
    }
    else
    {
        for (int i=0;romFuncs[i].name;i++)
        {
            romFuncAddr[i]=funcAddr[i];
            if ((initialPC<0x400000)&&
                (romFuncAddr[i]>=0x400000))
                romFuncAddr[i]-=0x200000;
        }
    }
}

int TI92Plus::ioReadByte(int p)
{
	int v,i;
	switch (p)
	{
		case 0: v=((contrast&1)<<5)|(io0Bit7<<7)|(io0Bit2<<2);
            io0Bit2=1;
            return v;
		case 1: return (memprotect<<2)|ram128;
		case 2: case 4: case 6: case 8: case 0xa: return 0x14;
		case 3: case 5: case 7: case 9: case 0xb: return 1;
		case 0xc: return 5|((1-transflag)<<1)|(directAccess?0x40:0);
		case 0xd:
            if ((emuLink)&&(!recvflag)&&(!transflag))
            {
                if (CheckSendBuf())
                {
                    recvflag=1;
                    recvbyte=GetSendBuf();
                    lastByteTicks=0;
                    lastIRQ4=0;
                    byteCount++;
                    if (byteCount<4)
                        IRQ(4);
                    else
                        byteCount=0;
                }
            }
            return (recvflag<<5)|((1-transflag)<<6);
		case 0xe:
            if (directAccess)
                return 0x50|(outWhite<<1)|outRed;
            return 0x10;
		case 0xf: recvflag=0; return recvbyte;
		case 0x10: case 0x12: return 0x14;
		case 0x11: case 0x13: return 1;
		case 0x14: return 0x14;
		case 0x15: return 0x1b;
		case 0x16: return 0x14;
		case 0x17: return timer;
		case 0x18: return (kbmask>>8)&3;
		case 0x19: return kbmask&0xff;
		case 0x1a: return 0x14|(onheld<<1);
		case 0x1b: for (v=0xff,i=0;i<10;i++)
			   {
			   	if (!(kbmask&(1<<i)))
			   		v&=kbmatrix[i];
			   }
			   return v;
		case 0x1c: case 0x1e: return 0x14;
		case 0x1d: case 0x1f: return 1;
	}
    return 0;
}

void TI92Plus::ioWriteByte(int p,int v)
{
	switch(p)
	{
		case 0: contrast=(contrast&(~1))|((v>>5)&1);
            io0Bit7=(v>>7)&1;
            io0Bit2=(v>>2)&1;
            break;
		case 1: ram128=v&1; memprotect=(v>>2)&1;
			memAnd=(ramWrap)?0x1ffff:(ram128?0x1ffff:0x3ffff);
            for (int i=0;i<32;i++)
                mem[i]=&ram[(i&(memAnd>>16))<<16];
			break;
		case 5: if (!(v&0x10)) cpuCompleteStop=1; break;
		case 0xc:
            directAccess=(v>>6)&1;
            break;
		case 0xd: break;
		case 0xe:
            directAccess=(v>>6)&1;
            if (directAccess)
            {
                outWhite=(v>>1)&1;
                outRed=v&1;
            }
            break;
		case 0xf:
            if ((emuLink)&&(!transflag))
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
            else
            {
                transflag=1;
                transbyte=v;
            }
            if (debugLogLinkSend)
            {
                char str[32];
                sprintf(str,"Send of %2.2X to link port",v&0xff);
                LogDlg->List->Items->Add(str);
            }
            IRQ(4);
            updateLink=1;
            if ((!sendInProgress)&&(!emuLink))
                ReceiveFile();
            break;
		case 0x10: lcdBase=(((lcdBase>>3)&0xff)|((v&0xff)<<8))<<3;
			   break;
		case 0x11: lcdBase=(((lcdBase>>3)&0xff00)|(v&0xff))<<3;
		           break;
		case 0x17: timerInit=v; break;
		case 0x18: kbmask=(kbmask&0xff)|((v&3)<<8); break;
		case 0x19: kbmask=(kbmask&0x300)|(v&0xff); break;
        case 0x1c: lcdOff=(v&0x80)>>7; break;
		case 0x1d: contrast=(contrast&1)|((v&15)<<1); break;
		default: break;
	}
}

int TI92Plus::getmem(int addr)
{
    addr&=0xffffff;
    if (addr<0x200000)
		return (mem[addr>>16][addr&0xffff]);
	else if (addr<0x600000)
		return (mem[addr>>16][addr&0xffff])|romRetOr;
	else if (addr<0x700000)
		return (ioReadByte(addr&0x1f))&0xff;
}

int TI92Plus::getmem_word(int addr)
{
	return ((getmem(addr)<<8)|getmem(addr+1))&0xffff;
}

int TI92Plus::getmem_dword(int addr)
{
	return (getmem(addr)<<24)|
	       (getmem(addr+1)<<16)|
	       (getmem(addr+2)<<8)|
	       getmem(addr+3);
}

void TI92Plus::setmem(int addr,int v)
{
    addr&=0xffffff;
	if (addr<0x200000)
		ram[addr&memAnd]=v;
    else if (addr<0x400000)
        intRomWriteByte(addr&0x1fffff,v);
    else if (addr<0x600000)
        extRomWriteByte(addr&0x1fffff,v);
	else if (addr<0x700000)
		ioWriteByte(addr&0x1f,v);
}

void TI92Plus::setmem_word(int addr,int v)
{
	setmem(addr,(v>>8)&0xff);
	setmem(addr+1,v&0xff);
}

void TI92Plus::setmem_dword(int addr,int v)
{
	setmem(addr,(v>>24)&0xff);
	setmem(addr+1,(v>>16)&0xff);
    setmem(addr+2,(v>>8)&0xff);
	setmem(addr+3,v&0xff);
}

int TI92Plus::readmem(int addr)
{
    addr&=0xffffff;
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=1;
    if ((flashProtect)&&(addr>=0x210000)&&(addr<=0x211fff))
        return 0x14;
    if (addr<0x200000)
		return (mem[addr>>16][addr&0xffff]);
	else if (addr<0x600000)
		return (mem[addr>>16][addr&0xffff])|romRetOr;
	else if (addr<0x700000)
		return (ioReadByte(addr&0x1f))&0xff;
}

int TI92Plus::readmem_word(int addr)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=1;
    if ((flashProtect)&&(addr>=0x210000)&&(addr<=0x211fff))
        return 0x1414;
	return ((getmem(addr)<<8)|getmem(addr+1))&0xffff;
}

int TI92Plus::readmem_dword(int addr)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_READ,0))
            DataBreak();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=1;
    if ((flashProtect)&&(addr>=0x210000)&&(addr<=0x211fff))
        return 0x14141414;
	return (getmem(addr)<<24)|
	       (getmem(addr+1)<<16)|
	       (getmem(addr+2)<<8)|
	       getmem(addr+3);
}

void TI92Plus::intRomWriteByte(int addr,int v)
{
    if (calc!=89) return;
    if (romWriteReady)
    {
//        if ((intRom[addr]==0xff)||(romWriteReady==1))
        {
            intRom[addr]=v;
            romChanged[addr>>16]=1;
        }
//        else
//            romWriteReady--;
        romWriteReady--;
        romRetOr=0xffffffff;
    }
    else if (v==0x50)
        romWritePhase=0x50;
    else if (v==0x10)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x51;
        else if (romWritePhase==0x51)
        {
            romWriteReady=2;
            romWritePhase=0x50;
        }
    }
    else if (v==0x20)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x20;
    }
    else if (v==0xd0)
    {
        if (romWritePhase==0x20)
        {
            romWritePhase=0xd0;
            romRetOr=0xffffffff;
            romErase=0xffffffff;
            romErasePhase=0;
            for (int i=0;i<0x10000;i++)
                intRom[(addr&0x1f0000)+i]=0xff;
            romChanged[addr>>16]=1;
        }
    }
    else if (v==0xff)
    {
        if (romWritePhase==0x50)
        {
            romWriteReady=0;
            romRetOr=0;
        }
    }
}

void TI92Plus::extRomWriteByte(int addr,int v)
{
    if ((calc<92)||(!plusMod)) return;
    if (romWriteReady)
    {
        if ((extRom[addr]==0xff)||(romWriteReady==1))
        {
            extRom[addr]=v;
            romChanged[addr>>16]=1;
        }
        else
            romWriteReady--;
        romWriteReady--;
        romRetOr=0xffffffff;
    }
    else if (v==0x50)
        romWritePhase=0x50;
    else if (v==0x10)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x51;
        else if (romWritePhase==0x51)
        {
            romWriteReady=2;
            romWritePhase=0x50;
        }
    }
    else if (v==0x20)
    {
        if (romWritePhase==0x50)
            romWritePhase=0x20;
    }
    else if (v==0xd0)
    {
        if (romWritePhase==0x20)
        {
            romWritePhase=0xd0;
            romRetOr=0xffffffff;
            romErase=0xffffffff;
            romErasePhase=0;
            for (int i=0;i<0x10000;i++)
                extRom[(addr&0x1f0000)+i]=0xff;
            romChanged[addr>>16]=1;
        }
    }
    else if (v==0xff)
    {
        if (romWritePhase==0x50)
        {
            romWriteReady=0;
            romRetOr=0;
        }
    }
}

void TI92Plus::writemem(int addr,int v)
{
    addr&=0xffffff;
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_WRITE,v))
            DataBreak();
    }
    if ((addr<0x100)&&(memprotect))
    {
        ::Exception(24+7,0);
        HandleException();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=0;
	if (addr<0x200000)
		ram[addr&memAnd]=v;
    else if (addr<0x400000)
        intRomWriteByte(addr&0x1fffff,v);
    else if (addr<0x600000)
        extRomWriteByte(addr&0x1fffff,v);
	else if (addr<0x700000)
		ioWriteByte(addr&0x1f,v);
}

void TI92Plus::writemem_word(int addr,int v)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_WRITE,v))
            DataBreak();
    }
    if ((addr<0x100)&&(memprotect))
    {
        ::Exception(24+7,0);
        HandleException();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=0;
	setmem(addr,(v>>8)&0xff);
	setmem(addr+1,v&0xff);
}

void TI92Plus::writemem_dword(int addr,int v)
{
    addr&=0xffffff;
    if (addr&1) { ::Exception(3,0); HandleException(); }
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,4,DATABREAK_WRITE,v))
            DataBreak();
    }
    if ((addr<0x100)&&(memprotect))
    {
        ::Exception(24+7,0);
        HandleException();
    }
    if ((addr>=0x1c0000)&&(addr<0x200000))
        flashProtect=0;
	setmem(addr,(v>>24)&0xff);
	setmem(addr+1,(v>>16)&0xff);
	setmem(addr+2,(v>>8)&0xff);
	setmem(addr+3,v&0xff);
}

int TI92Plus::readport(int port)
{
    return 0;
}

void TI92Plus::writeport(int port,int v)
{
}

void TI92Plus::Execute()
{
	static i=0;
	int j,f;

    if (!run) return;
    UpdateDebugCheckEnable();
   	i++;
    if (i>=8)
   	{
    	i=0;
	    int0Count++;
   		if (int0Count>3)
    	{
	    	int0Count=0;
   			IRQ(1);
    	}
	    if (int0Count==1) IRQ(4);
   		if (int0Count==2)
    	{
	    	for (j=0,f=0;j<10;j++)
   			{
    			if (kbmatrix[j]!=0xff)
	    			f=1;
   			}
    		if (f) { IRQ(2); }
	    }
   		lcdRefresh++;
    	if (lcdRefresh>15)
	    {
   			lcdRefresh=0;
    		RefreshLCD();
	    }
   		if (timer!=0)
    	{
	    	timer++;
   			timer&=0xff;
    	}
	    else
   		{
    		timer=timerInit;
   			IRQ(5);
    	}
   	}
    static int leftOver=0;
	leftOver=MC68000_Execute(625-leftOver)-(625-leftOver);
}

void TI92Plus::OneInstruction()
{
    UpdateDebugCheckEnable();
    MC68000_Execute(1);
}

void TI92Plus::OnUp()
{
    onheld=1;
}

void TI92Plus::OnDown()
{
    onheld=0;
    IRQ(6);
    if (cpuCompleteStop)
        cpuCompleteStop=0;
}

void TI92Plus::KeyUp(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]|=1<<c;
}

void TI92Plus::KeyDown(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]&=~(1<<c);
}

void TI92Plus::AlphaUp()
{
    KeyUp(0,7);
}

void TI92Plus::AlphaDown()
{
    KeyDown(0,7);
}

void TI92Plus::KBInt()
{
    IRQ(1);
}

int TI92Plus::GetKeyID(int i)
{
    return key[i];
}

void TI92Plus::SaveState(char *fn)
{
    FILE *fp=fopen(fn,"wb");
    fprintf(fp,"VTIv2.0 ");
    fwrite(romImage[currentROM].name,56,1,fp);
    MakeSR();
    fwrite(&regs,sizeof(regstruct),1,fp);
    fwrite(ram,256,1024,fp);
    fwrite(&memprotect,4,1,fp); fwrite(&ram128,4,1,fp);
    fwrite(&timer,4,1,fp); fwrite(&timerInit,4,1,fp);
    fwrite(&int0Count,4,1,fp);
    fwrite(&io0Bit7,4,1,fp); fwrite(&io0Bit2,4,1,fp);
    fwrite(&kbmask,4,1,fp);
    fwrite(&ramWrap,4,1,fp); fwrite(&(memAnd),4,1,fp);
    fwrite(&comError,4,1,fp);
    fwrite(&transflag,4,1,fp); fwrite(&transbyte,4,1,fp);
    fwrite(&transnotready,4,1,fp);
    fwrite(&recvflag,4,1,fp); fwrite(&recvbyte,4,1,fp);
    fwrite(&romWriteReady,4,1,fp); fwrite(&romWritePhase,4,1,fp);
    fwrite(&romRetOr,4,1,fp);
    fwrite(&lcdBase,4,1,fp); fwrite(&lcdOff,4,1,fp);
    fwrite(&contrast,4,1,fp);
    fwrite(&romErase,4,1,fp); fwrite(&romErasePhase,4,1,fp);
    fwrite(&cpuCompleteStop,4,1,fp);
    for (int i=0;i<32;i++)
    {
        fwrite(&romChanged[i],4,1,fp);
        if (romChanged[i])
            fwrite(&rom[i<<16],65536,1,fp);
    }
    fclose(fp);
}

void TI92Plus::LoadState(char *fn)
{
    FILE *fp=fopen(fn,"rb");
    if (!fp) return;
    char str[9],romName[56];
    fread(str,8,1,fp);
    str[8]=0;
    if (strcmp(str,"VTIv2.0 "))
        return;
    fread(romName,56,1,fp);
    if (strcmp(romName,romImage[currentROM].name))
        return;
    fread(&regs,sizeof(regstruct),1,fp);
    MakeFromSR();
    fread(ram,256,1024,fp);
    fread(&memprotect,4,1,fp); fread(&ram128,4,1,fp);
    fread(&timer,4,1,fp); fread(&timerInit,4,1,fp);
    fread(&int0Count,4,1,fp);
    fread(&io0Bit7,4,1,fp); fread(&io0Bit2,4,1,fp);
    fread(&kbmask,4,1,fp);
    fread(&ramWrap,4,1,fp); fread(&(memAnd),4,1,fp);
    fread(&comError,4,1,fp);
    fread(&transflag,4,1,fp); fread(&transbyte,4,1,fp);
    fread(&transnotready,4,1,fp);
    fread(&recvflag,4,1,fp); fread(&recvbyte,4,1,fp);
    fread(&romWriteReady,4,1,fp); fread(&romWritePhase,4,1,fp);
    fread(&romRetOr,4,1,fp);
    fread(&lcdBase,4,1,fp); fread(&lcdOff,4,1,fp);
    fread(&contrast,4,1,fp);
    fread(&romErase,4,1,fp); fread(&romErasePhase,4,1,fp);
    fread(&cpuCompleteStop,4,1,fp);
    for (int i=0;i<32;i++)
    {
        fread(&romChanged[i],4,1,fp);
        if (romChanged[i])
            fread(&rom[i<<16],65536,1,fp);
    }
    for (int i=0;i<32;i++)
        mem[i]=&ram[(i&(memAnd>>16))<<16];
    fclose(fp);
}

int TI92Plus::SendByte(int c)
{
    recvflag=1;
    recvbyte=c;
    IRQ(4);
    int count=10000;
    while ((recvflag)&&(count--))
        Execute();
    if (recvflag)
        return 0;
    return 1;
}

int TI92Plus::GetByte(int &c)
{
    int count=10000;
    while ((!transflag)&&(count--))
        Execute();
    if (transflag)
    {
        c=transbyte&0xff;
        transflag=0;
    }
    else
    {
        getError=1;
        return 0;
    }
    return 1;
}

int TI92Plus::SendReady()
{
    return !recvflag;
}

int TI92Plus::SendBuffer(int c)
{
    if (recvflag) return 0;
    recvflag=1;
    recvbyte=c;
    IRQ(4);
    return 1;
}

int TI92Plus::CheckBuffer()
{
    return transflag;
}

int TI92Plus::GetBuffer(int &c)
{
    if (!transflag) return 0;
    transflag=0;
    c=transbyte&0xff;
    return 1;
}

#define WaitForAck() \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }

int TI92Plus::SendFile(char *fn,int more)
{
	FILE *fp;
	int csum,type,size,id=0x89;
	int i,j,c,n,pos;
	char calcName[32],folderName[9],buf[16];
    unsigned char hdr[16];

    sendInProgress=1;
	for (i=0;i<100;i++)
        Execute();
	fp=fopen(fn,"rb");
	if (!fp)
		return 1;
    fseek(fp,0x3a,SEEK_SET);
    n=fgetc(fp);
    n|=fgetc(fp)<<8;
   	fseek(fp,0xa,SEEK_SET);
    fread(buf,8,1,fp);
  	buf[8]=0; strcpy(folderName,buf);
    pos=0x3c;
    for (;n;n--)
    {
        fseek(fp,pos,SEEK_SET);
        fread(hdr,16,1,fp);
        pos+=16;
	    type=hdr[0xc];
        memcpy(buf,&hdr[4],8);
        buf[8]=0;
        if (type==0x1f)
        {
            strcpy(folderName,buf);
            continue;
        }
	    fseek(fp,(hdr[0]&0xff)+((hdr[1]&0xff)<<8)+
            ((hdr[2]&0xff)<<16)+4,SEEK_SET);
    	size=fgetc(fp)<<8;
	    size|=fgetc(fp);
        strcpy(calcName,folderName);
	    strcat(calcName,"\\"); strcat(calcName,buf);
    	id=0x89;
	    SendByte(id); SendByte(6);
    	SendByte(6+strlen(calcName));
	    SendByte(0); SendByte((size+2)&0xff);
    	SendByte(((size+2)>>8)&0xff);
	    SendByte(0); SendByte(0);
    	csum=(size+2)&0xff; csum+=((size+2)>>8)&0xff;
	    SendByte(type);
    	csum+=type;
	    SendByte(strlen(calcName));
    	csum+=strlen(calcName);
	    for (i=0;i<strlen(calcName);i++)
    	{
	    	SendByte(calcName[i]);
    		csum+=calcName[i];
	    }
    	SendByte(csum&0xff); SendByte((csum>>8)&0xff);
	    WaitForAck();
    	WaitForAck();
	    SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
    	SendByte(id); SendByte(0x15);
	    SendByte((size+6)&0xff); SendByte(((size+6)>>8)&0xff);
    	SendByte(0); SendByte(0); SendByte(0); SendByte(0);
	    fseek(fp,(hdr[0]&0xff)+((hdr[1]&0xff)<<8)+
            ((hdr[2]&0xff)<<16)+4,SEEK_SET);
    	csum=0;
	    for (j=0;j<(size+2);j++)
    	{
	    	 i=fgetc(fp)&0xff;
    		 if (!SendByte(i)) { fclose(fp); sendInProgress=0; return 0; }
	    	 csum+=i;
    	}
	    csum&=0xffff;
    	SendByte(csum&0xff); SendByte((csum>>8)&0xff);
	    WaitForAck();
    	SendByte(id); SendByte(0x92); SendByte(0); SendByte(0);
	    WaitForAck();
	    for (i=0;i<1000;i++)
            Execute();
    }
   	fclose(fp);
    sendInProgress=0;
    return 1;
}

int TI92Plus::ReceiveFile()
{
    int i,j,c,len,size,type,id,csum,n,bufLen;
    int curOfs,folderSpecified,folderPos,group;
    char calcName[32],folder[9],name[9],*list,*buf,*newBuf;
    char oldFolder[9];

    for (i=0;i<10;i++) kbmatrix[i]=0xff;
    sendInProgress=1;
    getError=0;
    GetByte(c); id=c;
    GetByte(c); if (c!=6) { sendInProgress=0; return 0; }
    if (getError) { sendInProgress=0; getError=0; return 0; }
    Screen->Cursor=crHourGlass;
    oldFolder[0]=0; n=0; folderPos=-1;
    buf=NULL; bufLen=0;
    if (transferAsGroup)
        list=new char[32768];
    else
        list=NULL;
    while (1)
    {
        GetByte(c); len=c;
        GetByte(c); len|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); size=c;
        GetByte(c); size|=c<<8;
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); GetByte(c);
        GetByte(c); type=c;
        GetByte(c); j=c;
        if (j>8) { Screen->Cursor=crDefault; sendInProgress=0; return 0; }
        if ((len-6)>8) { Screen->Cursor=crDefault; sendInProgress=0; return 0; }
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        for (i=0;i<(len-6);i++)
        {
            GetByte(c);
            calcName[i]=c;
        }
        calcName[j]=0;
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
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
            if (list) delete[] list;
            return 0;
        }
        GetByte(c); GetByte(c); GetByte(c); GetByte(c);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        newBuf=new char[bufLen+size+6];
        if (buf)
        {
            memcpy(newBuf,buf,bufLen);
            delete[] buf;
        }
        buf=newBuf;
        curOfs=bufLen+4;
        buf[bufLen]=0; buf[bufLen+1]=0; buf[bufLen+2]=0;
        buf[bufLen+3]=0;
        csum=0;
        for (i=0;i<size;i++)
        {
            GetByte(c);
            buf[curOfs+i]=c;
            csum+=c&0xff;
            if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        }
        buf[curOfs+i]=csum&0xff;
        buf[curOfs+i+1]=csum>>8;
        bufLen+=size+6;
        GetByte(c); GetByte(c);
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        folder[5]=0x8b; folder[6]=0x76; folder[7]=0xa;
        if (!strchr(calcName,'\\'))
        {
            strcpy(folder,"main");
            memset(name,0,8);
            strcpy(name,calcName);
            folderSpecified=0;
        }
        else
        {
            *(strchr(calcName,'\\'))=0;
            strcpy(folder,calcName);
            memset(name,0,8);
            strcpy(name,strchr(calcName,0)+1);
            folderSpecified=1;
        }
        if (!transferAsGroup)
        {
            TSaveDialog *dlog=new TSaveDialog(EmuWnd);
            dlog->DefaultExt=(calc<94)?defExt92[type&0x1f]:
                defExt92p[type&0x1f];
            dlog->FileName=AnsiString(name)+AnsiString(".")+
                dlog->DefaultExt;
            dlog->FilterIndex=(calc<94)?6:7;
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
                FILE *fp=fopen(dlog->FileName.c_str(),"wb");
                if (calc==94)
                    fprintf(fp,"**TI92P*");
                else
                    fprintf(fp,"**TI92**");
                fputc(1,fp); fputc(0,fp);
                for (i=0;i<8;i++)
                    fputc(folder[i],fp);
                for (i=0;i<40;i++)
                    fputc(0,fp);
                fputc(1,fp); fputc(0,fp); fputc(0x52,fp);
                fputc(0,fp); fputc(0,fp); fputc(0,fp);
                for (i=0;i<8;i++)
                    fputc(name[i],fp);
                fputc(type,fp); fputc(0,fp); fputc(0,fp);
                fputc(0,fp);
                fputc((size+0x5a)&0xff,fp);
                fputc((size+0x5a)>>8,fp);
                fputc(0,fp); fputc(0,fp);
                fputc(0xa5,fp); fputc(0x5a,fp);
                for (i=0;i<bufLen;i++)
                    fputc(buf[i],fp);
                fclose(fp);
            }
            Screen->Cursor=crHourGlass;
            delete dlog;
            delete[] buf; buf=NULL; bufLen=0;
        }
        else
        {
            if (folderSpecified)
            {
                if (strcmp(folder,oldFolder))
                {
                    list[(n<<4)]=(curOfs-4)&0xff;
                    list[(n<<4)+1]=((curOfs-4)>>8)&0xff;
                    list[(n<<4)+2]=((curOfs-4)>>16)&0xff;
                    list[(n<<4)+3]=(curOfs-4)>>24;
                    memset(&list[(n<<4)+4],0,8);
                    memcpy(&list[(n<<4)+4],folder,strlen(folder));
                    list[(n<<4)+12]=0x1f;
                    list[(n<<4)+13]=0;
                    list[(n<<4)+14]=0;
                    list[(n<<4)+15]=0;
                    folderPos=n;
                    n++;
                    strcpy(oldFolder,folder);
                }
                if (folderPos!=-1)
                {
                    i=((list[(folderPos<<4)+14]&0xff)|((list[
                        (folderPos<<4)+15]&0xff)<<8))+1;
                    list[(folderPos<<4)+14]=i&0xff;
                    list[(folderPos<<4)+15]=i>>8;
                }
            }
            list[(n<<4)]=(curOfs-4)&0xff;
            list[(n<<4)+1]=((curOfs-4)>>8)&0xff;
            list[(n<<4)+2]=((curOfs-4)>>16)&0xff;
            list[(n<<4)+3]=(curOfs-4)>>24;
            memcpy(&list[(n<<4)+4],name,8);
            list[(n<<4)+12]=type;
            list[(n<<4)+13]=0;
            list[(n<<4)+14]=0;
            list[(n<<4)+15]=0;
            n++;
        }
        if (c!=6) break;
    }
    GetByte(c); GetByte(c);
    SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
    getError=0;
    Screen->Cursor=crDefault;
    if (transferAsGroup)
    {
        for (i=0;i<n;i++)
        {
            j=((list[(i<<4)]&0xff)|((list[(i<<4)+1]&
                0xff)<<8)|((list[(i<<4)+2]&0xff)<<16)|
                ((list[(i<<4)+3]&0xff)<<24))+(n<<4)+0x42;
            list[(i<<4)]=j&0xff;
            list[(i<<4)+1]=(j>>8)&0xff;
            list[(i<<4)+2]=(j>>16)&0xff;
            list[(i<<4)+3]=j>>24;
        }
        TSaveDialog *dlog=new TSaveDialog(EmuWnd);
        if ((n==1)||((n==2)&&(list[12]==0x1f)))
        {
            dlog->DefaultExt=(calc<94)?defExt92[type&0x1f]:
                defExt92p[type&0x1f];
            group=0;
            if (n==2)
            {
                memcpy(list,&list[16],16);
                n=1;
            }
        }
        else
        {
            dlog->DefaultExt=(calc<94)?"92g":"9xg";
            strcpy(name,"group");
            group=1;
        }
        dlog->FileName=AnsiString(name)+AnsiString(".")+
            dlog->DefaultExt;
        dlog->FilterIndex=(calc<94)?6:7;
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
            FILE *fp=fopen(dlog->FileName.c_str(),"wb");
            if (calc==94)
                fprintf(fp,"**TI92P*");
            else
                fprintf(fp,"**TI92**");
            fputc(1,fp); fputc(0,fp);
            if (group)
            {
                for (i=0;i<8;i++)
                    fputc(0,fp);
            }
            else
            {
                for (i=0;i<8;i++)
                    fputc(folder[i],fp);
            }
            for (i=0;i<40;i++)
                fputc(0,fp);
            fputc(n&0xff,fp); fputc(n>>8,fp);
            for (i=0;i<(n<<4);i++)
                fputc(list[i],fp);
            fputc((bufLen+(n<<4)+0x42)&0xff,fp);
            fputc(((bufLen+(n<<4)+0x42)>>8)&0xff,fp);
            fputc(((bufLen+(n<<4)+0x42)>>16)&0xff,fp);
            fputc((bufLen+(n<<4)+0x42)>>24,fp);
            fputc(0xa5,fp); fputc(0x5a,fp);
            for (i=0;i<bufLen;i++)
                fputc(buf[i],fp);
            fclose(fp);
        }
        delete dlog;
    }
    sendInProgress=0;
    if (list) delete[] list;
    if (buf) delete[] buf;
    return 1;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
