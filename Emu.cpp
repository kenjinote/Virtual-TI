//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\registry.hpp>
#pragma hdrstop
#define DIRECTINPUT_VERSION 0x300
//#include <dinput.h>
#include <stdio.h>
#include <dir.h>

#include "Emu.h"
#include "Debug.h"
#define NOTYPEREDEF
#include "z80.h"
#include "cpudefs.h"
#include "About.h"
#include "EmuMode.h"
#include "Entry.h"
#include "Log.h"
#include "Redef.h"
#include "rom.h"
#include "EmuThread.h"
#include "RomWizard.h"
#include "Sound.h"
#include <clipbrd.hpp>

#define ROMTBL(x) hw->getmem_dword(hw->getmem_dword(0xc8)+(x)*4)
#define HANDLE_TABLE hw->getmem_dword(ROMTBL(0x2f)+0x11a)
#define HANDLE_TABLE_92 cpu_readmem24_dword(romFuncAddr[0x1c]+0x1902)
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TEmuWnd *EmuWnd;
EmuThread *emuThread;

extern char vLinkFileName[32];

extern HANDLE emuEvent,hMutex;
HANDLE lcdUpdateEvent;

int percentChange=0;
int percent=0;
int showSpeed=0;

volatile bool pause=false;
volatile bool pauseReady=false;
volatile bool pauseExit=true;
volatile extern int exitEmu;
volatile extern int emuRunning;
volatile bool showDebugWnd=false;

extern Z80_Regs R;

char* alphakeystr[]={"Q","W","E","R","T","Y","U","I","O",
    "P","A","S","D","F","G","H","J","K","L","Z","X","C",
    "V","B","N","M"};

int ti92kmat[10][8]=
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

//extern void KeyDown(int,int,int,int);
//extern void KeyUp(int,int,int,int);
//extern void OnDown();
//extern void OnUp();
extern void IRQ(int n);
//extern void SaveState(char *name);
//extern void LoadState(char *name);

extern int calc;

#define SIZE_NONE 0
#define SIZE_LEFT 1
#define SIZE_RIGHT 2
#define SIZE_TOP 3
#define SIZE_BOTTOM 4

static int mouseClicked=0;
static int mouseX,mouseY;
static int skinFloating=0;
static int sizeType=SIZE_NONE;

int pauseLevel=0;

void PauseStart()
{
    pauseLevel++;
    if (pauseLevel>1)
        return;
    int startTime=GetTickCount();
    while (!pauseExit)
    {
        Sleep(0);
        if ((GetTickCount()-startTime)>3000)
            break;
    }
    pauseExit=false;
    pause=true;
    SetEvent(emuEvent);
    while (!pauseReady)
    {
        Sleep(0);
        if ((GetTickCount()-startTime)>3000)
            break;
    }
}

void PauseEnd()
{
    pauseLevel--;
    if (pauseLevel)
        return;
    pause=false;
    pauseReady=false;
    SetEvent(emuEvent);
    int startTime=GetTickCount();
    while (!pauseExit)
    {
        Sleep(0);
        if ((GetTickCount()-startTime)>3000)
            break;
    }
    pauseExit=true;
}

void kup(int k)
{
    PauseStart();
    if (k==k_on)
    {
        hw->OnUp();
        PauseEnd();
        return;
    }
    int r,c,i,j,alpha;
    r=-1; c=-1; alpha=0;
    if ((k==k_enter2)&&(calc<92))
        k=k_enter1;
    for (int j=0;j<10;j++)
    {
        for (int i=0;i<8;i++)
        {
            if (hw->GetKeyMat(j,i)==k)
            {
                r=j;
                c=i;
                alpha=0;
            }
            if (hw->GetKeyAlphaMat(j,i)==k)
            {
                r=j;
                c=i;
                alpha=1;
            }
        }
    }
    hw->KeyUp(r,7-c);
    if (calc>=89) hw->KBInt();
    PauseEnd();
}

void kdown(int k)
{
    PauseStart();
    if (k==k_on)
    {
        hw->OnDown();
        PauseEnd();
        return;
    }
    int r,c,i,j,alpha;
    r=-1; c=-1; alpha=0;
    if ((k==k_enter2)&&(calc<92))
        k=k_enter1;
    for (int j=0;j<10;j++)
    {
        for (int i=0;i<8;i++)
        {
            if (hw->GetKeyMat(j,i)==k)
            {
                r=j;
                c=i;
                alpha=0;
            }
            if (hw->GetKeyAlphaMat(j,i)==k)
            {
                r=j;
                c=i;
                alpha=1;
            }
        }
    }
    if (alpha)
    {
        for (int i=0;i<10;i++) hw->kbmatrix[i]=0xff;
        if ((calc==85)||(calc==86))
        {
            for (int j=0;j<4;j++)
            {
                if (hw->getmem(R.IY.D+0x12)&0x50)
                {
                    hw->AlphaDown();
                    hw->KBInt();
                    for (i=0;i<256;i++)
                        hw->Execute();
                    hw->AlphaUp();
                    hw->KBInt();
                    for (i=0;i<2048;i++)
                        hw->Execute();
                }
                if (hw->getmem(R.IY.D+0x12)&8)
                {
                    hw->KeyDown(6,5);
                    hw->KBInt();
                    for (i=0;i<256;i++)
                        hw->Execute();
                    hw->KeyUp(6,5);
                    hw->KBInt();
                    for (i=0;i<2048;i++)
                        hw->Execute();
                }
            }
            if ((GetKeyState(VK_SHIFT)&0x8000)!=0x8000)
            {
                hw->KeyDown(6,5);
                hw->KBInt();
                for (i=0;i<256;i++)
                    hw->Execute();
                hw->KeyUp(6,5);
                hw->KBInt();
                for (i=0;i<256;i++)
                    hw->Execute();
            }
        }
        if (calc==89)
        {
            if ((GetKeyState(VK_SHIFT)&0x8000)!=0x8000)
            {
                hw->AlphaDown();
                hw->KBInt();
                for (i=0;i<256;i++)
                    hw->Execute();
            }
            else
            {
                hw->KeyDown(0,5);
                hw->KBInt();
                for (i=0;i<256;i++)
                    hw->Execute();
            }
        }
        else
        {
            hw->AlphaDown();
            hw->KBInt();
            for (i=0;i<256;i++)
                hw->Execute();
        }
    }
    hw->KeyDown(r,7-c);
    if (calc>=89) hw->KBInt();
    if (alpha)
    {
        hw->KBInt();
        for (i=0;i<256;i++)
            hw->Execute();
        hw->AlphaUp();
        if (calc==89)
            hw->KeyUp(0,5);
        hw->KBInt();
        for (i=0;i<64;i++)
            hw->Execute();
        hw->KeyUp(r,7-c);
        hw->KBInt();
        for (i=0;i<64;i++)
            hw->Execute();
    }
    PauseEnd();
}

//---------------------------------------------------------------------------
int emuLink,openLink,realKeypad,graphLink,cl_timeout;

int port_type;
#define PORT_TYPE_SERIAL   1
#define PORT_TYPE_PARALLEL 0
int parallel_address;   //usually 0x378, 0x278, or 0x3bc

HANDLE hCom=NULL;
extern int debugLogLinkSend,debugLogLinkRecv;

int useInitPath=1;
extern char initPath[256];

//void SendCom(int c);
//int GetCom(int *c);
//int CheckCom();

int clErrorFlag=0;
int clVirtual=0;
int clOpen=0;

void clCloseCom();

void cl_setport(int b)
{
    if(port_type==PORT_TYPE_SERIAL)
    {
        EscapeCommFunction(hCom,(b&2)?SETRTS:CLRRTS);
        EscapeCommFunction(hCom,(b&1)?SETDTR:CLRDTR);
    }
    else    //if(port_type==PORT_TYPE_PARALLEL)
    {
        //_outp(parallel_address,b&3);
        asm push edx
        asm push eax
        asm mov eax,b
        asm mov edx,parallel_address
        asm out dx,al
        asm pop eax
        asm pop edx
    }
}

