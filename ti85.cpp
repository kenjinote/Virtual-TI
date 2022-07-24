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
static int keymap85[]={VK_ESCAPE,k_esc,'1',k_1,'2',k_2,
    '3',k_3,'4',k_4,'5',k_5,'6',k_6,'7',k_7,
    '8',k_8,'9',k_9,'0',k_0,0xbd,k_neg,
    0xbb,k_equ,VK_BACK,k_left,VK_TAB,k_sto,'Q',
    k_q,'W',k_w,'E',k_e,'R',k_r,'T',k_t,'Y',k_y,
    'U',k_u,'I',k_i,'O',k_o,'P',k_p,0xdb,
    k_lparan,0xdd,k_rparan,VK_RETURN,k_enter1,
    'A',k_a,'S',k_s,'D',k_d,VK_CONTROL,k_dia,
    'F',k_f,'G',k_g,'H',k_h,'J',k_j,'K',k_k,
    'L',k_l,0xc0,k_mode,VK_SHIFT,k_shift,
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
    k_square,0xde,k_log,VK_END,k_ee,VK_NEXT,k_clear,-1,-1};

static int ti85kmat[][8]=
    {kn,kn,kn,kn,k_up,k_right,k_left,k_down,
     kn,k_clear,k_power,k_div,k_mult,k_minus,k_plus,k_enter1,
     kn,k_custom,k_tan,k_rparan,k_9,k_6,k_3,k_neg,
     k_bs,k_prgm,k_cos,k_lparan,k_8,k_5,k_2,k_dot,
     k_dia,k_table,k_sin,k_ee,k_7,k_4,k_1,k_0,
     k_hand,k_graph,k_log,k_ln,k_square,k_comma,k_sto,kn,
     k_mode,k_esc,k_2nd,k_f1,k_f2,k_f3,k_f4,k_f5,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int ti85alphakmat[][8]=
    {kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,k_e,k_j,k_o,k_t,k_x,kn,
     kn,kn,k_d,k_i,k_n,k_s,k_w,k_space,
     kn,kn,k_c,k_h,k_m,k_r,k_v,k_z,
     kn,kn,k_b,k_g,k_l,k_q,k_u,k_y,
     kn,kn,k_a,k_f,k_k,k_p,k_equ,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn,
     kn,kn,kn,kn,kn,kn,kn,kn};

static int key[]=
    {k_f1,k_f2,k_f3,k_f4,k_f5,
     k_2nd,k_esc,k_mode,k_left,k_right,
     k_up,k_down,k_hand,k_dia,k_bs,
     k_graph,k_table,k_prgm,k_custom,k_clear,
     k_log,k_sin,k_cos,k_tan,k_power,
     k_ln,k_ee,k_lparan,k_rparan,k_div,
     k_square,k_7,k_8,k_9,k_mult,
     k_comma,k_4,k_5,k_6,k_minus,
     k_sto,k_1,k_2,k_3,k_plus,
     k_on,k_0,k_dot,k_neg,k_enter1};

char* defExt85[0x20]={"85n","85c","85v","85v","85l",
    "85l","85m","85m","85k","85k","85e","85r","85s",
    "85d","85d","85d","85d","85i","85p","85r","85p",
    "85p","85p","85r","85r","85r","85r","85r","85p",
    "85b","85p","85p"};

ROMFunc romFuncs85[]=
   {{0x8c3c,"PROGRAM_ADDR"},{0x8c3e,"ROM_VERS"},
    {0x8c3f,"ZSHELL_VER"},{0x8c40,"ZS_BITS"},
    {0x8e8b,"ORGSP"},{0x8ea2,"USGSHELL"},
    {0x8eab,"VATName"},{0x8eb4,"PAGE1ADDR"},
    {0x8eb6,"PAGE2ADDR"},{0x8c08,"PROG_BYTE"},
    {0x33,"LD_HL_MHL"},{0x8e,"CP_HL_DE"},
    {0x9a,"UNPACK_HL"},{0x1b1,"STORE_KEY"},
    {0x1be,"GET_KEY"},{0x115,"UPDATE_APD"},
    {0x168,"READ_KEY"},{0x10000,"TX_CHARPUT"},
    {0x10001,"D_LT_STR"},{0x10002,"M_CHARPUT"},
    {0x10003,"D_ZM_STR"},{0x10004,"D_LM_STR"},
    {0x10005,"GET_T_CUR"},{0x10006,"SCROLL_UP"},
    {0x10007,"TR_CHARPUT"},{0x10008,"CLEARLCD"},
    {0x10009,"D_HL_DECI"},{0x1000a,"CLEARTEXT"},
    {0x1000b,"D_ZT_STR"},{0x1000c,"BUSY_OFF"},
    {0x1000d,"BUSY_ON"},{0x10080,"FIND_PIXEL"},
    {0x8000,"KEY_0"},{0x8001,"KEY_1"},
    {0x8002,"KEY_2"},{0x8004,"KEY_STAT"},
    {0x8006,"LAST_KEY"},{0x8007,"CONTRAST"},
    {0x800c,"CURSOR_ROW"},{0x800d,"CURSOR_COL"},
    {0x800e,"CURSOR_LET"},{0x8080,"BUSY_COUNTER"},
    {0x8081,"BUSY_BITMAP"},{0x80c6,"CURR_INPUT"},
    {0x80cc,"BYTES_USED"},{0x80df,"TEXT_MEM"},
    {0x81be,"CHECKSUM"},{0x8333,"CURSOR_X"},
    {0x8334,"CURSOR_Y"},{0x8346,"_IY_TABLE"},
    {0x8641,"GRAPH_MEM"},{0x8a6b,"TEXT_MEM2"},
    {0x8b1b,"USER_MEM"},{0x8b3a,"FIXED_POINT"},
    {0x8beb,"VAT_END"},{0xfa6f,"VAT_START"},
    {0xfc00,"VIDEO_MEM"},{0x8c41,"TX_CHARPUT"},
    {0x8c44,"D_LT_STR"},{0x8c47,"M_CHARPUT"},
    {0x8c4a,"D_ZM_STR"},{0x8c4d,"D_LM_STR"},
    {0x8c50,"SCROLL_UP"},{0x8c53,"TR_CHARPUT"},
    {0x8c56,"CLEARLCD"},{0x8c59,"D_HL_DECI"},
    {0x8c5c,"CLEARTEXT"},{0x8c5f,"D_ZT_STR"},
    {0x8c62,"BUSY_OFF"},{0x8c65,"BUSY_ON"},
    {0x8c68,"RANDOM"},{0x8c6b,"FIND_PIXEL"},
    {0x8c77,"FREEMEM"},{0x8c7a,"VAR_LENGTH"},
    {0x8c7d,"ASCIIZ_LEN"},{0x8c80,"NEG_BC"},
    {0x8c83,"MUL_HL"},{0x8c8c,"COPY_STRING"},
    {0x8c9b,"INT_INSTALL"},{0x8c9e,"INT_REMOVE"},
    {0x8ca1,"INT_CLEAN"},{0x8c95,"APPEND"},
    {0x8c98,"UNAPPEND"},{0x8ccb,"CHECK_APPEND"},
    {0x8ca4,"VAR_NEW"},{0x8ca7,"VAR_DELETE"},
    {0x8caa,"VAR_EXEC"},{0x8cad,"VAR_GET"},
    {0x8cb0,"VAR_RESIZE"},{0x8cce,"RELOC"},
    {0x8cd1,"DERELOC"},{0x8cd7,"RELOC_TAB"},
    {0x8cb3,"SEARCH_VAT"},{0x8cb6,"OTH_SHUTDOWN"},
    {0x8cb9,"DM_HL_DECI"},{0x8cbc,"OTH_PAUSE"},
    {0x8cbf,"OTH_CLEAR"},{0x8cc2,"OTH_EXIT"},
    {0x8cc5,"OTH_ARROW"},{0x8cd4,"OTH_FILL"},
    {0,NULL}};

TI85::TI85()
{
    romFuncs=romFuncs85;
    asm_readmem=debug_readmem;
    asm_readmem_word=debug_readmem_word;
    asm_readmem_dword=debug_readmem_dword;
    asm_writemem=debug_writemem;
    asm_writemem_word=debug_writemem_word;
    asm_writemem_dword=debug_writemem_dword;
}

void TI85::EnableDebug()
{
}

void TI85::DisableDebug()
{
}

int* TI85::GetKeyMap()
{
    return keymap85;
}

int TI85::GetKeyMat(int r,int c)
{
    return ti85kmat[r][c];
}

int TI85::GetKeyAlphaMat(int r,int c)
{
    return ti85alphakmat[r][c];
}

void TI85::Reset()
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
	Z80_Reset();
    R.PC.D=initialPC;
	contrast=16;
	kbmask=0xff;
    for (i=0;i<7;i++) kbmatrix[i]=0xff;
	onheld=1; lcdBase=0xfc00;
    for (i=0;i<6;i++) oldScreenBase[i]=lcdBase;
	lcdRefresh=0;
    cpuCompleteStop=0; lcdOff=1;
    pageA=rom; pageAType=0; pageANum=0;
    port3Left=100000;
    redWire=1; whiteWire=1;
    outRed=0; outWhite=0;
    intr=0; port6=0; getError=0;
    for (int i=0;romFuncs[i].name;i++)
        romFuncAddr[i]=romFuncs[i].num;
}

