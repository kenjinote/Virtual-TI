//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "Debug.h"
#define NOTYPEREDEF
#include "cpudefs.h"
#include "gotodialog.h"
#include "Value.h"
#include "Entry.h"
#include "Data.h"
#include "Search.h"
#include "Log.h"
#include "EditLog.h"
#include "AddLog.h"
#include "calchw.h"
#include "rom.h"
#include "z80.h"
#include "z80dasm.h"
#include "Trap.h"
#include "Emu.h"

#define ROMTBL(x) hw->getmem_dword(hw->getmem_dword(0xc8)+(x)*4)
#define HANDLE_TABLE hw->getmem_dword(ROMTBL(0x2f)+0x11a)
#define HANDLE_TABLE_92 cpu_readmem24_dword(romFuncAddr[0x1c]+0x1902)
#define HANDLE(x) hw->getmem_dword(HANDLE_TABLE+4*(x))
#define HANDLE92(x) hw->getmem_dword(HANDLE_TABLE_92+4*(x))
#define GETBYTE(x) (cpu_readmem24(x)&0xff)
#define GETWORD(x) (cpu_readmem24_word(x)&0xffff)

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TDebugWnd *DebugWnd;
int debug=0;
int run=1;
int debugStartPC,debugStartMem;
int debugCheckEnable;
int debugEnforceAddr=0;
int debugFirstInstrucLen;
int debugCodeBreakCount,debugCodeBreak[256];
int debugDataBreakCount,debugDataBreakType[256];
int debugDataBreakLow[256],debugDataBreakHigh[256];
int debugDataBreak;
int debugProgBreak,debugProgBreakOfs,debugProgLen;
int debugBeginStepOver,debugEndStepOver;
int debugStepOver,debugStepGoal;
int debugStepA7;
int debugForceUpdate;
int debugLogEnable;
int debugLogCount;
int debugLogClass[256];
int debugLogType[256];
int debugLogLow[256];
int debugLogHigh[256];
int debugLogCheck[256];
int debugLogLast[256];
int debugLogState[256];
int debugLogAny[256];
int debugLogLinkSend,debugLogLinkRecv;
int debugException=0;
int debugRunToCursor=0;
int debugCursorAddr;
int exceptBPCount,exceptBP[48];
//extern int readmem24(int a);
//extern int readmem24_word(int a);
//extern int readmem24_dword(int a);
//extern int writemem24(int a,int v);
//extern int writemem24_word(int a,int v);
//extern int writemem24_dword(int a,int v);
#define UseDebugReadFuncs() hw->EnableDebug()
#define RestoreReadFuncs() hw->DisableDebug()
//extern void UseDebugReadFuncs();
//extern void RestoreReadFuncs();
extern void RefreshLCDNow();
extern void PauseStart();
extern void PauseEnd();
extern int Dasm68000 (unsigned char *pBase, char *buffer, int pc);
extern int lcdUpdate;
extern int calc,plusMod;
int oldHandleCount=0;
int oldHandleList[0x800];
extern Z80_Regs R;
volatile extern int emuRunning;

volatile extern int lcdUpdateNeeded;
extern HANDLE lcdUpdateEvent;

static char tmpStr[64];

static char* exceptionName[48]={"","","Bus error","Address error",
    "Illegal instruction","Divide by zero","CHK instruction",
    "TRAPV instruction","Privilege violation","Trace",
    "Line 1010 emulator","Line 1111 emulator","","","","","",
    "","","","","","","","Auto-int 0","Auto-int 1","Auto-int 2",
    "Auto-int 3","Auto-int 4","Auto-int 5","Auto-int 6",
    "Auto-int 7","Trap 0","Trap 1","Trap 2","Trap 3","Trap 4",
    "Trap 5","Trap 6","Trap 7","Trap 8","Trap 9","Trap 10",
    "Trap 11","Trap 12","Trap 13","Trap 14","Trap 15"};

int db92Count;
DWORD db92Addr[MAX_DEBUG_FILES];
WORD db92VarOfs[MAX_DEBUG_FILES];
WORD db92LblOfs[MAX_DEBUG_FILES];

int inline IsROMAddr(int addr)
{
    if (calc<89) return IsROMAddrZ80(addr);
    if (addr==0) return 0;
    if (romFuncs)
    {
        for (int i=0;romFuncs[i].name;i++)
        {
            if (addr==romFuncAddr[i])
                return 1;
        }
    }
    if ((calc!=89)&&(calc!=94))
        return 0;
    for (int i=0;i<db92Count;i++)
    {
        DWORD base=hw->getmem_dword(db92Addr[i])+2;
        WORD ptr=db92VarOfs[i];
        for (;;)
        {
            if (!hw->getmem_word(base+ptr))
                break;
            WORD fileAddr=ptr+hw->getmem_word(base+ptr);
            if (((DWORD)fileAddr+base)==addr)
                return 1;
            ptr+=hw->getmem_word(base+ptr+2)+2;
        }
        ptr=db92LblOfs[i];
        for (;;)
        {
            if (!hw->getmem_word(base+ptr))
                break;
            WORD fileAddr=ptr+hw->getmem_word(base+ptr);
            if (((DWORD)fileAddr+base)==addr)
                return 1;
            ptr+=hw->getmem_word(base+ptr+2)+2;
        }
    }
    return 0;
}

char inline *ROMAddrName(int addr)
{
    if (calc<89) return ROMAddrNameZ80(addr);
    if (romFuncs)
    {
        for (int i=0;romFuncs[i].name;i++)
        {
            if (addr==romFuncAddr[i])
            {
                sprintf(tmpStr,"tios::%s",romFuncs[i].name);
                return tmpStr;
            }
        }
    }
    if ((calc!=89)&&(calc!=94))
        return "Internal Error";
    for (int i=0;i<db92Count;i++)
    {
        DWORD base=hw->getmem_dword(db92Addr[i])+2;
        WORD ptr=db92VarOfs[i];
        for (;;)
        {
            if (!hw->getmem_word(base+ptr))
                break;
            WORD fileAddr=ptr+hw->getmem_word(base+ptr);
            if (((DWORD)fileAddr+base)==addr)
            {
                memset(tmpStr,0,64);
                for (int j=0;j<(hw->getmem_word(base+ptr+2)-3);j++)
                    tmpStr[j]=hw->getmem(base+ptr+5+j);
                return tmpStr;
            }
            ptr+=hw->getmem_word(base+ptr+2)+2;
        }
        ptr=db92LblOfs[i];
        for (;;)
        {
            if (!hw->getmem_word(base+ptr))
                break;
            WORD fileAddr=ptr+hw->getmem_word(base+ptr);
            if (((DWORD)fileAddr+base)==addr)
            {
                memset(tmpStr,0,64);
                for (int j=0;j<(hw->getmem_word(base+ptr+2)-2);j++)
                    tmpStr[j]=hw->getmem(base+ptr+4+j);
                return tmpStr;
            }
            ptr+=hw->getmem_word(base+ptr+2)+2;
        }
    }
    return "Internal Error";
}

//---------------------------------------------------------------------------
__fastcall TDebugWnd::TDebugWnd(TComponent* Owner)
    : TForm(Owner)
{
    debugStartMem=0;
    debugProgBreak=0;
    debugCodeBreakCount=0;
    debugDataBreakCount=0;
    debugDataBreak=0;
    debugStepOver=0;
    debugBeginStepOver=0;
    debugForceUpdate=1;
    debugLogEnable=0;
    debugLogCount=0;
    debugLogLinkSend=0;
    debugLogLinkRecv=0;
}