void clInitCom(int n,int timeout=0)
{
    if(n<=8)
    {
        char name[8],str[256];
        DCB dcb;
    
        if (clOpen)
        {
            clOpen++;
            return;
        }
        if (n==-99)
        {
            clVirtual=1;
            cl_timeout=timeout;
            clOpen=1;
            hCom=NULL;
            return;
        }
        else
            clVirtual=0;
        if (n>4)
        {
            n-=4;
            graphLink=0;
        }
        else
            graphLink=1;
        if (n>4)
            sprintf(name,"LPT%d:",n);
        else
            sprintf(name,"COM%d:",n);
        hCom=CreateFile(name,GENERIC_READ|GENERIC_WRITE,0,NULL,
            OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if (!hCom)
        {
            MessageBox(NULL,"Error opening COM port","Error",MB_OK);
            return;
        }
        dcb.DCBlength=sizeof(DCB);
        dcb.BaudRate=CBR_9600; dcb.ByteSize=8; dcb.Parity=0;
        dcb.StopBits=graphLink?0:1;
        dcb.fNull=0; dcb.fBinary=1; dcb.fParity=0;
        dcb.fOutxCtsFlow=1; dcb.fOutxDsrFlow=1;
        dcb.fDtrControl=0;
        dcb.fOutX=0; dcb.fInX=0;
        dcb.fErrorChar=0; dcb.fRtsControl=RTS_CONTROL_ENABLE;
        SetupComm(hCom,1024,1024);
        GetCommState(hCom,&dcb);
        sprintf(str,"COM%d: baud=9600 parity=N data=8 stop=1",n);
        BuildCommDCB(str,&dcb);
        dcb.fNull=0;
        SetCommState(hCom,&dcb);
        COMMTIMEOUTS to;
        to.ReadIntervalTimeout=MAXDWORD;
        to.ReadTotalTimeoutMultiplier=0;
        to.ReadTotalTimeoutConstant=cl_timeout=timeout;
        to.WriteTotalTimeoutMultiplier=0;
        to.WriteTotalTimeoutConstant=timeout;
        SetCommTimeouts(hCom,&to);
        clErrorFlag=0;
        clOpen=1;

        port_type=PORT_TYPE_SERIAL;
    }
    else
    {
        clVirtual=0;
        graphLink=0;
        cl_timeout=timeout;
        port_type=PORT_TYPE_PARALLEL;
        parallel_address=(n==9?0x378:n==10?0x278:0x3bc);
        hCom=NULL;
    }
    if (!graphLink)
        cl_setport(3);
    Sleep(20);
}

void clCloseCom()
{
    openLink=0;
    clOpen--;
    if (clOpen)
        return;
	if (hCom) CloseHandle(hCom);
}

int cl_getport()
{
    DWORD s;
    int b;

    if(port_type==PORT_TYPE_SERIAL)
    {
        GetCommModemStatus(hCom,&s);
        b=(s&MS_CTS_ON?1:0)|(s&MS_DSR_ON?2:0);
    }
    else    //if(port_type==PORT_TYPE_PARALLEL)
    {
        //b=(_inp(parallel_address+1)>>4)&3;
        asm push eax
        asm push edx
        asm mov edx,parallel_address
        asm inc edx
        asm in al,dx
        asm shr al,4
        asm and eax,3
        asm mov b,eax
        asm pop edx
        asm pop eax
    }
    return b;
}

void clSendCom(int ch)
{
    DWORD i;
    if (clVirtual)
    {
        DWORD startTime=GetTickCount();
        while (!PutVSendReady())
        {
            Sleep(50);
            if ((GetTickCount()-startTime)>250)
                return;
        }
        PutVSendBuf(ch);
        return;
    }
    if (graphLink)
        WriteFile(hCom,&ch,1,&i,NULL);
    else
    {
        DWORD startTime=GetTickCount();
        for (i=0;i<8;i++)
        {
            while (cl_getport()!=3)
            {
                if ((GetTickCount()-startTime)>250)
                {
                    cl_setport(3);
                    return;
                }
            }
//            asm cli
            if (ch&1)
                cl_setport(2);
            else
                cl_setport(1);
            while (cl_getport()!=0)
            {
                if ((GetTickCount()-startTime)>250)
                {
                    cl_setport(3);
                    return;
                }
            }
            cl_setport(3);
            while (cl_getport()!=3)
            {
                if ((GetTickCount()-startTime)>250)
                {
                    cl_setport(3);
                    return;
                }
            }
//            asm sti
            ch>>=1;
        }
    }
}

int clGetError()
{
    return clErrorFlag;
}

void clResetError()
{
    clErrorFlag=0;
}

void clFlushInput()
{
    DWORD i,j,bit;
    unsigned char c,ch;
    if (clVirtual)
    {
        DWORD startTime=GetTickCount();
        while (1)
        {
            if (CheckVRecvBuf())
            {
                GetVRecvBuf();
                startTime=GetTickCount();
            }
            else
                Sleep(50);
            if ((GetTickCount()-startTime)>50)
                return;
        }
    }
    if (graphLink)
    {
        DWORD startTime=GetTickCount();
        while (1)
        {
            i=0;
            ReadFile(hCom,&c,1,&i,NULL);
            if (i!=0)
                startTime=GetTickCount();
            else
                Sleep(50);
            if ((GetTickCount()-startTime)>50)
                return;
        }
    }
    DWORD startTime=GetTickCount();
    while (1)
    {
        for (i=0,bit=1,ch=0;i<8;i++)
        {
            while ((j=cl_getport())==3)
            {
                if ((GetTickCount()-startTime)>50)
                    return;
            }
//            asm cli
            if (j==1)
            {
                ch|=bit;
                cl_setport(1);
                j=2;
            }
            else
            {
                cl_setport(2);
                j=1;
            }
            while ((cl_getport()&j)==0)
            {
                if ((GetTickCount()-startTime)>50)
                {
//                    asm sti
                    return;
                }
            }
            cl_setport(3);
//            asm sti
            bit<<=1;
            startTime=GetTickCount();
        }
    }
}

int clGetCom(int &ch)
{
    DWORD i,j,bit;
    unsigned char c;
    if (clVirtual)
    {
        if (!cl_timeout)
        {
            if (!CheckVRecvBuf())
                return 0;
        }
        DWORD startTime=GetTickCount();
        int timeout=cl_timeout?cl_timeout:1000;
        while (!CheckVRecvBuf())
        {
            Sleep(50);
            if ((GetTickCount()-startTime)>timeout)
            {
                clErrorFlag=1;
                return 0;
            }
        }
        ch=GetVRecvBuf();
        return 1;
    }
    if (graphLink)
    {
        if (!ReadFile(hCom,&c,1,&i,NULL))
            clErrorFlag=1;
        if (!i) clErrorFlag=1;
        ch=c;
        return i;
    }
    if (!cl_timeout)
    {
        if (cl_getport()==3)
            return 0;
    }
    DWORD startTime=GetTickCount();
    int timeout=cl_timeout?cl_timeout:1000;
    for (i=0,bit=1,ch=0;i<8;i++)
    {
        while ((j=cl_getport())==3)
        {
            if ((GetTickCount()-startTime)>timeout)
            {
                clErrorFlag=1;
                cl_setport(3);
                return i;
            }
        }
//        asm cli
        if (j==1)
        {
            ch|=bit;
            cl_setport(1);
            j=2;
        }
        else
        {
            cl_setport(2);
            j=1;
        }
        while ((cl_getport()&j)==0)
        {
            if ((GetTickCount()-startTime)>timeout)
            {
                clErrorFlag=1;
//                asm sti
                cl_setport(3);
                return i;
            }
        }
        cl_setport(3);
//        asm sti
        bit<<=1;
    }
    return 8;
}

extern int plusMod;
int saveStateOnExit;
int lcdOnly=0;

bool restrict=true;

__fastcall TEmuWnd::TEmuWnd(TComponent* Owner)
    : TForm(Owner)
{
    emuLink=0; openLink=0; realKeypad=0;
    saveStateOnExit=1;
    keyHeld=-1; genTab=1;
    enforceRatio=0;
    otherInDebug=0;
    lcdUpdateEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
}
//---------------------------------------------------------------------------
extern char *ScreenLine[256];

//IDirectInput *lpDI;
//IDirectInputDevice *lpKeyboard;
extern HINSTANCE hInst;
bool minimizeOnDeactivate=true;

int IsCloseEnough(int color1,int color2)
{
    int r1=(color1>>16)&0xff;
    int g1=(color1>>8)&0xff;
    int b1=color1&0xff;
    int r2=(color2>>16)&0xff;
    int g2=(color2>>8)&0xff;
    int b2=color2&0xff;
    if ((abs(r1-r2)+abs(g1-g2)+abs(b1-b2))<48)
        return 1;
    return 0;
}

unsigned SkinColor(int x,int y)
{
    unsigned char *ptr=&((unsigned char *)skinImageLg->ScanLine[y])[x*3];
    return ptr[0]|(ptr[1]<<8)|(ptr[2]<<16);
}

int CanSkinFloat()
{
    if (!skinImageLg) return false;
    unsigned color=SkinColor(0,0);
    for (int x=0;x<skinImageLg->Width;x++)
    {
        if (!IsCloseEnough(SkinColor(x,0),color))
            return false;
        if (!IsCloseEnough(SkinColor(x,skinImageLg->Height-1),color))
            return false;
    }
    for (int y=0;y<skinImageLg->Height;y++)
    {
        if (!IsCloseEnough(SkinColor(0,y),color))
            return false;
        if (!IsCloseEnough(SkinColor(skinImageLg->Width-1,y),color))
            return false;
    }
    return true;
}

HRGN CreateFloatRgn()
{
    HRGN rgn=NULL;
    unsigned color=SkinColor(0,0);
    for (int _y=0;_y<EmuWnd->ClientHeight;_y++)
    {
        int y=(_y*skinImageLg->Height)/EmuWnd->ClientHeight;
        int leftX,rightX;
        for (leftX=0;leftX<skinImageLg->Width;leftX++)
        {
            if (!(IsCloseEnough(SkinColor(leftX,y),color)||IsCloseEnough(SkinColor(leftX,y+1),color)))
                break;
        }
        if (leftX>=skinImageLg->Width)
            continue;
        for (rightX=skinImageLg->Width-1;rightX>0;rightX--)
        {
            if (!(IsCloseEnough(SkinColor(rightX,y),color)||IsCloseEnough(SkinColor(rightX,y+1),color)))
                break;
        }
        if (rightX<=0)
            continue;
        leftX=(leftX*EmuWnd->ClientWidth+(skinImageLg->Width-1))/skinImageLg->Width;
        rightX=(rightX*EmuWnd->ClientWidth+(skinImageLg->Width-1))/skinImageLg->Width;
        HRGN newRgn=CreateRectRgn(leftX,_y,rightX,_y+1);
        if (rgn)
        {
            CombineRgn(rgn,rgn,newRgn,RGN_OR);
            DeleteObject(newRgn);
        }
        else
            rgn=newRgn;
    }
    return rgn;
}

void __fastcall TEmuWnd::Make()
{
    minimizeOnDeactivate=false;
    enforceRatio=0;
    if (calc==82)
        Caption="Virtual TI-82";
    else if (calc==83)
        Caption="Virtual TI-83";
    else if (calc==84)
        Caption="Virtual TI-83+";
    else if (calc==73)
        Caption="Virtual TI-73";
    else if (calc==85)
        Caption="Virtual TI-85";
    else if (calc==86)
        Caption="Virtual TI-86";
    else if (calc==89)
        Caption="Virtual TI-89";
    else if ((calc==92)||(calc==93))
        Caption="Virtual TI-92";
    else if (calc==94)
        Caption="Virtual TI-92+";
    Application->Title=Caption;
    if (!Fullscreenview1->Checked)
    {
        if (CanSkinFloat())
            BorderStyle=bsNone;
        else
            BorderStyle=bsSizeable;
        FormStyle=(Stayontop1->Checked)?fsStayOnTop:fsNormal;
        AutoScroll=false;
        Application->ProcessMessages();
        int sf1w=(currentSkin==-1)?0:skinImageSm->Width;
        int sf1h=(currentSkin==-1)?0:skinImageSm->Height;
        int sf2w=(currentSkin==-1)?0:skinImageLg->Width;
        int sf2h=(currentSkin==-1)?0:skinImageLg->Height;
        if (currentSkin==-1)
        {
            Normal1xview1->Checked=(scaleFact==1);
            Large2xcalculatorimage1->Checked=(scaleFact==2);
            ClientWidth=lcdWidth*scaleFact;
            ClientHeight=lcdHeight*scaleFact;
            LCD->Left=skinLcd.left/(3-scaleFact);
            LCD->Top=skinLcd.top/(3-scaleFact);
            LCD->ClientWidth=lcdWidth*scaleFact;
            LCD->ClientHeight=lcdHeight*scaleFact;
        }
        else if ((skin[currentSkin].defW)&&
                 (skin[currentSkin].defH))
        {
            ClientWidth=skin[currentSkin].defW;
            ClientHeight=skin[currentSkin].defH;
            Image->Width=skin[currentSkin].defW;
            Image->Height=skin[currentSkin].defH;
            skin[currentSkin].defW=0;
            skin[currentSkin].defH=0;
            Normal1xview1->Checked=false;
            Large2xcalculatorimage1->Checked=false;
            if (((abs((Image->Width-sf1w)))<5)&&
                ((abs((Image->Height-sf1h)))<5))
            {
                Normal1xview1->Checked=true;
                scaleFact=1;
                ClientWidth=skinImageSm->Width;
                ClientHeight=skinImageSm->Height;
                Image->Width=skinImageSm->Width;
                Image->Height=skinImageSm->Height;
                LCD->Left=skinLcd.left/(3-scaleFact);
                LCD->Top=skinLcd.top/(3-scaleFact);
                LCD->ClientWidth=lcdWidth*scaleFact;
                LCD->ClientHeight=lcdHeight*scaleFact;
            }
            else if (((abs((Image->Width-sf2w)))<5)&&
                     ((abs((Image->Height-sf2h)))<5))
            {
                Large2xcalculatorimage1->Checked=true;
                scaleFact=2;
                ClientWidth=skinImageLg->Width;
                ClientHeight=skinImageLg->Height;
                Image->Width=skinImageLg->Width;
                Image->Height=skinImageLg->Height;
                LCD->Left=skinLcd.left/(3-scaleFact);
                LCD->Top=skinLcd.top/(3-scaleFact);
                LCD->ClientWidth=lcdWidth*scaleFact;
                LCD->ClientHeight=lcdHeight*scaleFact;
            }
            else
            {
                if (LCD->Width>lcdWidth)
                    scaleFact=2;
                else
                    scaleFact=1;
                int imgW=(skinImageLg)?skinImageLg->Width:(lcdWidth<<1);
                int imgH=(skinImageLg)?skinImageLg->Height:(lcdHeight<<1);
                LCD->Left=skinLcd.left*Image->Width/imgW;
                LCD->Width=(skinLcd.right-skinLcd.left)*Image->Width/imgW;
                LCD->Top=skinLcd.top*Image->Height/imgH;
                LCD->Height=(skinLcd.bottom-skinLcd.top)*Image->Height/imgH;
            }
        }
        else if (scaleFact==1)
        {
            Normal1xview1->Checked=true;
            Large2xcalculatorimage1->Checked=false;
            ClientWidth=skinImageSm->Width;
            ClientHeight=skinImageSm->Height;
            Image->Width=skinImageSm->Width;
            Image->Height=skinImageSm->Height;
            LCD->Left=skinLcd.left/(3-scaleFact);
            LCD->Top=skinLcd.top/(3-scaleFact);
            LCD->ClientWidth=lcdWidth*scaleFact;
            LCD->ClientHeight=lcdHeight*scaleFact;
        }
        else
        {
            Normal1xview1->Checked=false;
            Large2xcalculatorimage1->Checked=true;
            ClientWidth=skinImageLg->Width;
            ClientHeight=skinImageLg->Height;
            Image->Width=skinImageLg->Width;
            Image->Height=skinImageLg->Height;
            LCD->Left=skinLcd.left/(3-scaleFact);
            LCD->Top=skinLcd.top/(3-scaleFact);
            LCD->ClientWidth=lcdWidth*scaleFact;
            LCD->ClientHeight=lcdHeight*scaleFact;
        }
        calcImage=skinImageLg;
        None1->Checked=(currentSkin==-1);
        while (Currentskin1->Count>1)
            Currentskin1->Delete(1);
        for (int i=0;i<skinCount;i++)
        {
            if ((calc==skin[i].calc)||((calc>=92)&&
                (skin[i].calc>=92)))
            {
                TMenuItem *itm=new TMenuItem(Currentskin1);
                itm->Caption=skin[i].name;
                itm->Checked=(i==currentSkin);
                itm->OnClick=OnSkinSelect;
                Currentskin1->Add(itm);
            }
        }
        if (CanSkinFloat())
        {
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            skinFloating=1;
        }
        else
        {
            SetWindowRgn(Handle,CreateRectRgn(0,0,Width,Height),true);
            skinFloating=0;
        }
    }
    else
    {
        BorderStyle=bsNone;
        FormStyle=fsStayOnTop;
        AutoScroll=false;
        Application->ProcessMessages();
        Left=0; Top=0;
        Width=GetDeviceCaps(Canvas->Handle,HORZRES);
        Height=GetDeviceCaps(Canvas->Handle,VERTRES);
        LCD->Left=0; LCD->Width=Width;
        LCD->Top=0; LCD->Height=Height;
        skinFloating=0;
    }
    PauseStart();
    bmp->Width=lcdWidth*scaleFact;
    bmp->Height=lcdHeight*scaleFact;
    bmp->PixelFormat=pf32bit;
    for (int i=0;i<(lcdHeight*scaleFact);i++)
        ScreenLine[i]=(char *)bmp->ScanLine[i];
    PauseEnd();
    enforceRatio=1;
    minimizeOnDeactivate=true;
    SpeedIndic->Left=LCD->Left+LCD->Width-SpeedIndic->Width;
    SpeedIndic->Top=LCD->Top-8-SpeedIndic->Height;
    if (skinFloating)
        SpeedIndic->Visible=showSpeed;
}

int lcdUpdate;
void __fastcall TEmuWnd::FormCreate(TObject *Sender)
{
    lcdUpdate=0;

    Application->OnIdle=TEmuWnd::OnIdle;
//    Application->OnActivate=FormActivate;
    Application->OnDeactivate=FormDeactivate;
    bmp=new Graphics::TBitmap;
    bmp->Width=480;
    bmp->Height=256;
    bmp->PixelFormat=pf32bit;
    for (int i=0;i<256;i++)
        ScreenLine[i]=(char *)bmp->ScanLine[i];

    TRegistry *reg=new TRegistry;
    reg->RootKey=HKEY_LOCAL_MACHINE;
    reg->OpenKey("\\Software\\ACZ\\Virtual TI",true);
    if (reg->ValueExists("StayOnTop"))
    {
        if (reg->ReadInteger("StayOnTop"))
            Stayontop1->Checked=true;
        else
            Stayontop1->Checked=false;
    }
    if (reg->ValueExists("FullScreen"))
    {
        if (reg->ReadInteger("FullScreen"))
        {
            Fullscreenview1->Checked=true;
            Normal1xview1->Checked=false;
            Large2xcalculatorimage1->Checked=false;
            Enterdebugmode1->Enabled=false;
            Currentskin1->Enabled=false;
            Setprogramentrybreakpoint1->Enabled=false;
            Showpercentageofactualspeed1->Enabled=false;
            Stayontop1->Enabled=false;
        }
        else
            Fullscreenview1->Checked=false;
    }
    reg->CloseKey();
    delete reg;

//	// Create DirectInput object
//    lpDI=NULL; lpKeyboard=NULL;
//	if (DirectInputCreate(hInst,DIRECTINPUT_VERSION,&lpDI,NULL)!=DI_OK)
//    {
//		MessageBox(NULL,"Could not initialize DirectInput","Error",MB_OK);
//        return;
//    }
//	// Create and set up the keyboard device
//	if (lpDI->CreateDevice(GUID_SysKeyboard,(LPDIRECTINPUTDEVICE*)&lpKeyboard,
//		NULL)!=DI_OK)
//    {
//		MessageBox(NULL,"Could not access keyboard from DirectInput","Error",MB_OK);
//        return;
//    }
//	lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
//	lpKeyboard->SetCooperativeLevel(Handle,DISCL_EXCLUSIVE|
//		DISCL_FOREGROUND);
//	DIPROPDWORD dpd;
//	dpd.diph.dwSize=sizeof(DIPROPDWORD);
//	dpd.diph.dwHeaderSize=sizeof(DIPROPHEADER);
//	dpd.diph.dwObj=0;
//	dpd.diph.dwHow=DIPH_DEVICE;
//	dpd.dwData=64;
//	lpKeyboard->SetProperty(DIPROP_BUFFERSIZE,&dpd.diph);
//
//    lpKeyboard->Acquire();

    Restricttoactualspeed1->Checked=restrict;

//    ShowLCDonly1->Checked=lcdOnly;

    VirtualLink1->Enabled=otherWnd?true:false;

    if (otherWnd)
    {
        SendMessage(otherWnd,WM_HELLO,0,(LPARAM)Handle);
        ATOM a=GlobalAddAtom(vLinkFileName);
        SendMessage(otherWnd,WM_SEND_BUFFER,0,(LPARAM)a);
        GlobalDeleteAtom(a);
    }

    emuThread=new EmuThread(false);

    Make();

    DragAcceptFiles(Handle,true);
}
//---------------------------------------------------------------------------
extern int contrast,debug,run,debugStartPC;
extern int lcdUpdateNeeded;
//unsigned long perfFreq=0;
extern int recvCount;
void __fastcall TEmuWnd::OnIdle(TObject *,bool& done)
{
//    WaitForSingleObject(lcdUpdateEvent,50);
    Sleep(10);
    done=false;
    if (showDebugWnd)
    {
        DebugWnd->Show();
        DebugWnd->Update();
        showDebugWnd=false;
    }
    int curLcdUpdate=lcdUpdateNeeded;
    if (curLcdUpdate)
    {
        TRect r;
        r.Left=0; r.Right=LCD->Width;
        r.Top=0; r.Bottom=LCD->Height;
        LCD->Canvas->StretchDraw(r,bmp);
        lcdUpdateNeeded-=curLcdUpdate;
    }
    int curPercentChange=percentChange;
    if (showSpeed&&curPercentChange)
    {
        char str[64];
        sprintf(str,"%s - %d%% speed",
            (calc==89)?"Virtual TI-89":((calc==94)?
            "Virtual TI-92+":(calc==82)?"Virtual TI-82":
            (calc==83)?"Virtual TI-83":(calc==84)?
            "Virtual TI-83+":(calc==73)?
            "Virtual TI-73":(calc==85)?
            "Virtual TI-85":(calc==86)?"Virtual TI-86":
            "Virtual TI-92"),percent);
        Caption=str;
        sprintf(str,"%d%%",percent);
        SpeedIndic->Caption=str;
        percentChange-=curPercentChange;
    }
    if (Active)
    {
        if (((GetKeyState(VK_TAB)&0x8000)==0x8000)&&(genTab))
        {
            TMessage msg;
            msg.WParam=VK_TAB;
            msg.LParam=0;
            FormKeyDown(msg);
            genTab=0;
        }
    }
//    if (emuLink)
//    {
//        char str[64];
//        sprintf(str,"%d%s,%d",recvCount,(vRecvBuf->start==
//            vRecvBuf->end)?" ":"*",(sendBuf.end-sendBuf.start)&255);
//        Caption=str;
//    }
    DoLink();
/*    if (!perfFreq)
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        perfFreq=li.u.LowPart/50;
    }
    if ((!debug)||(run))
    {
        int i;
        for (i=0;i<=31;i++)
        {
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
            hw->Execute();
        }
        if (restrict)
        {
            static int lastTime=0;
            LARGE_INTEGER li;
            if (!lastTime)
            {
                QueryPerformanceCounter(&li);
                lastTime=li.u.LowPart;
            }
            QueryPerformanceCounter(&li);
            int curTime=li.u.LowPart;
            int time=curTime-lastTime;
            if (time<perfFreq)
            {
                while (time<perfFreq)
                {
                    Sleep(0);
                    QueryPerformanceCounter(&li);
                    curTime=li.u.LowPart;
                    time=curTime-lastTime;
                }
                lastTime+=perfFreq;
            }
            else
                lastTime=curTime;
        }
        if (showSpeed&&run)
        {
            static int ticks=0; ticks+=200;
            static int lastTime=GetTickCount();
            int curTime=GetTickCount();
            if ((curTime-lastTime)>=1000)
            {
                char str[64];
                sprintf(str,"%s - %d%% speed",
                    (calc==89)?"Virtual TI-89":((calc==94)?
                    "Virtual TI-92+":(calc==82)?"Virtual TI-82":
                    (calc==83)?"Virtual TI-83":(calc==84)?
                    "Virtual TI-83+":(calc==73)?
                    "Virtual TI-73":(calc==85)?
                    "Virtual TI-85":(calc==86)?"Virtual TI-86":
                    "Virtual TI-92"),
                    (ticks*100)/(9875*(curTime-lastTime)/1000));
                Caption=str;
                lastTime=curTime;
                ticks=0;
            }
        }
        static int lcdRefresh=0; lcdRefresh++;
        if (lcdRefresh>1)
        {
            lcdRefresh=0;
            TRect r;
            r.Left=0; r.Right=LCD->Width;
            r.Top=0; r.Bottom=LCD->Height;
            LCD->Canvas->StretchDraw(r,bmp);
        }
        DoLink();
//        if (lpKeyboard)
//        {
//            DIDEVICEOBJECTDATA data[64];
//            DWORD count=64;
//            int err=lpKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
//                data,&count,0);
//            if (err==DIERR_INPUTLOST)
//            {
//                lpKeyboard->Acquire();
//                return;
//            }
//            else if (err!=DI_OK)
//                return;
//            for (i=0;i<count;i++)
//            {
//                int j;
//                if ((data[i].dwOfs==DIK_F10)&&(data[i].dwData&0x80))
//                    SendFile1Click(NULL);
//                else if ((data[i].dwOfs==DIK_F11)&&(data[i].dwData&0x80))
//                    Enterdebugmode1Click(NULL);
//                else if ((data[i].dwOfs==DIK_F12)&&(data[i].dwData&0x80))
//                    Setemulationmode1Click(NULL);
//                else
//                {
//                    int* keymap=hw->GetKeyMap();
//                    for (j=0;keymap[j]!=-1;j+=2)
//                    {
//                        if (data[i].dwOfs==keymap[j])
//                        {
//                            if (data[i].dwData&0x80)
//                                kdown(keymap[j+1]);
//                            else
//                                kup(keymap[j+1]);
//                            break;
//                        }
//                    }
//                }
//            }
//        }
    }
    else
    {
        if (lcdUpdate)
        {
            TRect r;
            r.Left=0; r.Top=LCD->Width;
            r.Right=0; r.Bottom=LCD->Height;
            LCD->Canvas->StretchDraw(r,bmp);
            lcdUpdate=0;
        }
        Sleep(0);
    }*/
}


void __fastcall TEmuWnd::LCDPaint(TObject *Sender)
{
    if (!bmp) return;
    TRect r;
    r.Left=0; r.Right=LCD->Width;
    r.Top=0; r.Bottom=LCD->Height;
    LCD->Canvas->StretchDraw(r,bmp);
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::LCDMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    TPoint p;
    if (Button==mbRight)
    {
        PauseStart();
        p.x=X; p.y=Y; p=LCD->ClientToScreen(p);
        PopupMenu1->Popup(p.x,p.y);
        PauseEnd();
    }
}
//---------------------------------------------------------------------------

//extern int TransferFile(char *fn);
void __fastcall TEmuWnd::SendFile1Click(TObject *Sender)
{
    PauseStart();
    TOpenDialog *dlog=new TOpenDialog(this);
    switch(calc)
    {
        case 82:
            dlog->DefaultExt="82p";
            dlog->FileName="*.82?";
            dlog->FilterIndex=1;
            break;
        case 83:
            dlog->DefaultExt="83p";
            dlog->FileName="*.83?";
            dlog->FilterIndex=2;
            break;
        case 84:
            dlog->DefaultExt="8xp";
            dlog->FileName="*.8x?";
            dlog->FilterIndex=3;
            break;
        case 85:
            dlog->DefaultExt="85p";
            dlog->FileName="*.85?";
            dlog->FilterIndex=4;
            break;
        case 86:
            dlog->DefaultExt="86p";
            dlog->FileName="*.86?";
            dlog->FilterIndex=5;
            break;
        case 89:
            dlog->DefaultExt="89z";
            dlog->FileName="*.89?";
            dlog->FilterIndex=6;
            break;
        case 92: case 93:
            dlog->DefaultExt="92p";
            dlog->FileName="*.92?";
            dlog->FilterIndex=7;
            break;
        case 94:
            dlog->DefaultExt="9xz";
            dlog->FileName="*.9x?";
            dlog->FilterIndex=8;
            break;
    }
    dlog->Options.Clear();
    dlog->Options << ofFileMustExist << ofPathMustExist << ofAllowMultiSelect;
    dlog->Filter="TI-82 files (*.82?)|*.82?|TI-83 files (*.83?)|*.83?|TI-83 Plus files (*.8x?)|*.8x?|TI-85 files (*.85?)|*.85?|TI-86 files (*.86?)|*.86?|TI-89 files (*.89?)|*.89?|TI-92 files (*.92?)|*.92?|TI-92 Plus files (*.9x?)|*.9x?";
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
        Screen->Cursor=crHourGlass;
        for (int i=0;i<dlog->Files->Count;)
        {
            if (hw->SendFile(dlog->Files->Strings[i].
                c_str(),i<(dlog->Files->Count-1)))
            {
                i++;
                continue;
            }
            Screen->Cursor=crDefault;
            if (MessageBox(Handle,"Transmission error","Error",
                MB_RETRYCANCEL|MB_ICONEXCLAMATION)==IDCANCEL)
                break;
            Screen->Cursor=crHourGlass;
        }
        Screen->Cursor=crDefault;
    }
    delete dlog;
    PauseEnd();
}
//---------------------------------------------------------------------------