int TI85::getmem(int addr)
{
	if ((word)addr<0x4000)
		return (rom[addr&0x3fff])&0xff;
	else if ((word)addr<0x8000)
		return (pageA[addr&0x3fff])&0xff;
	else
		return (ram[addr&0x7fff])&0xff;
}

int TI85::getmem_word(int addr)
{
	return ((getmem(addr+1)<<8)|getmem(addr))&0xffff;
}

int TI85::getmem_dword(int addr)
{
	return (getmem(addr+3)<<24)|
	       (getmem(addr+2)<<16)|
	       (getmem(addr+1)<<8)|
	       getmem(addr);
}

void TI85::setmem(int addr,int v)
{
    addr&=0xffffff;
	if ((word)addr>=0x8000)
		ram[addr&0x7fff]=v;
}

void TI85::setmem_word(int addr,int v)
{
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

void TI85::setmem_dword(int addr,int v)
{
	setmem(addr+3,(v>>24)&0xff);
	setmem(addr+2,(v>>16)&0xff);
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

int TI85::readmem(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,1,DATABREAK_READ,0))
            DataBreak();
    }
	if ((word)addr<0x4000)
		return (rom[addr&0x3fff])&0xff;
	else if ((word)addr<0x8000)
		return (pageA[addr&0x3fff])&0xff;
	else
		return (ram[addr&0x7fff])&0xff;
}