void UpdateDB92PSymFiles()
{
    db92Count=0;
    int addr=HANDLE(8)+4;
    if ((addr<0x4c00)||(addr>0x3ffff))
        return;
    for (int i=0;i<GETWORD(addr-2);i++)
    {
        int fa=HANDLE(GETWORD(addr+14*i+12))+4;
        for (int j=0;j<GETWORD(fa-2);j++)
        {
            int pa=HANDLE(GETWORD(fa+14*j+12));
            if (GETBYTE(pa+1+GETWORD(pa))==0xf3)
            {
                if ((GETWORD(pa+6)==0x3638)&&
                    (GETBYTE(pa+8)==0x6b))
                {
                    int mainPtr=pa+2+GETWORD(pa+0xe);
                    if ((GETWORD(mainPtr-4)==0x6469)&&
                        (GETWORD(mainPtr-2)==0x6e66))
                    {
                        db92Addr[db92Count]=HANDLE_TABLE+
                            (GETWORD(fa+14*j+12))*4;
                        WORD mp=(WORD)(mainPtr-(pa+2));
                        db92VarOfs[db92Count]=(mp-8)+
                            GETWORD(mainPtr-8);
                        db92LblOfs[db92Count++]=(mp-6)+
                            GETWORD(mainPtr-6);
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
extern int romWriteReady,romWritePhase;
void __fastcall TDebugWnd::Update()
{
    char str[256],str2[256];
    int useOldPC=0;

    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_ENTER_DEBUG,0,0);
    if (!debugException)
        sprintf(str,"Debugger");
    else
        sprintf(str,"Debugger - [%s]",exceptionName[debugException]);
    Caption=str;
    PauseStart();
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    if (z80)
        Setbreakpointontrapexception1->Enabled=false;
    else
        Setbreakpointontrapexception1->Enabled=true;
    if ((calc==89)||(calc==94))
        UpdateDB92PSymFiles();
    if ((!run)&&debugStepOver)
        debugStepOver=0;
    if ((!run)&&debugDataBreak)
    {
        debugDataBreak=0;
        useOldPC=1;
    }
    lcdUpdate=1;
    int chg,i,j,k,max,n=0;
    if ((calc==89)||(calc==94))
    {
        HandlesBox->Caption="Handles";
        max=hw->getmem_word(ROMTBL(0x2f)+0x104);
        if (max>0x800) max=0x800;
        for (i=0,chg=0;i<max;i++)
        {
            int addr=hw->getmem_dword(HANDLE_TABLE+4*i);
            if (addr)
            {
                n=i+1;
                if (addr!=oldHandleList[i])
                    chg=1;
            }
        }
        if ((n!=oldHandleCount)||chg)
        {
            Handles->Clear();
            for (i=0;i<n;i++)
            {
                int addr=hw->getmem_dword(HANDLE_TABLE+4*i);
                sprintf(str,"$%4.4X -> $%6.6X",i,addr);
                oldHandleList[i]=addr;
                Handles->Items->Add(str);
            }
            oldHandleCount=n;
        }
    }
    else if (calc==92)
    {
        HandlesBox->Caption="Handles";
        max=hw->getmem_word(0x5d2c);
        if (max>0x800) max=0x800;
        for (i=0,chg=0;i<max;i++)
        {
            int addr=hw->getmem_dword(hw->getmem_dword(0x5d42)+4*i);
            if (addr)
            {
                n=i+1;
                if (addr!=oldHandleList[i])
                    chg=1;
            }
        }
        if ((n!=oldHandleCount)||chg)
        {
            Handles->Clear();
            for (i=0;i<n;i++)
            {
                int addr=hw->getmem_dword(hw->getmem_dword(0x5d42)+4*i);
                sprintf(str,"$%4.4X -> $%6.6X",i,addr);
                oldHandleList[i]=addr;
                Handles->Items->Add(str);
            }
            oldHandleCount=n;
        }
    }
    else if (calc==93)
    {
        HandlesBox->Caption="Handles";
        max=hw->getmem_word(0x600c);
        if (max>0x800) max=0x800;
        for (i=0,chg=0;i<max;i++)
        {
            int addr=hw->getmem_dword(hw->getmem_dword(0x6022)+4*i);
            if (addr)
            {
                n=i+1;
                if (addr!=oldHandleList[i])
                    chg=1;
            }
        }
        if ((n!=oldHandleCount)||chg)
        {
            Handles->Clear();
            for (i=0;i<n;i++)
            {
                int addr=hw->getmem_dword(hw->getmem_dword(0x6022)+4*i);
                sprintf(str,"$%4.4X -> $%6.6X",i,addr);
                oldHandleList[i]=addr;
                Handles->Items->Add(str);
            }
            oldHandleCount=n;
        }
    }
    else if (calc==84)
    {
        if (!Handles->Items->Count)
            oldHandleList[1]=oldHandleList[0]-1;
        else
        {
            oldHandleList[1]=0;
            for (i=0xfe66,j=0;j<oldHandleCount;j++)
                oldHandleList[1]+=hw->getmem(i--)&0xff;
        }
        if (oldHandleList[1]!=oldHandleList[0])
        {
            HandlesBox->Caption="Variables";
            Handles->Clear();
            int prgm=0;
            for (i=0xfe66,j=0;;)
            {
                int type=hw->getmem(i--)&0xf;
                i-=2;
                if ((type==5)||(type==6))
                {
                    prgm=1;
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    k=hw->getmem(i--);
                    char name[9];
                    for (j=0;j<k;j++)
                        name[j]=hw->getmem(i--);
                    if ((name[0]<'0')&&(k==1))
                        continue;
                    name[k]=0;
                    sprintf(str,"Prgm %-8s->%4.4X",name,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==3)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    i--;
                    if (tkn1==0x5c)
                    {
                        sprintf(str,"YVar [%c]     ->%4.4X",'A'+tkn2,addr);
                        Handles->Items->Insert(0,str);
                    }
                    else if (tkn1==0x5e)
                    {
                        if ((tkn2&0xf0)==0x10)
                        {
                            sprintf(str,"YVar Y%-2d     ->%4.4X",(tkn2&0xf)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if ((tkn2&0xf1)==0x20)
                        {
                            sprintf(str,"YVar Xt%-2d    ->%4.4X",((tkn2&0xf)>>1)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if ((tkn2&0xf1)==0x21)
                        {
                            sprintf(str,"YVar Yt%-2d    ->%4.4X",((tkn2&0xf)>>1)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if ((tkn2&0xf0)==0x40)
                        {
                            sprintf(str,"YVar r%-2d     ->%4.4X",(tkn2&0xf)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if (tkn2==0x80)
                        {
                            sprintf(str,"YVar Un      ->%4.4X",addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if (tkn2==0x81)
                        {
                            sprintf(str,"YVar Vn      ->%4.4X",addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if (tkn2==0x82)
                        {
                            sprintf(str,"YVar Wn      ->%4.4X",addr);
                            Handles->Items->Insert(0,str);
                        }
                        else
                            continue;
                    }
                }
                else if (type==0)
                {
                    if (prgm) break;
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    if (tkn1==0)
                        break;
                    i--;
                    if (tkn1!=0x5b)
                        sprintf(str,"Real %c       ->%4.4X",tkn1,addr);
                    else
                        sprintf(str,"Real theta   ->%4.4X",addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==1)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    int len=hw->getmem(i--);
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    i-=len-2;
                    if (tkn2>10)
                        continue;
                    if (tkn1!=0x5d)
                        continue;
                    if (tkn2==9) tkn2=-1;
                    sprintf(str,"List L%d      ->%4.4X",tkn2+1,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==2)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    i--;
                    if (tkn2>10)
                        continue;
                    if (tkn1!=0x5c)
                        continue;
                    sprintf(str,"Mat  [%c]     ->%4.4X",tkn2+'A',addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==7)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    if (tkn1!=0x60)
                        continue;
                    i--;
                    if (tkn2==9) tkn2=-1;
                    sprintf(str,"Pic  Pic%d    ->%4.4X",tkn2+1,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==8)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    i--;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    if (tkn1!=0x61)
                        continue;
                    i--;
                    if (tkn2==9) tkn2=-1;
                    sprintf(str,"GDB  GDB%d    ->%4.4X",tkn2+1,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==4)
                    i-=5;
                else
                    break;
            }
            oldHandleCount=0xfe66-i;
            oldHandleList[0]=0;
            for (i=0xfe66,j=0;j<oldHandleCount;j++)
                oldHandleList[0]+=hw->getmem(i--)&0xff;
        }
    }
    else if ((calc==82)||(calc==83))
    {
        if (!Handles->Items->Count)
            oldHandleList[1]=oldHandleList[0]-1;
        else
        {
            oldHandleList[1]=0;
            for (i=0xfe6e,j=0;j<oldHandleCount;j++)
                oldHandleList[1]+=hw->getmem(i--)&0xff;
        }
        if (oldHandleList[1]!=oldHandleList[0])
        {
            HandlesBox->Caption="Variables";
            Handles->Clear();
            int prgm=0;
            for (i=0xfe6e,j=0;i>hw->getmem_word(0x8d10);)
            {
                int type=hw->getmem(i--)&0xf;
                if ((type==5)||(type==6))
                {
                    prgm=1;
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    k=hw->getmem(i--);
                    char name[9];
                    for (j=0;j<k;j++)
                        name[j]=hw->getmem(i--);
                    if ((name[0]<'0')&&(k==1))
                        continue;
                    name[k]=0;
                    sprintf(str,"Prgm %-8s->%4.4X",name,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==3)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    i--;
                    if (tkn1==0x5c)
                    {
                        sprintf(str,"YVar [%c]     ->%4.4X",'A'+tkn2,addr);
                        Handles->Items->Insert(0,str);
                    }
                    else if (tkn1==0x5e)
                    {
                        if ((tkn2&0xf0)==0x10)
                        {
                            sprintf(str,"YVar Y%-2d     ->%4.4X",(tkn2&0xf)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if ((tkn2&0xf1)==0x20)
                        {
                            sprintf(str,"YVar Xt%-2d    ->%4.4X",((tkn2&0xf)>>1)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if ((tkn2&0xf1)==0x21)
                        {
                            sprintf(str,"YVar Yt%-2d    ->%4.4X",((tkn2&0xf)>>1)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if ((tkn2&0xf0)==0x40)
                        {
                            sprintf(str,"YVar r%-2d     ->%4.4X",(tkn2&0xf)+1,addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if (tkn2==0x80)
                        {
                            sprintf(str,"YVar Un      ->%4.4X",addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if (tkn2==0x81)
                        {
                            sprintf(str,"YVar Vn      ->%4.4X",addr);
                            Handles->Items->Insert(0,str);
                        }
                        else if (tkn2==0x82)
                        {
                            sprintf(str,"YVar Wn      ->%4.4X",addr);
                            Handles->Items->Insert(0,str);
                        }
                        else
                            continue;
                    }
                }
                else if (type==0)
                {
                    if (prgm) break;
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    if (tkn1==0)
                        break;
                    i--;
                    if (tkn1!=0x5b)
                        sprintf(str,"Real %c       ->%4.4X",tkn1,addr);
                    else
                        sprintf(str,"Real theta   ->%4.4X",addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==1)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    if (calc==83) i--;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    i--;
                    if (tkn2>10)
                        continue;
                    if (tkn1!=0x5d)
                        continue;
                    if (tkn2==9) tkn2=-1;
                    sprintf(str,"List L%d      ->%4.4X",tkn2+1,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==2)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    i--;
                    if (tkn2>10)
                        continue;
                    if (tkn1!=0x5c)
                        continue;
                    sprintf(str,"Mat  [%c]     ->%4.4X",tkn2+'A',addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==7)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    if (tkn1!=0x60)
                        continue;
                    i--;
                    if (tkn2==9) tkn2=-1;
                    sprintf(str,"Pic  Pic%d    ->%4.4X",tkn2+1,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==8)
                {
                    int addr=hw->getmem(i--);
                    addr|=hw->getmem(i--)<<8;
                    int tkn1=hw->getmem(i--);
                    int tkn2=hw->getmem(i--);
                    if (tkn1!=0x61)
                        continue;
                    i--;
                    if (tkn2==9) tkn2=-1;
                    sprintf(str,"GDB  GDB%d    ->%4.4X",tkn2+1,addr);
                    Handles->Items->Insert(0,str);
                }
                else if (type==4)
                    i-=5;
                else
                    break;
            }
            oldHandleCount=0xfe6e-i;
            oldHandleList[0]=0;
            for (i=0xfe6e,j=0;j<oldHandleCount;j++)
                oldHandleList[0]+=hw->getmem(i--)&0xff;
        }
    }
    else if (calc==85)
    {
        if (!Handles->Items->Count)
            oldHandleList[1]=oldHandleList[0]-1;
        else
        {
            oldHandleList[1]=0;
            for (i=0xfa6f,j=0;j<oldHandleCount;j++)
                oldHandleList[1]+=hw->getmem(i--)&0xff;
        }
        if (oldHandleList[1]!=oldHandleList[0])
        {
            HandlesBox->Caption="Variables";
            Handles->Clear();
            for (i=0xfa6f,j=0;i>hw->getmem_word(0x8be5);)
            {
                int type=hw->getmem(i--);
                int addr=hw->getmem(i--);
                addr|=hw->getmem(i--)<<8;
                k=hw->getmem(i--);
                char name[9];
                for (j=0;j<k;j++)
                    name[j]=hw->getmem(i--);
                name[k]=0;
                if ((name[0]<'0')&&(k==1))
                    continue;
                switch(type)
                {
                    case 0:
                        sprintf(str,"Real %-8s->%4.4X",name,addr);
                        break;
                    case 1:
                        sprintf(str,"Cplx %-8s->%4.4X",name,addr);
                        break;
                    case 2:
                        sprintf(str,"Vect %-8s->%4.4X",name,addr);
                        break;
                    case 3:
                        sprintf(str,"VctC %-8s->%4.4X",name,addr);
                        break;
                    case 4:
                        sprintf(str,"List %-8s->%4.4X",name,addr);
                        break;
                    case 5:
                        sprintf(str,"LstC %-8s->%4.4X",name,addr);
                        break;
                    case 6:
                        sprintf(str,"Mat  %-8s->%4.4X",name,addr);
                        break;
                    case 7:
                        sprintf(str,"MatC %-8s->%4.4X",name,addr);
                        break;
                    case 8:
                        sprintf(str,"Cnst %-8s->%4.4X",name,addr);
                        break;
                    case 9:
                        sprintf(str,"CstC %-8s->%4.4X",name,addr);
                        break;
                    case 0xa:
                        sprintf(str,"Equ  %-8s->%4.4X",name,addr);
                        break;
                    case 0xb:
                        sprintf(str,"Rng  %-8s->%4.4X",name,addr);
                        break;
                    case 0xc:
                        sprintf(str,"Str  %-8s->%4.4X",name,addr);
                        break;
                    case 0xd:
                    case 0xe:
                    case 0xf:
                    case 0x10:
                        sprintf(str,"GDB  %-8s->%4.4X",name,addr);
                        break;
                    case 0x11:
                        sprintf(str,"Pic  %-8s->%4.4X",name,addr);
                        break;
                    case 0x12:
                        sprintf(str,"Prgm %-8s->%4.4X",name,addr);
                        break;
                    case 0x13:
                        sprintf(str,"Rng  %-8s->%4.4X",name,addr);
                        break;
                    default:
                        str[0]=0;
                        break;
                }
                if (str[0])
                    Handles->Items->Insert(0,str);
            }
            oldHandleCount=0xfa6f-i;
            oldHandleList[0]=0;
            for (i=0xfa6f,j=0;j<oldHandleCount;j++)
                oldHandleList[0]+=hw->getmem(i--)&0xff;
        }
    }
    else if (calc==86)
    {
        if (!Handles->Items->Count)
            oldHandleList[1]=oldHandleList[0]-1;
        else
        {
            oldHandleList[1]=0;
            for (i=0x2bfff,j=0;j<oldHandleCount;j++)
                oldHandleList[1]+=hw->getmem(i--)&0xff;
        }
        if (oldHandleList[1]!=oldHandleList[0])
        {
            HandlesBox->Caption="Variables";
            Handles->Clear();
            for (i=0x2bfff,j=0;i>(0x20000|hw->getmem_word(0xd298));)
            {
                int type=hw->getmem(i--);
                int addr=hw->getmem(i--);
                addr|=hw->getmem(i--)<<8;
                addr|=hw->getmem(i--)<<16;
                i--;
                k=hw->getmem(i--);
                char name[9];
                for (j=0;j<k;j++)
                    name[j]=hw->getmem(i--);
                name[k]=0;
                if ((name[0]<'0')&&(k==1))
                    continue;
                switch(type)
                {
                    case 0:
                        sprintf(str,"Real %-8s->%5.5X",name,addr);
                        break;
                    case 1:
                        sprintf(str,"Cplx %-8s->%5.5X",name,addr);
                        break;
                    case 2:
                        sprintf(str,"Vect %-8s->%5.5X",name,addr);
                        break;
                    case 3:
                        sprintf(str,"VctC %-8s->%5.5X",name,addr);
                        break;
                    case 4:
                        sprintf(str,"List %-8s->%5.5X",name,addr);
                        break;
                    case 5:
                        sprintf(str,"LstC %-8s->%5.5X",name,addr);
                        break;
                    case 6:
                        sprintf(str,"Mat  %-8s->%5.5X",name,addr);
                        break;
                    case 7:
                        sprintf(str,"MatC %-8s->%5.5X",name,addr);
                        break;
                    case 8:
                        sprintf(str,"Cnst %-8s->%5.5X",name,addr);
                        break;
                    case 9:
                        sprintf(str,"CstC %-8s->%5.5X",name,addr);
                        break;
                    case 0xa:
                        sprintf(str,"Equ  %-8s->%5.5X",name,addr);
                        break;
                    case 0xb:
                        sprintf(str,"Rng  %-8s->%5.5X",name,addr);
                        break;
                    case 0xc:
                        sprintf(str,"Str  %-8s->%5.5X",name,addr);
                        break;
                    case 0xd:
                    case 0xe:
                    case 0xf:
                    case 0x10:
                        sprintf(str,"GDB  %-8s->%5.5X",name,addr);
                        break;
                    case 0x11:
                        sprintf(str,"Pic  %-8s->%5.5X",name,addr);
                        break;
                    case 0x12:
                        sprintf(str,"Prgm %-8s->%5.5X",name,addr);
                        break;
                    case 0x13:
                        sprintf(str,"Rng  %-8s->%5.5X",name,addr);
                        break;
                    default:
                        str[0]=0;
                        break;
                }
                if (str[0])
                    Handles->Items->Insert(0,str);
            }
            oldHandleCount=0x2bfff-i;
            oldHandleList[0]=0;
            for (i=0x2bfff,j=0;j<oldHandleCount;j++)
                oldHandleList[0]+=hw->getmem(i--)&0xff;
        }
    }
    else
        Handles->Clear();
    if (z80)
    {
        sprintf(str,"AF=%4.4X",R.AF.D); D0->Caption=str; D0->Update();
        sprintf(str,"BC=%4.4X",R.BC.D); D1->Caption=str; D1->Update();
        sprintf(str,"DE=%4.4X",R.DE.D); D2->Caption=str; D2->Update();
        sprintf(str,"HL=%4.4X",R.HL.D); D3->Caption=str; D3->Update();
        sprintf(str,"IX=%4.4X",R.IX.D); D4->Caption=str; D4->Update();
        sprintf(str,"IY=%4.4X",R.IY.D); D5->Caption=str; D5->Update();
        sprintf(str,"SP=%4.4X",R.SP.D); D6->Caption=str; D6->Update();
        sprintf(str,""); D7->Caption=str; D7->Update();
        sprintf(str,"AF'=%4.4X",R.AF2.D); A0->Caption=str; A0->Update();
        sprintf(str,"BC'=%4.4X",R.BC2.D); A1->Caption=str; A1->Update();
        sprintf(str,"DE'=%4.4X",R.DE2.D); A2->Caption=str; A2->Update();
        sprintf(str,"HL'=%4.4X",R.HL2.D); A3->Caption=str; A3->Update();
        sprintf(str,"I=%2.2X",R.I&0xff); A4->Caption=str; A4->Update();
        sprintf(str,"R=%2.2X",(R.R&127)|(R.R2&128)); A5->Caption=str; A5->Update();
        sprintf(str,"IM=%X",R.IM); A6->Caption=str; A6->Update();
        sprintf(str,""); A7->Caption=str; A7->Update();
        sprintf(str,""); PC->Caption=str; PC->Update();
//        MakeSR();
        sprintf(str,"PC=%4.4X",R.PC.D); SR->Caption=str; SR->Update();
        sprintf(str,""); TFlag->Caption=str; TFlag->Update();
        sprintf(str,""); SFlag->Caption=str; SFlag->Update();
        sprintf(str,""); MFlag->Caption=str; MFlag->Update();
        sprintf(str,""); IFlag->Caption=str; IFlag->Update();
        sprintf(str,""); XFlag->Caption=str; XFlag->Update();
        sprintf(str,""); NFlag->Caption=str; NFlag->Update();
        sprintf(str,""); ZFlag->Caption=str; ZFlag->Update();
        sprintf(str,""); CFlag->Caption=str; CFlag->Update();
        sprintf(str,""); VFlag->Caption=str; ZFlag->Update();
    }
    else
    {
        sprintf(str,"D0=%8.8X",regs.d[0]); D0->Caption=str; D0->Update();
        sprintf(str,"D1=%8.8X",regs.d[1]); D1->Caption=str; D1->Update();
        sprintf(str,"D2=%8.8X",regs.d[2]); D2->Caption=str; D2->Update();
        sprintf(str,"D3=%8.8X",regs.d[3]); D3->Caption=str; D3->Update();
        sprintf(str,"D4=%8.8X",regs.d[4]); D4->Caption=str; D4->Update();
        sprintf(str,"D5=%8.8X",regs.d[5]); D5->Caption=str; D5->Update();
        sprintf(str,"D6=%8.8X",regs.d[6]); D6->Caption=str; D6->Update();
        sprintf(str,"D7=%8.8X",regs.d[7]); D7->Caption=str; D7->Update();
        sprintf(str,"A0=%8.8X",regs.a[0]); A0->Caption=str; A0->Update();
        sprintf(str,"A1=%8.8X",regs.a[1]); A1->Caption=str; A1->Update();
        sprintf(str,"A2=%8.8X",regs.a[2]); A2->Caption=str; A2->Update();
        sprintf(str,"A3=%8.8X",regs.a[3]); A3->Caption=str; A3->Update();
        sprintf(str,"A4=%8.8X",regs.a[4]); A4->Caption=str; A4->Update();
        sprintf(str,"A5=%8.8X",regs.a[5]); A5->Caption=str; A5->Update();
        sprintf(str,"A6=%8.8X",regs.a[6]); A6->Caption=str; A6->Update();
        sprintf(str,"A7=%8.8X",regs.a[7]); A7->Caption=str; A7->Update();
        sprintf(str,"PC=%6.6X",regs.pc); PC->Caption=str; PC->Update();
        MakeSR();
        sprintf(str,"SR=%4.4X",regs.sr&0xffff); SR->Caption=str; SR->Update();
        sprintf(str,"T=%d",(regs.t1<<1)+regs.t0); TFlag->Caption=str; TFlag->Update();
        sprintf(str,"S=%d",regs.s); SFlag->Caption=str; SFlag->Update();
        sprintf(str,"M=%d",regs.m); MFlag->Caption=str; MFlag->Update();
        sprintf(str,"I=%d",regs.intmask); IFlag->Caption=str; IFlag->Update();
        sprintf(str,"X=%d",regs.x); XFlag->Caption=str; XFlag->Update();
        sprintf(str,"N=%d",NFLG); NFlag->Caption=str; NFlag->Update();
        sprintf(str,"Z=%d",ZFLG); ZFlag->Caption=str; ZFlag->Update();
        sprintf(str,"C=%d",CFLG); CFlag->Caption=str; CFlag->Update();
        sprintf(str,"V=%d",VFLG); VFlag->Caption=str; ZFlag->Update();
    }
    int updateCode=1;
    int pc=z80?R.PC.D:regs.pc;
    if ((Code->Items->Count)&&(!debugEnforceAddr))
    {
        for (i=0,j=-1,k=-1;i<15;i++)
        {
            int addr;
            if (Code->Items->Strings[i].c_str()[0]!='@')
            {
                sscanf(Code->Items->Strings[i].c_str(),"%x",&addr);
                if (addr==pc)
                {
                    j=i;
                    if (!useOldPC) k=i;
                }
                if ((useOldPC)&&(addr==debugStartPC))
                    k=i;
            }
        }
        if ((j!=-1)&&(k==-1)) k=j;
        if (j!=-1)
        {
            if (j>=11)
            {
                if (Code->Items->Strings[j-10].c_str()[0]!='@')
                {
                    sscanf(Code->Items->Strings[j-10].c_str(),
                        "%x",&debugStartPC);
                }
                else
                {
                    sscanf(Code->Items->Strings[j-9].c_str(),
                        "%x",&debugStartPC);
                }
            }
            else
            {
                if (Code->Items->Strings[0].c_str()[0]!='@')
                {
                    sscanf(Code->Items->Strings[0].c_str(),
                        "%x",&debugStartPC);
                }
                else
                {
                    sscanf(Code->Items->Strings[1].c_str(),
                        "%x",&debugStartPC);
                }
                if (!debugForceUpdate)
                    updateCode=0;
                for (i=0;i<15;i++)
                {
                    if (i==j)
                    {
                        strcpy(str,Code->Items->Strings[i].c_str());
                        str[z80?4:6]='-'; str[z80?5:7]='>';
                        Code->Items->Strings[i]=str;
                    }
                    else
                    {
                        strcpy(str,Code->Items->Strings[i].c_str());
                        if (str[0]!='@')
                        {
                            str[z80?4:6]=':'; str[z80?5:7]=' ';
                            Code->Items->Strings[i]=str;
                        }
                    }
                }
                Code->ItemIndex=k;
            }
        }
    }
    debugEnforceAddr=0;
    if (updateCode)
    {
        unsigned short buf[256];
        for (i=0;i<256;i++)
            buf[i]=hw->getmem_word(debugStartPC+(i<<1));
        Code->Clear();
        int first=1;
        for (i=0,j=0,k=-1;i<15;i++)
        {
            int oj=j;
            if (IsROMAddr(debugStartPC+j))
            {
                sprintf(str2,"@%s:",ROMAddrName(
                    debugStartPC+j));
                Code->Items->Add(str2);
                i++; if (i>=15) break;
            }
            if (z80)
                j+=DasmZ80(str,debugStartPC+j);
            else
                j+=Dasm68000((BYTE*)(buf+(j>>1)),str,debugStartPC+j);
            if (first)
            {
                debugFirstInstrucLen=j;
                first=0;
            }
            if ((debugStartPC+oj)!=pc)
                sprintf(str2,z80?"%4.4X: %s":"%6.6X: %s",
                    (debugStartPC+oj)&0xffffff,str);
            else
            {
                sprintf(str2,z80?"%4.4X->%s":"%6.6X->%s",
                    (debugStartPC+oj)&0xffffff,str);
                k=i;
            }
            Code->Items->Add(str2);
        }
        Code->ItemIndex=k;
    }
    Code->Update();
    Stack->Clear();
    for (i=0;i<16;i++)
    {
        sprintf(str,z80?"%4.4X: %4.4X":"%6.6X: %4.4X",
            (z80?R.SP.D:regs.a[7])+(i*2),hw->getmem_word(
            (z80?R.SP.D:regs.a[7])+(i*2)));
        Stack->Items->Add(str);
    }
    Stack->Update();
    UpdateMem();
    RefreshLCDNow();
    lcdUpdateNeeded++;
    SetEvent(lcdUpdateEvent);
    debugForceUpdate=0;
    PauseEnd();
}

void __fastcall TDebugWnd::UpdateMem()
{
    int i,j;
    char str[32];
    TListBox *memCol[8]={MemCol0,MemCol1,MemCol2,MemCol3,
        MemCol4,MemCol5,MemCol6,MemCol7};
    PauseStart();
    MemAddr->Clear(); MemCol0->Clear(); MemCol1->Clear();
    MemCol2->Clear(); MemCol3->Clear(); MemCol4->Clear();
    MemCol5->Clear(); MemCol6->Clear(); MemCol7->Clear();
    MemText->Clear();
    for (i=0;i<0x30;)
    {
        sprintf(str,"%6.6X",(debugStartMem+i)&0xffffff);
        MemAddr->Items->Add(str);
        for (j=0;j<8;j++)
        {
            sprintf(str,"%2.2X",hw->getmem(debugStartMem+i+j));
            memCol[j]->Items->Add(str);
        }
        str[8]=0;
        for (j=0;j<8;j++,i++)
        {
            int c=hw->getmem(debugStartMem+i)&0xff;
            if ((c>=32)&&(c<128))
                str[j]=c;
            else
                str[j]='.';
        }
        MemText->Items->Add(str);
    }
    MemAddr->Repaint(); MemCol0->Repaint(); MemCol1->Repaint();
    MemCol2->Repaint(); MemCol3->Repaint(); MemCol4->Repaint();
    MemCol5->Repaint(); MemCol6->Repaint(); MemCol7->Repaint();
    MemText->Repaint();
    PauseEnd();
}

void __fastcall TDebugWnd::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_EXIT_DEBUG,0,0);
    for (int i=0;i<10;i++) hw->kbmatrix[i]=0xff;
    debugStepOver=0;
    debug=0; run=1;
    CanClose=false;
    Hide();
    debugForceUpdate=1;
    debugException=0;
}
//---------------------------------------------------------------------------

//void DoOneInstruction();

void __fastcall TDebugWnd::Step1Click(TObject *Sender)
{
    debugException=0;
    PauseStart();
    run=1;
    hw->OneInstruction();
    run=0;
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    debugStartPC=z80?R.PC.D:regs.pc;
    Update();
    PauseEnd();
}
//---------------------------------------------------------------------------


void __fastcall TDebugWnd::Run1Click(TObject *Sender)
{
    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_EXIT_DEBUG,0,0);
    debugException=0;
    for (int i=0;i<10;i++) hw->kbmatrix[i]=0xff;
    debugStepOver=0;
    debug=0; run=1;
    Hide();
    debugForceUpdate=1;
}
//---------------------------------------------------------------------------


void __fastcall TDebugWnd::CodeKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    if (Key==VK_PRIOR)
    {
        debugStartPC-=z80?1:2;
        debugEnforceAddr=1;
        Update();
        Application->ProcessMessages();
    }
    else if (Key==VK_NEXT)
    {
        debugStartPC+=debugFirstInstrucLen;
        debugEnforceAddr=1;
        Update();
        Application->ProcessMessages();
    }
    else if (Key==VK_UP)
    {
        if (Code->ItemIndex==0)
        {
            debugStartPC-=z80?1:2;
            debugEnforceAddr=1;
            Update();
            Application->ProcessMessages();
            Code->ItemIndex=0;
        }
    }
    else if (Key==VK_DOWN)
    {
        if (Code->ItemIndex==14)
        {
            debugStartPC+=debugFirstInstrucLen;
            debugEnforceAddr=1;
            Update();
            Application->ProcessMessages();
            Code->ItemIndex=14;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Gotoaddress1Click(TObject *Sender)
{
    GoToDlg->Address->Text="";
    if (GoToDlg->ShowModal()==1)
    {
        sscanf(GoToDlg->Address->Text.c_str(),"%x",&debugStartPC);
        debugEnforceAddr=1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::SetPC1Click(TObject *Sender)
{
    debugException=0;
    if (Code->ItemIndex==-1) return;
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    sscanf(Code->Items->Strings[Code->ItemIndex].c_str(),
        "%x",&(z80?R.PC.D:regs.pc));
    Update();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Break1Click(TObject *Sender)
{
    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_ENTER_DEBUG,0,0);
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    debug=1; run=0;
    while (emuRunning)
        Sleep(0);
    debugStartPC=z80?R.PC.D:regs.pc;
    Update();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol0KeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if (Key==VK_PRIOR)
    {
        int row=((TListBox*)Sender)->ItemIndex;
        debugStartMem-=0x30;
        UpdateMem();
        ((TListBox*)Sender)->ItemIndex=row;
        Key=0;
    }
    else if (Key==VK_NEXT)
    {
        int row=((TListBox*)Sender)->ItemIndex;
        debugStartMem+=0x30;
        UpdateMem();
        ((TListBox*)Sender)->ItemIndex=row;
        Key=0;
    }
    else if (Key==VK_DOWN)
    {
        if (((TListBox *)Sender)->ItemIndex==5)
        {
            debugStartMem+=8;
            UpdateMem();
            ((TListBox *)Sender)->ItemIndex=5;
        }
    }
    else if (Key==VK_UP)
    {
        if (((TListBox *)Sender)->ItemIndex==0)
        {
            debugStartMem-=8;
            UpdateMem();
            ((TListBox *)Sender)->ItemIndex=0;
        }
    }
    else if (Key==VK_LEFT)
    {
        if (Sender==MemCol1)
        {
            MemCol0->ItemIndex=MemCol1->ItemIndex;
            MemCol1->ItemIndex=-1;
            ActiveControl=MemCol0;
        }
        else if (Sender==MemCol2)
        {
            MemCol1->ItemIndex=MemCol2->ItemIndex;
            MemCol2->ItemIndex=-1;
            ActiveControl=MemCol1;
        }
        else if (Sender==MemCol3)
        {
            MemCol2->ItemIndex=MemCol3->ItemIndex;
            MemCol3->ItemIndex=-1;
            ActiveControl=MemCol2;
        }
        else if (Sender==MemCol4)
        {
            MemCol3->ItemIndex=MemCol4->ItemIndex;
            MemCol4->ItemIndex=-1;
            ActiveControl=MemCol3;
        }
        else if (Sender==MemCol5)
        {
            MemCol4->ItemIndex=MemCol5->ItemIndex;
            MemCol5->ItemIndex=-1;
            ActiveControl=MemCol4;
        }
        else if (Sender==MemCol6)
        {
            MemCol5->ItemIndex=MemCol6->ItemIndex;
            MemCol6->ItemIndex=-1;
            ActiveControl=MemCol5;
        }
        else if (Sender==MemCol7)
        {
            MemCol6->ItemIndex=MemCol7->ItemIndex;
            MemCol7->ItemIndex=-1;
            ActiveControl=MemCol6;
        }
        Key=0;
    }
    else if (Key==VK_RIGHT)
    {
        if (Sender==MemCol0)
        {
            MemCol1->ItemIndex=MemCol0->ItemIndex;
            MemCol0->ItemIndex=-1;
            ActiveControl=MemCol1;
        }
        else if (Sender==MemCol1)
        {
            MemCol2->ItemIndex=MemCol1->ItemIndex;
            MemCol1->ItemIndex=-1;
            ActiveControl=MemCol2;
        }
        else if (Sender==MemCol2)
        {
            MemCol3->ItemIndex=MemCol2->ItemIndex;
            MemCol2->ItemIndex=-1;
            ActiveControl=MemCol3;
        }
        else if (Sender==MemCol3)
        {
            MemCol4->ItemIndex=MemCol3->ItemIndex;
            MemCol3->ItemIndex=-1;
            ActiveControl=MemCol4;
        }
        else if (Sender==MemCol4)
        {
            MemCol5->ItemIndex=MemCol4->ItemIndex;
            MemCol4->ItemIndex=-1;
            ActiveControl=MemCol5;
        }
        else if (Sender==MemCol5)
        {
            MemCol6->ItemIndex=MemCol5->ItemIndex;
            MemCol5->ItemIndex=-1;
            ActiveControl=MemCol6;
        }
        else if (Sender==MemCol6)
        {
            MemCol7->ItemIndex=MemCol6->ItemIndex;
            MemCol6->ItemIndex=-1;
            ActiveControl=MemCol7;
        }
        Key=0;
    }
    else if (Key==VK_RETURN)
    {
        if (Sender==MemCol0)
            MemDblClick(0,MemCol0->ItemIndex);
        else if (Sender==MemCol1)
            MemDblClick(1,MemCol1->ItemIndex);
        else if (Sender==MemCol2)
            MemDblClick(2,MemCol2->ItemIndex);
        else if (Sender==MemCol3)
            MemDblClick(3,MemCol3->ItemIndex);
        else if (Sender==MemCol4)
            MemDblClick(4,MemCol4->ItemIndex);
        else if (Sender==MemCol5)
            MemDblClick(5,MemCol5->ItemIndex);
        else if (Sender==MemCol6)
            MemDblClick(6,MemCol6->ItemIndex);
        else if (Sender==MemCol7)
            MemDblClick(7,MemCol7->ItemIndex);
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemGotoClick(TObject *Sender)
{
    GoToDlg->Address->Text="";
    if (GoToDlg->ShowModal()==1)
    {
        sscanf(GoToDlg->Address->Text.c_str(),"%x",&debugStartMem);
        UpdateMem();
    }
}

void __fastcall TDebugWnd::MemCol0Click(TObject *Sender)
{
    MemCol1->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol1Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol2Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol1->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol3Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol1->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol4Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol1->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol5Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol1->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol6Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol1->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol7->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol7Click(TObject *Sender)
{
    MemCol0->ItemIndex=-1;
    MemCol1->ItemIndex=-1;
    MemCol2->ItemIndex=-1;
    MemCol3->ItemIndex=-1;
    MemCol4->ItemIndex=-1;
    MemCol5->ItemIndex=-1;
    MemCol6->ItemIndex=-1;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemDblClick(int col,int selindx)
{
    if (selindx==-1) return;
    int addr=debugStartMem+col+(selindx<<3);
    char str[4];
    sprintf(str,"%2.2X",hw->getmem(addr)&0xff);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        hw->setmem(addr&0xffffff,v);
        int row=selindx;
        UpdateMem();
        switch(col)
        {
            case 0: MemCol0->ItemIndex=row; break;
            case 1: MemCol1->ItemIndex=row; break;
            case 2: MemCol2->ItemIndex=row; break;
            case 3: MemCol3->ItemIndex=row; break;
            case 4: MemCol4->ItemIndex=row; break;
            case 5: MemCol5->ItemIndex=row; break;
            case 6: MemCol6->ItemIndex=row; break;
            case 7: MemCol7->ItemIndex=row; break;
        }
    }
}

void __fastcall TDebugWnd::MemCol0DblClick(TObject *Sender)
{
    MemDblClick(0,MemCol0->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol1DblClick(TObject *Sender)
{
    MemDblClick(1,MemCol1->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol2DblClick(TObject *Sender)
{
    MemDblClick(2,MemCol2->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol3DblClick(TObject *Sender)
{
    MemDblClick(3,MemCol3->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol4DblClick(TObject *Sender)
{
    MemDblClick(4,MemCol4->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol5DblClick(TObject *Sender)
{
    MemDblClick(5,MemCol5->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol6DblClick(TObject *Sender)
{
    MemDblClick(6,MemCol6->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MemCol7DblClick(TObject *Sender)
{
    MemDblClick(7,MemCol7->ItemIndex);
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Setbreakpoint1Click(TObject *Sender)
{
    if (Code->ItemIndex==-1) return;
    int pc,i,j;
    PauseStart();
    sscanf(Code->Items->Strings[Code->ItemIndex].c_str(),
        "%x",&pc);
    for (i=0;i<debugCodeBreakCount;i++)
    {
        if (debugCodeBreak[i]==pc)
        {
            for (j=i;j<(debugCodeBreakCount-1);j++)
                debugCodeBreak[j]=debugCodeBreak[j+1];
            debugCodeBreakCount--;
            Code->Repaint();
            PauseEnd();
            return;
        }
    }
    debugCodeBreak[debugCodeBreakCount++]=pc;
    Code->Repaint();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::CodeDrawItem(TWinControl *Control, int Index,
      TRect &Rect, TOwnerDrawState State)
{
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    TBrush *brush=new TBrush;
    TBrush *oldBrush=Code->Canvas->Brush;
    brush->Style=bsSolid;
    TPen *pen=new TPen;
    TPen *oldPen=Code->Canvas->Pen;
    int pos,bp=0;
    int txt=0;
    if (Code->Items->Strings[Index].c_str()[0]!='@')
    {
        sscanf(Code->Items->Strings[Index].c_str(),
            "%X",&pos);
        for (int i=0;i<debugCodeBreakCount;i++)
        {
            if (pos==debugCodeBreak[i])
                bp=1;
        }
    }
    else
        txt=1;
    if (State.Contains(odSelected))
    {
        if ((!txt)&&(Code->Items->Strings[Index].c_str()[z80?4:6]=='-'))
        {
            if (bp)
            {
                brush->Color=clDkGray;
                pen->Color=clWhite;
            }
            else
            {
                brush->Color=clTeal;
                pen->Color=clWhite;
            }
        }
        else
        {
            if (bp)
            {
                brush->Color=clPurple;
                pen->Color=clWhite;
            }
            else
            {
                brush->Color=clNavy;
                pen->Color=clWhite;
            }
        }
    }
    else
    {
        if ((!txt)&&(Code->Items->Strings[Index].c_str()[z80?4:6]=='-'))
        {
            if (bp)
            {
                brush->Color=clOlive;
                pen->Color=clWhite;
            }
            else
            {
                brush->Color=clGreen;
                pen->Color=clWhite;
            }
        }
        else
        {
            if (bp)
            {
                brush->Color=clMaroon;
                pen->Color=clWhite;
            }
            else
            {
                brush->Color=clWhite;
                pen->Color=clBlack;
            }
        }
    }
    Code->Canvas->Brush=brush;
    Code->Canvas->Pen=pen;
    Code->Canvas->FillRect(Rect);
    Code->Canvas->Font->Color=pen->Color;
    HFONT fnt;
    TFont *oldFont;
    if (txt)
    {
        fnt=CreateFont(-MulDiv(8,GetDeviceCaps(
            Code->Canvas->Handle, LOGPIXELSY),72),0,0,0,
            FW_BOLD,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
            FF_DONTCARE,"Courier New");
        oldFont=Code->Canvas->Font;
        Code->Canvas->Font->Handle=fnt;
        Code->Canvas->TextOut(Rect.Left,Rect.Top,&(Code->Items->
            Strings[Index].c_str()[1]));
        Code->Canvas->Font=oldFont;
        DeleteObject(fnt);
    }
    else
    {
        Code->Canvas->TextOut(Rect.Left,Rect.Top,Code->Items->
            Strings[Index]);
    }
    Code->Canvas->Brush=oldBrush;
    Code->Canvas->Pen=oldPen;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::GotoPC1Click(TObject *Sender)
{
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    debugStartPC=z80?R.PC.D:regs.pc;
    Update();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Clearallbreakpoints1Click(TObject *Sender)
{
    PauseStart();
    debugCodeBreakCount=0;
    debugDataBreakCount=0;
    debugProgBreak=0;
    exceptBPCount=0;
    for (int i=0;i<48;i++)
        exceptBP[i]=0;
    Code->Repaint();
    if ((debugLogEnable&&debugLogCount)||debugDataBreakCount)
        UseDebugReadFuncs();
    else
        RestoreReadFuncs();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Setbreakpointonprogramentry1Click(
      TObject *Sender)
{
    PauseStart();
    ProgramEntryDlg->UpdateTree();
    if (ProgramEntryDlg->ShowModal()==1)
    {
        int z80=romImage[currentROM].type&ROMFLAG_Z80;
        if (!z80)
        {
            if ((calc==92)||(calc==93))
            {
                debugProgBreak=((((int)ProgramEntryDlg->Programs->
                    Selected->Data)&0xffff)<<2)+HANDLE_TABLE_92;
            }
            else
            {
                debugProgBreak=((((int)ProgramEntryDlg->Programs->
                    Selected->Data)&0xffff)<<2)+HANDLE_TABLE;
            }
            debugProgBreakOfs=(((int)ProgramEntryDlg->Programs->
                Selected->Data)>>16)&0xffff;
        }
        else
        {
            debugProgBreak=(int)ProgramEntryDlg->Programs->Selected->Data;
            if (calc==85)
            {
                int addr=hw->getmem(debugProgBreak&0xffff);
                addr|=hw->getmem((debugProgBreak&0xffff)-1)<<8;
                debugProgLen=hw->getmem_word(addr);
            }
        }
    }
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::HandlesDblClick(TObject *Sender)
{
    if (Handles->ItemIndex==-1) return;
    int addr;
    if (calc>=89)
    {
        sscanf(&((Handles->Items->Strings[Handles->ItemIndex].
            c_str())[10]),"%X",&addr);
    }
    else
    {
        sscanf(&((Handles->Items->Strings[Handles->ItemIndex].
            c_str())[15]),"%X",&addr);
    }
    debugStartMem=addr;
    Update();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Viewmemory1Click(TObject *Sender)
{
    if (Handles->ItemIndex==-1) return;
    int addr;
    if (calc>=89)
    {
        sscanf(&((Handles->Items->Strings[Handles->ItemIndex].
            c_str())[10]),"%X",&addr);
    }
    else
    {
        sscanf(&((Handles->Items->Strings[Handles->ItemIndex].
            c_str())[15]),"%X",&addr);
    }
    debugStartMem=addr;
    Update();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Viewdisassembly1Click(TObject *Sender)
{
    if (Handles->ItemIndex==-1) return;
    int addr;
    if (calc>=89)
    {
        sscanf(&((Handles->Items->Strings[Handles->ItemIndex].
            c_str())[10]),"%X",&addr);
    }
    else
    {
        sscanf(&((Handles->Items->Strings[Handles->ItemIndex].
            c_str())[15]),"%X",&addr);
    }
    debugStartPC=addr+((calc==84)?4:2);
    debugEnforceAddr=1;
    Update();
}
//---------------------------------------------------------------------------
void __fastcall TDebugWnd::Setdatabreakpoint1Click(TObject *Sender)
{
    PauseStart();
    DataBreakDlg->UpdateList();
    DataBreakDlg->ShowModal();
    if ((debugLogEnable&&debugLogCount)||debugDataBreakCount)
        UseDebugReadFuncs();
    else
        RestoreReadFuncs();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Searchforvalue1Click(TObject *Sender)
{
    SearchDlg->ActiveControl=SearchDlg->Value;
    if (SearchDlg->ShowModal()==1)
    {
        int v,addr;
        if (SearchDlg->Size->ItemIndex<3)
        {
            sscanf(SearchDlg->Value->Text.c_str(),"%x",&v);
            if (SearchDlg->Size->ItemIndex==0) v&=0xff;
            else if (SearchDlg->Size->ItemIndex==1) v&=0xffff;
            addr=(debugStartMem+2)&0xffffff;
            int size=SearchDlg->Size->ItemIndex;
            if (size!=0) addr&=~1;
            for (;addr<0x600000;addr+=(size==0)?1:2)
            {
                if ((addr>=0x40000)&&(addr<0x200000))
                {
                    addr=0x200000-((size==0)?1:2);
                    continue;
                }
                if (size==0)
                {
                    if ((hw->getmem(addr)&0xff)==v)
                    {
                        debugStartMem=addr;
                        UpdateMem();
                        break;
                    }
                }
                else if (size==1)
                {
                    if ((hw->getmem_word(addr)&0xffff)==v)
                    {
                        debugStartMem=addr;
                        UpdateMem();
                        break;
                    }
                }
                else
                {
                    if (hw->getmem_dword(addr)==v)
                    {
                        debugStartMem=addr;
                        UpdateMem();
                        break;
                    }
                }
            }
        }
        else
        {
            char str[256];
            int indx;

            addr=(debugStartMem+2)&0xffffff;
            strcpy(str,SearchDlg->Value->Text.c_str());
            for (indx=0;addr<0x600000;addr++)
            {
                if ((addr>=0x40000)&&(addr<0x200000))
                {
                    addr=0x200000-1;
                    indx=0;
                    continue;
                }
                if (!str[indx])
                {
                    addr-=strlen(str);
                    debugStartMem=addr;
                    UpdateMem();
                    break;
                }
                if ((hw->getmem(addr)&0xff)!=(str[indx]&0xff))
                    indx=0;
                else
                    indx++;
            }
        }
        if (addr>=0x600000)
            MessageBox(Handle,"No matches found","Search error",MB_OK);
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Stepover1Click(TObject *Sender)
{
    debugException=0;
    if (debugStepOver) return;
    unsigned short buf[16];
    char str[64];
    int addr,i;
    PauseStart();
    for (i=0;i<16;i++)
        buf[i]=hw->getmem_word(regs.pc+(i<<1));
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    if (z80)
        addr=R.PC.D+DasmZ80(str,R.PC.D);
    else
        addr=regs.pc+Dasm68000((BYTE*)buf,str,regs.pc);
    debugStepOver=1; debugStepGoal=addr;
    debugBeginStepOver=1; debugEndStepOver=0;
    debugStepA7=z80?R.SP.D:regs.a[7];
    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_EXIT_DEBUG,0,0);
    run=1;
    hw->OneInstruction();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D0Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.AF.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.AF.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[0]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[0]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D1Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.BC.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.BC.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[1]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[1]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D2Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.DE.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.DE.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[2]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[2]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D3Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.HL.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.HL.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[3]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[3]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D4Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.IX.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.IX.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[4]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[4]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D5Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.IY.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.IY.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[5]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[5]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D6Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.SP.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.SP.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.d[6]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[6]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::D7Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%8.8X",regs.d[7]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.d[7]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A0Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.AF2.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.AF2.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[0]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[0]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A1Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.BC2.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.BC2.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[1]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[1]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A2Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.DE2.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.DE2.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[2]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[2]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A3Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.HL2.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.HL2.D);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[3]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[3]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A4Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%2.2X",R.I&0xff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.I);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[4]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[4]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A5Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%2.2X",R.R&0x7f);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.R);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[5]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[5]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A6Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%X",R.IM);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.IM);
            Update();
        }
        return;
    }
    sprintf(str,"%8.8X",regs.a[6]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[6]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::A7Click(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%8.8X",regs.a[7]);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.a[7]);
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::PCClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%6.6X",regs.pc);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",
            &regs.pc);
        debugStartPC=regs.pc;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::SRClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80)
    {
        sprintf(str,"%4.4X",R.PC.D&0xffff);
        NewValueDlg->Address->Text=str;
        NewValueDlg->Address->SelectAll();
        NewValueDlg->ActiveControl=NewValueDlg->Address;
        if (NewValueDlg->ShowModal()==1)
        {
            sscanf(NewValueDlg->Address->Text.c_str(),"%x",
                &R.PC.D);
            Update();
        }
        return;
    }
    MakeSR();
    sprintf(str,"%4.4X",regs.sr);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        regs.sr=v&0xffff;
        MakeFromSR();
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::TFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",(regs.t1<<1)+regs.t0);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        regs.t1=v&2; regs.t0=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::SFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",regs.s);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        regs.s=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::MFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",regs.m);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        regs.m=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::IFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",regs.intmask);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        regs.intmask=v&7;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::XFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",regs.x);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        regs.x=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::NFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",NFLG);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        NFLG=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::ZFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",ZFLG);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        ZFLG=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::VFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",VFLG);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        VFLG=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::CFlagClick(TObject *Sender)
{
    char str[4];
    if (romImage[currentROM].type&ROMFLAG_Z80) return;
    sprintf(str,"%1.1X",CFLG);
    NewValueDlg->Address->Text=str;
    NewValueDlg->Address->SelectAll();
    NewValueDlg->ActiveControl=NewValueDlg->Address;
    if (NewValueDlg->ShowModal()==1)
    {
        int v;
        sscanf(NewValueDlg->Address->Text.c_str(),"%x",&v);
        CFLG=v&1;
        Update();
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Viewlog1Click(TObject *Sender)
{
    PauseStart();
    LogDlg->ShowModal();
    Enablelogging1->Checked=debugLogEnable;
    if ((debugLogEnable&&debugLogCount)||debugDataBreakCount)
        UseDebugReadFuncs();
    else
        RestoreReadFuncs();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Editloggedaddresses1Click(TObject *Sender)
{
    PauseStart();
    EditLogDlg->UpdateList();
    EditLogDlg->ShowModal();
    Enablelogging1->Checked=debugLogEnable;
    if ((debugLogEnable&&debugLogCount)||debugDataBreakCount)
        UseDebugReadFuncs();
    else
        RestoreReadFuncs();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Enablelogging1Click(TObject *Sender)
{
    PauseStart();
    debugLogEnable=!debugLogEnable;
    Enablelogging1->Checked=debugLogEnable;
    if ((debugLogEnable&&debugLogCount)||debugDataBreakCount)
        UseDebugReadFuncs();
    else
        RestoreReadFuncs();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Clearall1Click(TObject *Sender)
{
    PauseStart();
    debugLogEnable=0;
    Enablelogging1->Checked=false;
    debugLogCount=0;
    if ((debugLogEnable&&debugLogCount)||debugDataBreakCount)
        UseDebugReadFuncs();
    else
        RestoreReadFuncs();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Loglinkportsend1Click(TObject *Sender)
{
    Loglinkportsend1->Checked=!Loglinkportsend1->Checked;
    debugLogLinkSend=Loglinkportsend1->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Loglinkportreceive1Click(TObject *Sender)
{
    Loglinkportreceive1->Checked=!Loglinkportreceive1->Checked;
    debugLogLinkRecv=Loglinkportreceive1->Checked;
}
//---------------------------------------------------------------------------

int CheckAddr(int addr,int size,int type,int v)
{
    int i;
    for (i=0;i<debugDataBreakCount;i++)
    {
        if (!(debugDataBreakType[i]&type)) continue;
        if ((addr>(debugDataBreakLow[i]-size))&&
            (addr<=debugDataBreakHigh[i]))
            return 1;
    }
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    int pc=z80?R.PC.D:regs.pc;
    for (i=0;i<debugLogCount;i++)
    {
        if (debugLogClass[i]==LOG_VALUE)
        {
            if (!(debugLogType[i]&type)) continue;
            if ((addr>(debugLogLow[i]-size))&&
                (addr<=debugLogHigh[i]))
            {
                char str[64];
                if (type==DATABREAK_READ)
                {
                    if ((z80)&&((debugLogLow[i]>>20)==6))
                    {
                        if (size==1)
                            sprintf(str,"Read byte %2.2X from I/O port %X at PC=%X",hw->getmem(addr)&0xff,addr&0xff,pc&0xffffff);
                        else if (size==2)
                            sprintf(str,"Read word %4.4X from I/O port %X at PC=%X",hw->getmem_word(addr)&0xffff,addr&0xff,pc&0xffffff);
                        else
                            sprintf(str,"Read long %8.8X from I/O port %X at PC=%X",hw->getmem_dword(addr),addr&0xff,pc&0xffffff);
                    }
                    else
                    {
                        if (size==1)
                            sprintf(str,"Read byte %2.2X from %X at PC=%X",hw->getmem(addr)&0xff,addr,pc&0xffffff);
                        else if (size==2)
                            sprintf(str,"Read word %4.4X from %X at PC=%X",hw->getmem_word(addr)&0xffff,addr,pc&0xffffff);
                        else
                            sprintf(str,"Read long %8.8X from %X at PC=%X",hw->getmem_dword(addr),addr,pc&0xffffff);
                    }
                }
                else
                {
                    if ((z80)&&((debugLogLow[i]>>20)==6))
                    {
                        if (size==1)
                            sprintf(str,"Write byte %2.2X to I/O port %X at PC=%X",v&0xff,addr&0xff,pc&0xffffff);
                        else if (size==2)
                            sprintf(str,"Write word %4.4X to I/O port %X at PC=%X",v&0xffff,addr&0xff,pc&0xffffff);
                        else
                            sprintf(str,"Write long %8.8X to I/O port %X at PC=%X",v,addr&0xff,pc&0xffffff);
                    }
                    else
                    {
                        if (size==1)
                            sprintf(str,"Write byte %2.2X to %X at PC=%X",v&0xff,addr,pc&0xffffff);
                        else if (size==2)
                            sprintf(str,"Write word %4.4X to %X at PC=%X",v&0xffff,addr,pc&0xffffff);
                        else
                            sprintf(str,"Write long %8.8X to %X at PC=%X",v,addr,pc&0xffffff);
                    }
                }
                LogDlg->List->Items->Add(str);
            }
        }
        else
        {
            if (type==DATABREAK_READ) continue;
            if (!((addr>(debugLogLow[i]-size))&&
                  (addr<=debugLogLow[i])))
                continue;
            int j,b,nv;
            char str[64];
            nv=v&0xff;
            if (size==2)
            {
                if ((debugLogLow[i]-addr)==0)
                    nv=(v>>8)&0xff;
                else
                    nv=v&0xff;
            }
            else if (size==4)
            {
                if ((debugLogLow[i]-addr)==0)
                    nv=(v>>24)&0xff;
                else if ((debugLogLow[i]-addr)==1)
                    nv=(v>>16)&0xff;
                else if ((debugLogLow[i]-addr)==2)
                    nv=(v>>8)&0xff;
                else if ((debugLogLow[i]-addr)==3)
                    nv=v&0xff;
            }
            for (j=0,b=1;j<8;j++,b<<=1)
            {
                if (!debugLogCheck[i]&b) continue;
                if (debugLogAny[i]&b)
                {
                    if ((debugLogLast[i]&b)!=(nv&b))
                    {
                        if ((z80)&&((debugLogLow[i]>>20)==6))
                        {
                            if (nv&b)
                                sprintf(str,"Set bit %d of I/O port %X at PC=%X",j,debugLogLow[i]&0xff,pc&0xffffff);
                            else
                                sprintf(str,"Clear bit %d of I/O port %X at PC=%X",j,debugLogLow[i]&0xff,pc&0xffffff);
                        }
                        else
                        {
                            if (nv&b)
                                sprintf(str,"Set bit %d of %X at PC=%X",j,debugLogLow[i],pc&0xffffff);
                            else
                                sprintf(str,"Clear bit %d of %X at PC=%X",j,debugLogLow[i],pc&0xffffff);
                        }
                        LogDlg->List->Items->Add(str);
                    }
                }
                else
                {
                    if ((debugLogLast[i]&b)==(nv&b)) continue;
                    if ((debugLogState[i]&b)==(nv&b))
                    {
                        if ((z80)&&((debugLogLow[i]>>20)==6))
                        {
                            if (nv&b)
                                sprintf(str,"Set bit %d of I/O port %X at PC=%X",j,debugLogLow[i]&0xff,pc&0xffffff);
                            else
                                sprintf(str,"Clear bit %d of I/O port %X at PC=%X",j,debugLogLow[i]&0xff,pc&0xffffff);
                        }
                        else
                        {
                            if (nv&b)
                                sprintf(str,"Set bit %d of %X at PC=%X",j,debugLogLow[i],pc&0xffffff);
                            else
                                sprintf(str,"Clear bit %d of %X at PC=%X",j,debugLogLow[i],pc&0xffffff);
                        }
                        LogDlg->List->Items->Add(str);
                    }
                }
            }
            debugLogLast[i]=v;
        }
    }
    return 0;
}

void DataBreak()
{
    debugDataBreak=1;
}

void UpdateDebugCheckEnable()
{
    if (debugCodeBreakCount||debugDataBreakCount||
        debugProgBreak||debugStepOver||debugLogEnable||
        exceptBPCount||debugRunToCursor)
        debugCheckEnable=1;
    else
        debugCheckEnable=0;
}

void __fastcall TDebugWnd::FormCreate(TObject *Sender)
{
    Code->ItemHeight=MulDiv(10,GetDeviceCaps(Code->Canvas->
        Handle, LOGPIXELSY),96)+4;
    exceptBPCount=0;
    for (int i=0;i<48;i++)
        exceptBP[i]=0;
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Setbreakpointontrapexception1Click(
      TObject *Sender)
{
    TrapDlg->AddressError->Checked=exceptBP[3];
    TrapDlg->IllegalInstruction->Checked=exceptBP[4];
    TrapDlg->DivZero->Checked=exceptBP[5];
    TrapDlg->CHK->Checked=exceptBP[6];
    TrapDlg->TRAPV->Checked=exceptBP[7];
    TrapDlg->PrivilegeViolation->Checked=exceptBP[8];
    TrapDlg->Trace->Checked=exceptBP[9];
    TrapDlg->Line1010->Checked=exceptBP[10];
    TrapDlg->Line1111->Checked=exceptBP[11];
    TrapDlg->AutoInt1->Checked=exceptBP[25];
    TrapDlg->AutoInt2->Checked=exceptBP[26];
    TrapDlg->AutoInt4->Checked=exceptBP[28];
    TrapDlg->AutoInt5->Checked=exceptBP[29];
    TrapDlg->AutoInt6->Checked=exceptBP[30];
    TrapDlg->AutoInt7->Checked=exceptBP[31];
    TrapDlg->Trap0->Checked=exceptBP[32];
    TrapDlg->Trap1->Checked=exceptBP[33];
    TrapDlg->Trap2->Checked=exceptBP[34];
    TrapDlg->Trap3->Checked=exceptBP[35];
    TrapDlg->Trap4->Checked=exceptBP[36];
    TrapDlg->Trap5->Checked=exceptBP[37];
    TrapDlg->Trap6->Checked=exceptBP[38];
    TrapDlg->Trap7->Checked=exceptBP[39];
    TrapDlg->Trap8->Checked=exceptBP[40];
    TrapDlg->Trap9->Checked=exceptBP[41];
    TrapDlg->Trap10->Checked=exceptBP[42];
    TrapDlg->Trap11->Checked=exceptBP[43];
    TrapDlg->Trap12->Checked=exceptBP[44];
    TrapDlg->Trap13->Checked=exceptBP[45];
    TrapDlg->Trap14->Checked=exceptBP[46];
    TrapDlg->Trap15->Checked=exceptBP[47];
    if (TrapDlg->ShowModal()!=1)
        return;
    exceptBP[3]=TrapDlg->AddressError->Checked;
    exceptBP[4]=TrapDlg->IllegalInstruction->Checked;
    exceptBP[5]=TrapDlg->DivZero->Checked;
    exceptBP[6]=TrapDlg->CHK->Checked;
    exceptBP[7]=TrapDlg->TRAPV->Checked;
    exceptBP[8]=TrapDlg->PrivilegeViolation->Checked;
    exceptBP[9]=TrapDlg->Trace->Checked;
    exceptBP[10]=TrapDlg->Line1010->Checked;
    exceptBP[11]=TrapDlg->Line1111->Checked;
    exceptBP[25]=TrapDlg->AutoInt1->Checked;
    exceptBP[26]=TrapDlg->AutoInt2->Checked;
    exceptBP[28]=TrapDlg->AutoInt4->Checked;
    exceptBP[29]=TrapDlg->AutoInt5->Checked;
    exceptBP[30]=TrapDlg->AutoInt6->Checked;
    exceptBP[31]=TrapDlg->AutoInt7->Checked;
    exceptBP[32]=TrapDlg->Trap0->Checked;
    exceptBP[33]=TrapDlg->Trap1->Checked;
    exceptBP[34]=TrapDlg->Trap2->Checked;
    exceptBP[35]=TrapDlg->Trap3->Checked;
    exceptBP[36]=TrapDlg->Trap4->Checked;
    exceptBP[37]=TrapDlg->Trap5->Checked;
    exceptBP[38]=TrapDlg->Trap6->Checked;
    exceptBP[39]=TrapDlg->Trap7->Checked;
    exceptBP[40]=TrapDlg->Trap7->Checked;
    exceptBP[41]=TrapDlg->Trap9->Checked;
    exceptBP[42]=TrapDlg->Trap10->Checked;
    exceptBP[43]=TrapDlg->Trap11->Checked;
    exceptBP[44]=TrapDlg->Trap12->Checked;
    exceptBP[45]=TrapDlg->Trap13->Checked;
    exceptBP[46]=TrapDlg->Trap14->Checked;
    exceptBP[47]=TrapDlg->Trap15->Checked;
    exceptBPCount=0;
    for (int i=0;i<48;i++)
    {
        if (exceptBP[i])
            exceptBPCount++;
    }
}
//---------------------------------------------------------------------------

void __fastcall TDebugWnd::Runtocursor1Click(TObject *Sender)
{
    if (Code->ItemIndex==-1) return;
    int pc,i,j;
    PauseStart();
    debugRunToCursor=1;
    sscanf(Code->Items->Strings[Code->ItemIndex].c_str(),
        "%x",&pc);
    debugCursorAddr=pc;
    debugStepOver=0;
    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_EXIT_DEBUG,0,0);
    run=1;
    Hide();
    hw->OneInstruction();
    PauseEnd();
}
//---------------------------------------------------------------------------