//extern void ResetEmu();
void __fastcall TEmuWnd::Reset1Click(TObject *Sender)
{
    PauseStart();
    hw->Reset();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::FormDestroy(TObject *Sender)
{
    DisableSound();
    SetEvent(emuEvent);
    exitEmu=true;
    while (exitEmu)
        Sleep(0);
    CloseSkin();
    if (calc==82) scaleDefault[0]=scaleFact;
    else if ((calc==83)||(calc==84)) scaleDefault[1]=scaleFact;
    else if (calc==73) scaleDefault[6]=scaleFact;
    else if (calc==85) scaleDefault[2]=scaleFact;
    else if (calc==86) scaleDefault[3]=scaleFact;
    else if (calc==89) scaleDefault[4]=scaleFact;
    else scaleDefault[5]=scaleFact;
    TRegistry *reg=new TRegistry;
    reg->RootKey=HKEY_LOCAL_MACHINE;
    reg->OpenKey("\\Software\\ACZ\\Virtual TI",true);
    reg->WriteString("LastEmuROM",romImage[currentROM].name);
    reg->WriteInteger("RestrictSpeed",restrict);
    reg->WriteInteger("LCDOnly",lcdOnly);
    reg->WriteInteger("StayOnTop",Stayontop1->Checked?1:0);
    reg->WriteInteger("FullScreen",Fullscreenview1->Checked?1:0);
    reg->OpenKey("EmuScaleDefault",true);
    for (int i=0;i<7;i++)
    {
        char str[4];
        sprintf(str,"%d",i);
        reg->WriteInteger(str,scaleDefault[i]);
    }
    reg->OpenKey("\\Software\\ACZ\\Virtual TI",true);
    reg->OpenKey("EmuKeyMap",true);
    for (int i=k_q;i<=k_m;i++)
    {
        int j;
        int* keymap=hw->GetKeyMap();
        for (j=0;keymap[j]!=-1;j+=2)
        {
            if (keymap[j+1]==i)
            {
                char str[4];
                sprintf(str,"%s",alphakeystr[i-k_q]);
                reg->WriteInteger(str,keymap[j]);
                break;
            }
        }
    }
    reg->CloseKey();
    if (saveStateOnExit)
    {
        chdir(initPath);
        SaveState();
    }
//    if (lpKeyboard) lpKeyboard->Unacquire();
//    if (lpKeyboard) lpKeyboard->Release();
//    lpKeyboard=NULL;
//    if (lpDI) lpDI->Release();
    if (openLink) clCloseCom();
    if (otherWnd)
        SendMessage(otherWnd,WM_GOODBYE,0,0);
    CloseHandle(lcdUpdateEvent);
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::COM1Click(TObject *Sender)
{
    if (emuLink==1)
    {
        emuLink=0;
        COM1->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=1;
        COM1->Checked=true;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::COM2Click(TObject *Sender)
{
    if (emuLink==2)
    {
        emuLink=0;
        COM2->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=2;
        COM1->Checked=false;
        COM2->Checked=true;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::COM3Click(TObject *Sender)
{
    if (emuLink==3)
    {
        emuLink=0;
        COM3->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=3;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=true;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::COM4Click(TObject *Sender)
{
    if (emuLink==4)
    {
        emuLink=0;
        COM4->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=4;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=true;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::FormDeactivate(TObject *Sender)
{
    if ((Fullscreenview1->Checked)&&(minimizeOnDeactivate))
        Application->Minimize();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::About1Click(TObject *Sender)
{
    PauseStart();
    AboutDlg->ShowModal();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Enterdebugmode1Click(TObject *Sender)
{
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    debug=1; run=0;
    while (emuRunning)
        Sleep(0);
    debugStartPC=z80?R.PC.D:regs.pc;
    DebugWnd->Show();
    DebugWnd->Update();
}
//---------------------------------------------------------------------------

#define MODE_TI89 0
#define MODE_TI92 1
#define MODE_TI92PLUS 2
#define MODE_TI92PLUSINT 3

extern void CloseEmu();
extern int currentROM;
void __fastcall TEmuWnd::Setemulationmode1Click(TObject *Sender)
{
    PauseStart();
    run=0;
    EmuModeDlg->Calc->ItemIndex=currentROM;
    if (EmuModeDlg->ShowModal()==1)
    {
        if (EmuModeDlg->Calc->ItemIndex!=-1)
        {
//            chdir(initPath);
            SaveState();
            CloseSkin();
            CloseEmu();
            InitEmu(EmuModeDlg->romID[EmuModeDlg->Calc->ItemIndex]);
            TRegistry *reg=new TRegistry;
            reg->RootKey=HKEY_LOCAL_MACHINE;
            reg->OpenKey("\\Software\\ACZ\\Virtual TI\\Skins",true);
            char keyName[24];
            switch (calc)
            {
                case 73: strcpy(keyName,"DefaultSkin73"); break;
                case 82: strcpy(keyName,"DefaultSkin82"); break;
                case 83: strcpy(keyName,"DefaultSkin83"); break;
                case 84: strcpy(keyName,"DefaultSkin83Plus"); break;
                case 85: strcpy(keyName,"DefaultSkin85"); break;
                case 86: strcpy(keyName,"DefaultSkin86"); break;
                case 89: strcpy(keyName,"DefaultSkin89"); break;
                case 92: case 93: strcpy(keyName,"DefaultSkin92"); break;
                case 94: strcpy(keyName,"DefaultSkin92Plus"); break;
            }
            if (reg->ValueExists(keyName))
            {
                char skinName[32];
                strcpy(skinName,reg->ReadString(keyName).c_str());
                int found=-1;
                for (int i=0;i<skinCount;i++)
                {
                    if (!strcmp(skinName,skin[i].file))
                    {
                        found=i;
                        break;
                    }
                }
                if (found!=-1)
                {
                    LoadSkin(found);
                    AboutDlg->UpdateText();
                }
            }
            else
            {
                for (int i=0;i<skinCount;i++)
                {
                    if ((calc==skin[i].calc)||
                        ((calc>=92)&&(skin[i].calc>=92)))
                    {
                        LoadSkin(i);
                        break;
                    }
                }
            }
            reg->CloseKey();
            delete reg;
            Make();
            LoadState();
            AboutDlg->UpdateText();
//            useInitPath=1;
        }
    }
    run=1;
    PauseEnd();
}
//---------------------------------------------------------------------------

extern int debugProgBreak,debugProgBreakOfs,debugProgLen;
void __fastcall TEmuWnd::Setprogramentrybreakpoint1Click(TObject *Sender)
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

void __fastcall TEmuWnd::Savestateimage1Click(TObject *Sender)
{
    PauseStart();
    TSaveDialog *dlog=new TSaveDialog(this);
    dlog->DefaultExt="sav";
    char str[256];
    strcpy(str,romImage[currentROM].file);
    str[strlen(str)-3]='s';
    str[strlen(str)-2]='a';
    str[strlen(str)-1]='v';
    dlog->FileName=str;
    dlog->Options.Clear();
    dlog->Filter="State files (*.sav)|*.sav";
    dlog->FilterIndex=1;
    char cwd[256];
    getcwd(cwd,256);
    dlog->InitialDir=cwd;
    if (useInitPath)
    {
        useInitPath=0;
        dlog->InitialDir=initPath;
    }
    if (dlog->Execute())
        hw->SaveState(dlog->FileName.c_str());
    delete dlog;
    PauseEnd();
}
//---------------------------------------------------------------------------

extern int calcChanged;
void __fastcall TEmuWnd::Loadstateimage1Click(TObject *Sender)
{
    PauseStart();
    TOpenDialog *dlog=new TOpenDialog(this);
    dlog->DefaultExt="sav";
    dlog->FileName="*.sav";
    dlog->Options.Clear();
    dlog->Options << ofFileMustExist << ofPathMustExist;
    dlog->Filter="State files (*.sav)|*.sav";
    dlog->FilterIndex=1;
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
        char old[256];
        getcwd(old,256);
        chdir(initPath);
        FILE *fp=fopen(dlog->FileName.c_str(),"rb");
        char str[9],romName[56];
        fread(str,8,1,fp);
        fread(romName,56,1,fp);
        fclose(fp);
        str[8]=0;
        if ((strcmp(str,"VTIv2.0 "))&&(strcmp(str,"VTIv2.1 ")))
        {
            MessageBox(NULL,"Unrecognized file format","State Load Error",MB_OK|MB_ICONSTOP);
            PauseEnd();
            chdir(old);
            return;
        }
        if (!strcmp(romName,romImage[currentROM].name))
            hw->LoadState(dlog->FileName.c_str());
        else
        {
            SaveState();
            int newROM=-1;
            for (int i=0;i<romImageCount;i++)
            {
                if (!strcmp(romImage[i].name,romName))
                {
                    newROM=i;
                    break;
                }
            }
            if (newROM==-1)
            {
                MessageBox(NULL,"Required ROM not present","State Load Error",MB_OK|MB_ICONSTOP);
                PauseEnd();
                chdir(old);
                return;
            }
            CloseSkin();
            CloseEmu();
            InitEmu(newROM);
            TRegistry *reg=new TRegistry;
            reg->RootKey=HKEY_LOCAL_MACHINE;
            reg->OpenKey("\\Software\\ACZ\\Virtual TI\\Skins",true);
            char keyName[24];
            switch (calc)
            {
                case 73: strcpy(keyName,"DefaultSkin73"); break;
                case 82: strcpy(keyName,"DefaultSkin82"); break;
                case 83: strcpy(keyName,"DefaultSkin83"); break;
                case 84: strcpy(keyName,"DefaultSkin83Plus"); break;
                case 85: strcpy(keyName,"DefaultSkin85"); break;
                case 86: strcpy(keyName,"DefaultSkin86"); break;
                case 89: strcpy(keyName,"DefaultSkin89"); break;
                case 92: case 93: strcpy(keyName,"DefaultSkin92"); break;
                case 94: strcpy(keyName,"DefaultSkin92Plus"); break;
            }
            if (reg->ValueExists(keyName))
            {
                char skinName[32];
                strcpy(skinName,reg->ReadString(keyName).c_str());
                int found=-1;
                for (int i=0;i<skinCount;i++)
                {
                    if (!strcmp(skinName,skin[i].file))
                    {
                        found=i;
                        break;
                    }
                }
                if (found!=-1)
                {
                    LoadSkin(found);
                    AboutDlg->UpdateText();
                }
            }
            else
            {
                for (int i=0;i<skinCount;i++)
                {
                    if ((calc==skin[i].calc)||
                        ((calc>=92)&&(skin[i].calc>=92)))
                    {
                        LoadSkin(i);
                        break;
                    }
                }
            }
            reg->CloseKey();
            delete reg;
            hw->LoadState(dlog->FileName.c_str());
            Make();
            AboutDlg->UpdateText();
        }
        chdir(old);
    }
    delete dlog;
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Exitwithoutsavingstate1Click(TObject *Sender)
{
    PauseStart();
//    chdir(initPath);
    LoadState();
//    useInitPath=1;
    PauseEnd();
}
//---------------------------------------------------------------------------

extern int oldEmu;
void __fastcall TEmuWnd::UseNewClick(TObject *Sender)
{
    MakeSR();
    oldEmu=!oldEmu;
//    UseNew->Checked=!oldEmu;
    MakeFromSR();
    MakeSR();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::FormKeyDown(TMessage &Msg)
{
    int Key;
    Key=Msg.WParam;
    if ((Key==13)&&(Msg.LParam&0x1000000))
        Key=VK_SEPARATOR;
    if (Key==VK_F10)
        SendFile1Click(NULL);
    else if ((Key==VK_F11)&&(Enterdebugmode1->Enabled))
        Enterdebugmode1Click(NULL);
    else if (Key==VK_F12)
        Setemulationmode1Click(NULL);
    else if (Key==VK_APPS)
    {
        POINT p;
        p.x=10; p.y=10; p=LCD->ClientToScreen(p);
        PopupMenu1->Popup(p.x,p.y);
    }
    else
    {
        int* keymap=hw->GetKeyMap();
        for (int j=0;keymap[j]!=-1;j+=2)
        {
            if (Key==keymap[j])
            {
                kdown(keymap[j+1]);
                break;
            }
        }
    }
    Msg.Result=0;
}
//---------------------------------------------------------------------------void __fastcall TEmuWnd::FormKeyUp(TObject *Sender, WORD &Key,
void __fastcall TEmuWnd::FormKeyUp(TMessage &Msg)
{
    int Key;
    Key=Msg.WParam;
    if (Key==VK_TAB)
        genTab=1;
    if ((Key==13)&&(Msg.LParam&0x1000000))
        Key=VK_SEPARATOR;
    int* keymap=hw->GetKeyMap();
    for (int j=0;keymap[j]!=-1;j+=2)
    {
        if (Key==keymap[j])
        {
            kup(keymap[j+1]);
            break;
        }
    }
    Msg.Result=0;
}
//---------------------------------------------------------------------------

extern int DumpROM();
void __fastcall TEmuWnd::DumpROM1Click(TObject *Sender)
{
    PauseStart();
    Hide();
    DebugWnd->Hide();
    TROMWizardWnd *rw=new TROMWizardWnd(NULL);
    if (rw->ShowModal()==1)
    {
        MessageBox(Handle,"You will need to restart Virtual TI\n"
            "before using the new ROM image.","Virtual TI",
            MB_ICONINFORMATION|MB_OK);
    }
    Application->OnIdle=NULL;
    delete rw;
    Application->OnIdle=TEmuWnd::OnIdle;
    Show();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Restricttoactualspeed1Click(TObject *Sender)
{
    restrict=!restrict;
    Restricttoactualspeed1->Checked=restrict;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Redefinekeys1Click(TObject *Sender)
{
/*    if (!lpKeyboard) return;
    unacquire=false;
    RedefineWnd->Show();
    RedefineWnd->Update();
    int held=0;
    RedefineWnd->Key->Caption="Press new key for \"Q\"";
    for (int n=k_q;n<=k_m;)
    {
        DIDEVICEOBJECTDATA data[64];
        DWORD count=64;
        int err=lpKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
            data,&count,0);
        if (err==DIERR_INPUTLOST)
        {
            lpKeyboard->Acquire();
            continue;
        }
        else if (err!=DI_OK)
        {
            RedefineWnd->Hide();
            unacquire=true;
            return;
        }
        for (int i=0;i<count;i++)
        {
            int j;
            if (data[i].dwData&0x80)
            {
                if (held) continue;
                int* keymap=hw->GetKeyMap();
                for (j=0;keymap[j]!=-1;j+=2)
                {
                    if (n==keymap[j+1])
                    {
                        held=1;
                        keymap[j]=data[i].dwOfs;
                        break;
                    }
                }
            }
            else if (held)
            {
                held=0;
                n++;
                if (n>k_m) break;
                RedefineWnd->Key->Caption=AnsiString("Press new key for \"")+
                    AnsiString(alphakeystr[n-k_q])+AnsiString("\"");
                RedefineWnd->Update();
            }
        }
    }
    RedefineWnd->Hide();
    unacquire=true;*/
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Large2xcalculatorimage1Click(TObject *Sender)
{
    PauseStart();
    if (Fullscreenview1->Checked)
    {
        Fullscreenview1->Checked=false;
        Top=5; Left=100;
        Enterdebugmode1->Enabled=true;
        Currentskin1->Enabled=true;
        Setprogramentrybreakpoint1->Enabled=true;
        Showpercentageofactualspeed1->Enabled=true;
        Stayontop1->Enabled=true;
        FormStyle=(Stayontop1->Checked)?fsStayOnTop:fsNormal;
    }
    Normal1xview1->Checked=false;
    Large2xcalculatorimage1->Checked=true;
    scaleFact=2;
    Make();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Showpercentageofactualspeed1Click(
      TObject *Sender)
{
    showSpeed=!showSpeed;
    if (skinFloating)
        SpeedIndic->Visible=showSpeed;
    Showpercentageofactualspeed1->Checked=showSpeed;
    if (calc==82)
        Caption="Virtual TI-82";
    else if (calc==83)
        Caption="Virtual TI-83";
    else if (calc==84)
        Caption="Virtual TI-83+";
    else if (calc==73)
        Caption="Virtual TI-73";
    else if (calc==85)
        Caption="Virtual TI-85";
    else if (calc==86)
        Caption="Virtual TI-86";
    else if (calc==89)
        Caption="Virtual TI-89";
    else if ((calc==92)||(calc==93))
        Caption="Virtual TI-92";
    else if (calc==94)
        Caption="Virtual TI-92+";
    Application->Title=Caption;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::LCDonlyblackwhiteBMP1Click(TObject *Sender)
{
    PauseStart();
    TSaveDialog *dlog=new TSaveDialog(this);
    dlog->DefaultExt="bmp";
    dlog->FileName="screen.bmp";
    dlog->Options.Clear();
    dlog->Options << ofOverwritePrompt << ofPathMustExist
        << ofNoChangeDir;
    dlog->Filter="Bitmap files (*.bmp)|*.bmp";
    dlog->FilterIndex=1;
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
        Graphics::TBitmap *bmp=GetLCD_BW();
        TFileStream *f=new TFileStream(dlog->FileName,fmCreate);
        bmp->SaveToStream(f);
        delete f;
        delete bmp;
    }
    delete dlog;
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::LCDonlytruecolorsBMP1Click(TObject *Sender)
{
    PauseStart();
    TSaveDialog *dlog=new TSaveDialog(this);
    dlog->DefaultExt="bmp";
    dlog->FileName="screen.bmp";
    dlog->Options.Clear();
    dlog->Options << ofOverwritePrompt << ofPathMustExist
        << ofNoChangeDir;
    dlog->Filter="Bitmap files (*.bmp)|*.bmp";
    dlog->FilterIndex=1;
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
        Graphics::TBitmap *bmp=GetLCD_True();
        TFileStream *f=new TFileStream(dlog->FileName,fmCreate);
        bmp->SaveToStream(f);
        delete f;
        delete bmp;
    }
    delete dlog;
    PauseEnd();
}
//---------------------------------------------------------------------------


void __fastcall TEmuWnd::LCDonlyblackwhiteClipboard1Click(
      TObject *Sender)
{
    PauseStart();
    Graphics::TBitmap *bmp=GetLCD_BW();
    int DataHandle;
    HPALETTE APalette;
    unsigned short format;
    bmp->SaveToClipboardFormat(format,DataHandle,APalette);
    Clipboard()->SetAsHandle(format,DataHandle);
    delete bmp;
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::LCDonlytruecolorsClipboard1Click(
      TObject *Sender)
{
    PauseStart();
    Graphics::TBitmap *bmp=GetLCD_True();
    int DataHandle;
    HPALETTE APalette;
    unsigned short format;
    bmp->SaveToClipboardFormat(format,DataHandle,APalette);
    Clipboard()->SetAsHandle(format,DataHandle);
    delete bmp;
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::CalculatorimageBMP1Click(TObject *Sender)
{
    PauseStart();
    TSaveDialog *dlog=new TSaveDialog(this);
    dlog->DefaultExt="bmp";
    dlog->FileName="screen.bmp";
    dlog->Options.Clear();
    dlog->Options << ofOverwritePrompt << ofPathMustExist
        << ofNoChangeDir;
    dlog->Filter="Bitmap files (*.bmp)|*.bmp";
    dlog->FilterIndex=1;
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
        Graphics::TBitmap *bmp=GetLCD_Calc(calcImage);
        TFileStream *f=new TFileStream(dlog->FileName,fmCreate);
        bmp->SaveToStream(f);
        delete f;
        delete bmp;
    }
    delete dlog;
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::CalculatorimageClipboard1Click(TObject *Sender)
{
    PauseStart();
    Graphics::TBitmap *bmp=GetLCD_Calc(calcImage);
    int DataHandle;
    HPALETTE APalette;
    unsigned short format;
    bmp->SaveToClipboardFormat(format,DataHandle,APalette);
    Clipboard()->SetAsHandle(format,DataHandle);
    delete bmp;
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ShowLCDonly1Click(TObject *Sender)
{
//    ShowLCDonly1->Checked=!ShowLCDonly1->Checked;
//    lcdOnly=ShowLCDonly1->Checked;
    Make();
}
//---------------------------------------------------------------------------
/*void __fastcall TEmuWnd::OnMessage(tagMSG &Msg,bool &Handled)
{
    if (Msg.message==WM_HELLO)
    {
        MessageBox(NULL,"Hey!","",MB_OK);
        otherWnd=(HWND)Msg.lParam;
        VirtualLink1->Enabled=true;
        Handled=true;
    }
    else if (Msg.message==WM_GOODBYE)
    {
        otherWnd=NULL;
        VirtualLink1->Enabled=false;
        Handled=true;
    }
    else if (Msg.message==WM_ENABLE_LINK)
    {
        emuLink=-1;
        VirtualLink1->Checked=true;
        Handled=true;
    }
    else if (Msg.message==WM_DISABLE_LINK)
    {
        emuLink=0;
        VirtualLink1->Checked=false;
        Handled=true;
    }
}*/

void __fastcall TEmuWnd::VirtualLink1Click(TObject *Sender)
{
    if (emuLink==-1)
    {
        otherInDebug=0;
        if (otherWnd)
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=0;
        VirtualLink1->Checked=false;
        None2->Checked=true;
    }
    else if (otherWnd)
    {
        SendMessage(otherWnd,WM_ENABLE_LINK,0,0);
        emuLink=-1;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=true;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------
void __fastcall TEmuWnd::WMHello(TMessage &Msg)
{
    otherWnd=(HWND)Msg.LParam;
    VirtualLink1->Enabled=true;
    ATOM a=GlobalAddAtom(vLinkFileName);
    SendMessage(otherWnd,WM_SEND_BUFFER,0,(LPARAM)a);
    GlobalDeleteAtom(a);
}

void __fastcall TEmuWnd::WMGoodbye(TMessage &Msg)
{
    otherInDebug=0;
    if (emuLink==-1)
    {
        emuLink=0;
        VirtualLink1->Checked=false;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        None2->Checked=true;
    }
    otherWnd=NULL;
    VirtualLink1->Enabled=false;
}

void __fastcall TEmuWnd::WMEnableLink(TMessage &Msg)
{
    emuLink=-1;
    VirtualLink1->Checked=true;
    COM1->Checked=false;
    COM2->Checked=false;
    COM3->Checked=false;
    COM4->Checked=false;
    SerialLinkonCOM11->Checked=false;
    SerialLinkonCOM21->Checked=false;
    SerialLinkonCOM31->Checked=false;
    SerialLinkonCOM41->Checked=false;
    ParallelLinkonLPT11->Checked=false;
    ParallelLinkonLPT21->Checked=false;
    None2->Checked=false;
}

void __fastcall TEmuWnd::WMDisableLink(TMessage &Msg)
{
    otherInDebug=0;
    emuLink=0;
    VirtualLink1->Checked=false;
    COM1->Checked=false;
    COM2->Checked=false;
    COM3->Checked=false;
    COM4->Checked=false;
    SerialLinkonCOM11->Checked=false;
    SerialLinkonCOM21->Checked=false;
    SerialLinkonCOM31->Checked=false;
    SerialLinkonCOM41->Checked=false;
    ParallelLinkonLPT11->Checked=false;
    ParallelLinkonLPT21->Checked=false;
    None2->Checked=true;
}

void __fastcall TEmuWnd::WMSendBuffer(TMessage &Msg)
{
    char name[32];
    GlobalGetAtomName((ATOM)Msg.LParam,name,32);
    HANDLE h=OpenFileMapping(FILE_MAP_ALL_ACCESS,false,name);
    vRecvBuf=(LinkBuffer*)MapViewOfFile(h,
        FILE_MAP_ALL_ACCESS,0,0,sizeof(LinkBuffer));
}

void __fastcall TEmuWnd::WMEnterDebug(TMessage &Msg)
{
    otherInDebug=1;
}

void __fastcall TEmuWnd::WMExitDebug(TMessage &Msg)
{
    otherInDebug=0;
}

void __fastcall TEmuWnd::DoLink()
{
    if (!Visible)
        return;
    if (emuLink!=openLink)
    {
        if (openLink) clCloseCom();
        if (emuLink>0)
        {
            clInitCom(emuLink);
            openLink=emuLink;
        }
    }
    if (realKeypad)
    {
        int c;
        updateLink=0;
        if (clGetCom(c))
        {
            int startTime=GetTickCount();
            int n=c;
            if (n>10) return;
            while (!clGetCom(c))
            {
                int time=GetTickCount()-startTime;
                if (time<0) return;
                if (time>1000) return;
            }
            if ((c&2)&&(!hw->onheld))
                hw->OnUp();
            else if ((!(c&2))&&(hw->onheld))
                hw->OnDown();
            for (int i=0;i<n;i++)
            {
                while (!clGetCom(c))
                {
                    int time=GetTickCount()-startTime;
                    if (time<0) return;
                    if (time>1000) return;
                }
                hw->kbmatrix[i]=c;
            }
        }
    }
    else if (openLink)
    {
        int c;
//        if (hw->SendReady())
        if (PutSendReady())
        {
            while (clGetCom(c))
            {
                PutSendBuf(c);
                if (!PutSendReady())
                    break;
            }
//                hw->SendBuffer(c);
        }
//        if (hw->CheckBuffer())
        while (CheckRecvBuf())
        {
            int c;
//            hw->GetBuffer(c);
            c=GetRecvBuf();
            clSendCom(c);
            updateLink=0;
        }
    }
    else if (emuLink==-1)
    {
//        if (hw->CheckBuffer())
        while (CheckRecvBuf())
        {
            if (PutVSendReady())
//            if (SendMessage(otherWnd,WM_SEND_READY,0,0))
            {
                int c;
                c=GetRecvBuf();
//                hw->GetBuffer(c);
                PutVSendBuf(c);
//                SendMessage(otherWnd,WM_SEND,c,0);
                updateLink=0;
            }
            else
                break;
        }
        if (PutSendReady())
        {
            while (CheckVRecvBuf())
            {
                PutSendBuf(GetVRecvBuf());
                if (!PutSendReady())
                    break;
            }
        }
    }
    else
        updateLink=0;
}

void __fastcall TEmuWnd::Normal1xview1Click(TObject *Sender)
{
    PauseStart();
    if (Fullscreenview1->Checked)
    {
        Fullscreenview1->Checked=false;
        Top=5; Left=100;
        Enterdebugmode1->Enabled=true;
        Currentskin1->Enabled=true;
        Setprogramentrybreakpoint1->Enabled=true;
        Showpercentageofactualspeed1->Enabled=true;
        Stayontop1->Enabled=true;
        FormStyle=(Stayontop1->Checked)?fsStayOnTop:fsNormal;
    }
    Normal1xview1->Checked=true;
    Large2xcalculatorimage1->Checked=false;
    scaleFact=1;
    Make();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Usecalculatorkeypad1Click(TObject *Sender)
{
//    Usecalculatorkeypad1->Checked=!Usecalculatorkeypad1->Checked;
//    realKeypad=Usecalculatorkeypad1->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Transferindividually1Click(TObject *Sender)
{
    transferAsGroup=0;
    Transferindividually1->Checked=true;
    Transferasgroup1->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Transferasgroup1Click(TObject *Sender)
{
    transferAsGroup=1;
    Transferindividually1->Checked=false;
    Transferasgroup1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::SerialLinkonCOM11Click(TObject *Sender)
{
    if (emuLink==5)
    {
        emuLink=0;
        SerialLinkonCOM11->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=5;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=true;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::SerialLinkonCOM21Click(TObject *Sender)
{
    if (emuLink==6)
    {
        emuLink=0;
        SerialLinkonCOM21->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=6;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=true;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::SerialLinkonCOM31Click(TObject *Sender)
{
    if (emuLink==7)
    {
        emuLink=0;
        SerialLinkonCOM31->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=7;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=true;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::SerialLinkonCOM41Click(TObject *Sender)
{
    if (emuLink==8)
    {
        emuLink=0;
        SerialLinkonCOM41->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=8;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=true;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::OnKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    TMessage msg;
    msg.WParam=Key;
    msg.LParam=0;
    FormKeyDown(msg);
    Key=0;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::OnKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    TMessage msg;
    msg.WParam=Key;
    msg.LParam=0;
    FormKeyUp(msg);
    Key=0;
}
//---------------------------------------------------------------------------
void __fastcall TEmuWnd::OnSkinSelect(TObject *Sender)
{
    int indx=((TMenuItem*)Sender)->Parent->
        IndexOf((TMenuItem*)Sender);
    for (int i=0;i<skinCount;i++)
    {
        if ((calc==skin[i].calc)||((calc>=92)&&
            (skin[i].calc>=92)))
        {
            indx--;
            if (!indx)
            {
                PauseStart();
                LoadSkin(i);
                PauseEnd();
                AboutDlg->UpdateText();
                None1->Checked=false;
                Make();
                break;
            }
        }
    }
}

void __fastcall TEmuWnd::None1Click(TObject *Sender)
{
    CloseSkin();
    None1->Checked=true;
    AboutDlg->UpdateText();
    Make();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ImagePaint(TObject *Sender)
{
    if (!skinImageSm) return;
    if (!skinImageLg) return;
    TRect r;
    r.Left=0; r.Right=Image->Width;
    r.Top=0; r.Bottom=Image->Height;
    if (Image->Width>skinImageSm->Width)
        Image->Canvas->StretchDraw(r,skinImageLg);
    else
        Image->Canvas->StretchDraw(r,skinImageSm);
}
//---------------------------------------------------------------------------
void __fastcall TEmuWnd::WMSizing(TMessage &Msg)
{
    RECT *r=(LPRECT)Msg.LParam;
    if (!enforceRatio)
    {
        Msg.Result=false;
        return;
    }
    Normal1xview1->Checked=false;
    Large2xcalculatorimage1->Checked=false;
    int cliW=r->right-r->left-(Width-ClientWidth);
    int cliH=r->bottom-r->top-(Height-ClientHeight);
    int imgW=(skinImageLg)?skinImageLg->Width:(lcdWidth<<1);
    int imgH=(skinImageLg)?skinImageLg->Height:(lcdHeight<<1);
    int newW=imgW*cliH/imgH;
    int newH=imgH*cliW/imgW;
    newW+=(Width-ClientWidth);
    newH+=(Height-ClientHeight);
    int dx=abs(newW-(r->right-r->left));
    int dy=abs(newH-(r->bottom-r->top));
    if (((dx>=dy)||(Msg.WParam==WMSZ_LEFT)||(Msg.WParam==WMSZ_RIGHT))&&
        ((Msg.WParam!=WMSZ_TOP)&&(Msg.WParam!=WMSZ_BOTTOM)))
    {
        if (newH<GetSystemMetrics(SM_CYMIN))
            GetWindowRect(Handle,r);
        else
        {
            if ((Msg.WParam==WMSZ_TOP)||
                (Msg.WParam==WMSZ_TOPLEFT)||
                (Msg.WParam==WMSZ_TOPRIGHT))
                r->top=r->bottom-newH;
            else
                r->bottom=r->top+newH;
        }
    }
    else
    {
        if (newW<GetSystemMetrics(SM_CXMIN))
            GetWindowRect(Handle,r);
        else
        {
            if ((Msg.WParam==WMSZ_BOTTOMLEFT)||
                (Msg.WParam==WMSZ_LEFT)||
                (Msg.WParam==WMSZ_TOPLEFT))
                r->left=r->right-newW;
            else
                r->right=r->left+newW;
        }
    }
    Image->Width=r->right-r->left-(Width-ClientWidth);
    Image->Height=r->bottom-r->top-(Height-ClientHeight);
    LCD->Left=skinLcd.left*Image->Width/imgW;
    LCD->Width=(skinLcd.right-skinLcd.left)*Image->Width/imgW;
    LCD->Top=skinLcd.top*Image->Height/imgH;
    LCD->Height=(skinLcd.bottom-skinLcd.top)*Image->Height/imgH;
    if ((LCD->Width>lcdWidth)&&(scaleFact==1))
    {
        PauseStart();
        scaleFact=2;
        bmp->Width=lcdWidth*scaleFact;
        bmp->Height=lcdHeight*scaleFact;
        bmp->PixelFormat=pf32bit;
        for (int i=0;i<(lcdHeight*scaleFact);i++)
            ScreenLine[i]=(char *)bmp->ScanLine[i];
        RefreshLCDNow();
        PauseEnd();
    }
    else if ((LCD->Width<=lcdWidth)&&(scaleFact==2))
    {
        PauseStart();
        scaleFact=1;
        bmp->Width=lcdWidth*scaleFact;
        bmp->Height=lcdHeight*scaleFact;
        bmp->PixelFormat=pf32bit;
        for (int i=0;i<(lcdHeight*scaleFact);i++)
            ScreenLine[i]=(char *)bmp->ScanLine[i];
        RefreshLCDNow();
        PauseEnd();
    }
    Msg.Result=true;
}

void __fastcall TEmuWnd::Fullscreenview1Click(TObject *Sender)
{
    PauseStart();
    if (Fullscreenview1->Checked)
    {
        Fullscreenview1->Checked=false;
        Top=5; Left=100;
        Enterdebugmode1->Enabled=true;
        Currentskin1->Enabled=true;
        Setprogramentrybreakpoint1->Enabled=true;
        Showpercentageofactualspeed1->Enabled=true;
        Stayontop1->Enabled=true;
        FormStyle=(Stayontop1->Checked)?fsStayOnTop:fsNormal;
        Make();
        return;
    }
    if (currentSkin!=-1)
    {
        skin[currentSkin].defW=Image->Width;
        skin[currentSkin].defH=Image->Height;
    }
    Fullscreenview1->Checked=true;
    Normal1xview1->Checked=false;
    Large2xcalculatorimage1->Checked=false;
    Enterdebugmode1->Enabled=false;
    Currentskin1->Enabled=false;
    Setprogramentrybreakpoint1->Enabled=false;
    Showpercentageofactualspeed1->Enabled=false;
    Stayontop1->Enabled=false;
    DebugWnd->Clearallbreakpoints1Click(Sender);
    Make();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Exitemulator1Click(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Stayontop1Click(TObject *Sender)
{
    Stayontop1->Checked=!Stayontop1->Checked;
    FormStyle=(Stayontop1->Checked)?fsStayOnTop:fsNormal;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    TPoint p;
    if (Button==mbRight)
    {
        PauseStart();
        p.x=X; p.y=Y; p=Image->ClientToScreen(p);
        PopupMenu1->Popup(p.x,p.y);
        PauseEnd();
    }
    else if (Button==mbLeft)
    {
        if (keyHeld!=-1) return;
        int imgW=(skinImageLg)?skinImageLg->Width:(lcdWidth<<1);
        int imgH=(skinImageLg)?skinImageLg->Height:(lcdHeight<<1);
        int key=GetKeyAt(X*imgW/Image->Width,
            Y*imgH/Image->Height);
        if (key!=-1)
        {
            kdown(key);
            keyHeld=key;
        }
        else
        {
            mouseClicked=1;
            mouseX=X;
            mouseY=Y;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ParallelLinkonLPT11Click(TObject *Sender)
{
    if (emuLink==9)
    {
        emuLink=0;
        ParallelLinkonLPT11->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=9;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=true;
        ParallelLinkonLPT21->Checked=false;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ParallelLinkonLPT21Click(TObject *Sender)
{
    if (emuLink==10)
    {
        emuLink=0;
        ParallelLinkonLPT21->Checked=false;
        None2->Checked=true;
    }
    else
    {
        otherInDebug=0;
        if ((emuLink==-1)&&(otherWnd))
            SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
        emuLink=10;
        COM1->Checked=false;
        COM2->Checked=false;
        COM3->Checked=false;
        COM4->Checked=false;
        SerialLinkonCOM11->Checked=false;
        SerialLinkonCOM21->Checked=false;
        SerialLinkonCOM31->Checked=false;
        SerialLinkonCOM41->Checked=false;
        ParallelLinkonLPT11->Checked=false;
        ParallelLinkonLPT21->Checked=true;
        VirtualLink1->Checked=false;
        None2->Checked=false;
    }
}
//---------------------------------------------------------------------------
void __fastcall TEmuWnd::WMDropFiles(TMessage &Msg)
{
    HANDLE hDrop=(HANDLE)Msg.WParam;
    PauseStart();
    int fileCount=DragQueryFile(hDrop,-1,NULL,0);
    Screen->Cursor=crHourGlass;
    char name[256];
    for (int i=0;i<fileCount;)
    {
        DragQueryFile(hDrop,i,name,255);
        if (hw->SendFile(name,i<(fileCount-1)))
        {
            i++;
            continue;
        }
        Screen->Cursor=crDefault;
        if (MessageBox(Handle,"Transmission error","Error",
            MB_RETRYCANCEL|MB_ICONEXCLAMATION)==IDCANCEL)
            break;
        Screen->Cursor=crHourGlass;
    }
    DragFinish(hDrop);
    Screen->Cursor=crDefault;
    PauseEnd();
}


void __fastcall TEmuWnd::Exitwithoutsavingstate2Click(TObject *Sender)
{
    saveStateOnExit=0;
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::N4FPSLCDUpdates1Click(TObject *Sender)
{
    lcdUpdateFreq=3;
    N4FPSLCDUpdates1->Checked=true;
    N10HzLCDUpdates1->Checked=false;
    N30HzLCDUpdates1->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::N10HzLCDUpdates1Click(TObject *Sender)
{
    lcdUpdateFreq=2;
    N4FPSLCDUpdates1->Checked=false;
    N10HzLCDUpdates1->Checked=true;
    N30HzLCDUpdates1->Checked=false;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::N30HzLCDUpdates1Click(TObject *Sender)
{
    lcdUpdateFreq=1;
    N4FPSLCDUpdates1->Checked=false;
    N10HzLCDUpdates1->Checked=false;
    N30HzLCDUpdates1->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::None2Click(TObject *Sender)
{
    otherInDebug=0;
    if ((emuLink==-1)&&(otherWnd))
        SendMessage(otherWnd,WM_DISABLE_LINK,0,0);
    emuLink=0;
    COM1->Checked=false;
    COM2->Checked=false;
    COM3->Checked=false;
    COM4->Checked=false;
    SerialLinkonCOM11->Checked=false;
    SerialLinkonCOM21->Checked=false;
    SerialLinkonCOM31->Checked=false;
    SerialLinkonCOM41->Checked=false;
    ParallelLinkonLPT11->Checked=false;
    ParallelLinkonLPT21->Checked=false;
    VirtualLink1->Checked=false;
    None2->Checked=true;
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::Enablesound1Click(TObject *Sender)
{
    Enablesound1->Checked=!Enablesound1->Checked;
    PauseStart();
    if (Enablesound1->Checked)
        EnableSound();
    else
        DisableSound();
    PauseEnd();
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ImageMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    mouseClicked=0;
    if (Button==mbLeft)
    {
        if (keyHeld==-1) return;
        kup(keyHeld);
        keyHeld=-1;
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::ImageMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
    if (mouseClicked&&skinFloating)
    {
        if (sizeType==SIZE_NONE)
        {
            Left+=X-mouseX;
            Top+=Y-mouseY;
        }
        else if (sizeType==SIZE_LEFT)
        {
            TMessage msg;
            RECT r;
            r.left=Left+(X-mouseX);
            r.right=Left+Width;
            r.top=Top;
            r.bottom=Top+Height;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_LEFT;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            EmuWnd->Update();
        }
        else if (sizeType==SIZE_RIGHT)
        {
            TMessage msg;
            RECT r;
            r.left=Left;
            r.right=Left+Width+(X-mouseX);
            mouseX=X;
            r.top=Top;
            r.bottom=Top+Height;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_RIGHT;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            EmuWnd->Update();
        }
        else if (sizeType==SIZE_TOP)
        {
            TMessage msg;
            RECT r;
            r.left=Left;
            r.right=Left+Width;
            r.top=Top+(Y-mouseY);
            r.bottom=Top+Height;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_TOP;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            EmuWnd->Update();
        }
        else if (sizeType==SIZE_BOTTOM)
        {
            TMessage msg;
            RECT r;
            r.left=Left;
            r.right=Left+Width;
            r.top=Top;
            r.bottom=Top+Height+(Y-mouseY);
            mouseY=Y;
            msg.LParam=(DWORD)&r;
            msg.WParam=WMSZ_BOTTOM;
            WMSizing(msg);
            Left=r.left; Width=r.right-r.left;
            Top=r.top; Height=r.bottom-r.top;
            SetWindowRgn(Handle,CreateFloatRgn(),true);
            EmuWnd->Update();
        }
    }
    else if (skinFloating)
    {
        int x=(X*skinImageLg->Width)/ClientWidth;
        int y=(Y*skinImageLg->Height)/ClientHeight;
        int w=skinImageLg->Width;
        int h=skinImageLg->Height;
        int border=5*skinImageLg->Width/ClientWidth;
        int leftX,rightX,topY,bottomY;
        unsigned color=SkinColor(0,0);
        for (leftX=0;leftX<skinImageLg->Width;leftX++)
        {
            if (!IsCloseEnough(SkinColor(leftX,y),color))
                break;
        }
        for (rightX=skinImageLg->Width-1;rightX>0;rightX--)
        {
            if (!IsCloseEnough(SkinColor(rightX,y),color))
                break;
        }
        for (topY=0;topY<skinImageLg->Height;topY++)
        {
            if (!IsCloseEnough(SkinColor(x,topY),color))
                break;
        }
        for (bottomY=skinImageLg->Height-1;bottomY>0;bottomY--)
        {
            if (!IsCloseEnough(SkinColor(x,bottomY),color))
                break;
        }
        if ((abs(x-leftX)<border)||(abs(x-rightX)<border)||(abs(y-topY)<border)||
            (abs(y-bottomY)<border))
        {
            if ((x<(w/2))&&(y>(x*h/w))&&(y<(h-x*h/w)))
            {
                Screen->Cursor=crSizeWE;
                sizeType=SIZE_LEFT;
            }
            else if ((x>=(w/2))&&(y>((w-x)*h/w))&&(y<(h-(w-x)*h/w)))
            {
                Screen->Cursor=crSizeWE;
                sizeType=SIZE_RIGHT;
            }
            else if ((y<(h/2))&&(x>(y*w/h))&&(x<(w-y*w/h)))
            {
                Screen->Cursor=crSizeNS;
                sizeType=SIZE_TOP;
            }
            else
            {
                Screen->Cursor=crSizeNS;
                sizeType=SIZE_BOTTOM;
            }
        }
        else
        {
            Screen->Cursor=crArrow;
            sizeType=SIZE_NONE;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::LCDMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
    Screen->Cursor=crArrow;    
}
//---------------------------------------------------------------------------

void __fastcall TEmuWnd::FormResize(TObject *Sender)
{
    if (!skinFloating)
        SetWindowRgn(Handle,CreateRectRgn(0,0,Width,Height),true);
}
//---------------------------------------------------------------------------