int TI85::readmem_word(int addr)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_READ,0))
            DataBreak();
    }
	return ((getmem(addr+1)<<8)|getmem(addr))&0xffff;
}

int TI85::readmem_dword(int addr)
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

void TI85::writemem(int addr,int v)
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

void TI85::writemem_word(int addr,int v)
{
    if (debugCheckEnable)
    {
        if (CheckAddr(addr,2,DATABREAK_WRITE,v))
            DataBreak();
    }
	setmem(addr+1,(v>>8)&0xff);
	setmem(addr,v&0xff);
}

void TI85::writemem_dword(int addr,int v)
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

int TI85::readport(int port)
{
    int i,v;
    static int onpressed=0;
    if (CheckAddr((port&0xff)|0x600000,1,DATABREAK_READ,0)) DataBreak();
    switch(port&7)
    {
        case 1:
            for (v=0xff,i=0;i<7;i++)
			{
			    if (!(kbmask&(1<<i)))
			   	    v&=kbmatrix[i];
            }
			return v;
        case 3:
            if (port3Left)
            {
                port3Left--;
                return 1;
            }
            if (unstop)
                return (R.IFF1?0:2)|1;
            if ((!onpressed)&&(!onheld))
            {
                onpressed=1;
                return (onheld<<3)|(R.IFF1?0:2)|1;
            }
            if ((onpressed)&&(onheld))
                onpressed=0;
            return (onheld<<3)|(R.IFF1?0:2);
        case 4:
            return 1;
        case 5:
            return pageANum;
        case 6:
            return port6;
        case 7:
            return (outWhite<<3)|(outRed<<2)|
                ((whiteWire&(1-outWhite))<<1)|
                (redWire&(1-outRed));
    }
    return 0;
}

void TI85::writeport(int port,int v)
{
    if (CheckAddr((port&0xff)|0x600000,1,DATABREAK_WRITE,v)) DataBreak();
    v&=0xff;
    switch(port&7)
    {
        case 0:
            lcdBase=0xc000+((v&0x3f)<<8);
            break;
        case 1:
            kbmask=v;
            break;
        case 2:
            contrast=v&31;
            break;
        case 3:
            if (port3Left) return;
            lcdOff=1-((v>>3)&1);
            cpuCompleteStop=lcdOff&(v&1);
            if (!lcdOff)
                unstop=0;
            break;
        case 5:
            pageANum=v&0x7;
            pageA=&rom[pageANum<<14];
            break;
        case 6:
            port6=v;
            break;
        case 7:
            outRed=(v>>2)&1;
            outWhite=(v>>3)&1;
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
    }
}

void TI85::Execute()
{
    if (!run) return;
    UpdateDebugCheckEnable();
    intr+=375;
    if (intr>=33333)//100000)
    {
        RefreshLCD();
        intr=0;
        if (cpuCompleteStop) return;
        Z80_Cause_Interrupt(0);
    }
    if (cpuCompleteStop) return;
    static int leftOver=0;
	leftOver=Z80_Execute(375-leftOver)-(375-leftOver);
}

void TI85::OneInstruction()
{
    UpdateDebugCheckEnable();
    Z80_Execute(1);
}

void TI85::OnUp()
{
    onheld=1;
}

void TI85::OnDown()
{
    if (!onheld) return;
    onheld=0;
    if (cpuCompleteStop)
    {
        cpuCompleteStop=0;
        unstop=1;
    }
}

void TI85::KeyUp(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]|=1<<c;
}

void TI85::KeyDown(int r,int c)
{
    if (r!=-1)
        kbmatrix[r]&=~(1<<c);
}

void TI85::AlphaUp()
{
    KeyUp(5,7);
}

void TI85::AlphaDown()
{
    KeyDown(5,7);
}

void TI85::KBInt()
{
    for (int i=0;i<200;i++)
        Execute();
}

int TI85::GetKeyID(int i)
{
    return key[i];
}

void TI85::SaveState(char *fn)
{
    FILE *fp=fopen(fn,"wb");
    fprintf(fp,"VTIv2.0 ");
    fwrite(romImage[currentROM].name,56,1,fp);
    fwrite(&R,sizeof(Z80_Regs),1,fp);
    fwrite(ram,32,1024,fp);
    fwrite(&kbmask,4,1,fp);
    fwrite(&pageAType,4,1,fp); fwrite(&pageANum,4,1,fp);
    fwrite(&port3Left,4,1,fp); fwrite(&intr,4,1,fp);
    fwrite(&port6,4,1,fp);
    fwrite(&redWire,4,1,fp); fwrite(&whiteWire,4,1,fp);
    fwrite(&outRed,4,1,fp); fwrite(&outWhite,4,1,fp);
    fwrite(&lcdBase,4,1,fp); fwrite(&lcdOff,4,1,fp);
    fwrite(&contrast,4,1,fp); fwrite(&unstop,4,1,fp);
    fwrite(&cpuCompleteStop,4,1,fp);
    fclose(fp);
}

void TI85::LoadState(char *fn)
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
    fread(&R,sizeof(Z80_Regs),1,fp);
    fread(ram,32,1024,fp);
    fread(&kbmask,4,1,fp);
    fread(&pageAType,4,1,fp); fread(&pageANum,4,1,fp);
    pageA=&rom[pageANum<<14];
    fread(&port3Left,4,1,fp); fread(&intr,4,1,fp);
    fread(&port6,4,1,fp);
    fread(&redWire,4,1,fp); fread(&whiteWire,4,1,fp);
    fread(&outRed,4,1,fp); fread(&outWhite,4,1,fp);
    fread(&lcdBase,4,1,fp); fread(&lcdOff,4,1,fp);
    fread(&contrast,4,1,fp); fread(&unstop,4,1,fp);
    fread(&cpuCompleteStop,4,1,fp);
    fclose(fp);
}

int TI85::SendByte(int c)
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
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
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
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
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
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
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
                if (intr>=100000)
                {
                    RefreshLCD();
                    intr=0;
                    Z80_Cause_Interrupt(0);
                }
            }
            if (!left) return 0;
        }
    }
    return 1;
}

int TI85::GetByte(int &c)
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
            if (intr>=100000)
            {
                RefreshLCD();
                intr=0;
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
            if (intr>=100000)
            {
                RefreshLCD();
                intr=0;
                Z80_Cause_Interrupt(0);
            }
        }
        if (!left) { getError=1; return 0; }
        whiteWire=1; redWire=1;
        left=200000;
    }
    return 1;
}

int TI85::SendReady()
{
    return 1;
}

int TI85::SendBuffer(int c)
{
    return SendByte(c);
}

int TI85::CheckBuffer()
{
    if ((redWire&(1-outRed))&&(whiteWire&(1-outWhite)))
        return 0;
    return 1;
}

int TI85::GetBuffer(int &c)
{
    return GetByte(c);
}

#define WaitForAck() \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; } \
    if (!GetByte(c)) { fclose(fp); sendInProgress=0; return 0; }

int TI85::SendFile(char *fn,int more)
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
        if (type==0x1d)
        {
            len2=0; fread(&len2,2,1,fp);
            len3=0; fread(&len3,2,1,fp);
            fseek(fp,2,SEEK_CUR);
            whiteWire=1; redWire=1; KBInt();
            SendByte(0x85); SendByte(0x6);
            SendByte(9); SendByte(0);
            SendByte(len&0xff); SendByte(len>>8);
            SendByte(0x1d);
            SendByte(len2&0xff); SendByte(len2>>8);
            SendByte(len3&0xff); SendByte(len3>>8);
            SendByte(0xf7); SendByte(0x8b);
            csum=(len&0xff)+(len>>8)+0x1d+(len2&0xff)+
                (len2>>8)+(len3&0xff)+(len3>>8)+0xf7+0x8b;
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
            SendByte(0x85); SendByte(0x56); SendByte(0); SendByte(0);
            SendByte(0x85); SendByte(0x15);
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
            SendByte(0x85); SendByte(0x15);
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
            SendByte(0x85); SendByte(0x15);
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
        nameLen=0; fread(&nameLen,1,1,fp);
        for (i=0;i<8;i++) name[i]=0;
        fread(name,hdr-4,1,fp);
        fseek(fp,2,SEEK_CUR);
        whiteWire=1; redWire=1; KBInt();
        SendByte(0x85); SendByte(0x6);
        SendByte(nameLen+4); SendByte(0);
        SendByte(len&0xff); SendByte(len>>8);
        SendByte(type); SendByte(nameLen);
        csum=(len&0xff)+(len>>8)+type+nameLen;
        for (i=0;i<8;i++)
        {
            c=(int)((unsigned char)name[i]);
            if (c==0) break;
            SendByte(c);
            csum+=c;
        }
        SendByte(csum&0xff); SendByte((csum>>8)&0xff);
        WaitForAck(); WaitForAck();
        whiteWire=1; redWire=1; KBInt();
        SendByte(0x85); SendByte(0x56); SendByte(0); SendByte(0);
        SendByte(0x85); SendByte(0x15);
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
    if (!more)
    {
        SendByte(0x85); SendByte(0x92); SendByte(0); SendByte(0);
        WaitForAck();
    }
    fclose(fp);
    sendInProgress=0;
    return 1;
}

int TI85::ReceiveFile()
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
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        GetByte(c); type=c;
        GetByte(c); j=c;
        if (j>8) { Screen->Cursor=crDefault; sendInProgress=0; return 0; }
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        memset(name,0,8);
        for (i=0;i<j;i++)
        {
            GetByte(c);
            name[i]=c;
        }
        name[j]=0;
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
        curOfs=bufLen+8+strlen(name);
        buf[bufLen]=4+strlen(name); buf[bufLen+1]=0;
        buf[bufLen+2]=size&0xff; buf[bufLen+3]=size>>8;
        buf[bufLen+4]=type; buf[bufLen+5]=strlen(name);
        for (i=0;i<strlen(name);i++)
            buf[bufLen+6+i]=name[i];
        buf[bufLen+6+i]=size&0xff; buf[bufLen+7+i]=size>>8;
        for (i=0;i<size;i++)
        {
            GetByte(c);
            buf[curOfs+i]=c;
            if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        }
        bufLen+=size+8+strlen(name);
        GetByte(c); GetByte(c);
        whiteWire=1; redWire=1; KBInt();
        SendByte(id); SendByte(0x56); SendByte(0); SendByte(0);
        GetByte(c); GetByte(c);
        if (getError) { Screen->Cursor=crDefault; sendInProgress=0; getError=0; return 0; }
        if (!transferAsGroup)
        {
            TSaveDialog *dlog=new TSaveDialog(EmuWnd);
            dlog->DefaultExt=defExt85[type&0x1f];
            dlog->FileName=AnsiString(name)+AnsiString(".")+
                dlog->DefaultExt;
            dlog->FilterIndex=3;
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
                fprintf(fp,"**TI85**");
                fputc(0x1a,fp); fputc(0xc,fp);
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
            dlog->DefaultExt=defExt85[type&0x1f];
            group=0;
        }
        else
        {
            dlog->DefaultExt="85g";
            strcpy(name,"group");
            group=1;
        }
        dlog->FileName=AnsiString(name)+AnsiString(".")+
            dlog->DefaultExt;
        dlog->FilterIndex=3;
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
            fprintf(fp,"**TI85**");
            fputc(0x1a,fp); fputc(0xc,fp);
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
