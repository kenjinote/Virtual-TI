#define VERSION_STRING "2.5b5"
//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\registry.hpp>
#include <stdio.h>
#pragma hdrstop
USEFORM("Emu.cpp", EmuWnd);
USEFORM("Debug.cpp", DebugWnd);
USEUNIT("m68kdasm.cpp");
USEFORM("gotodialog.cpp", GoToDlg);
USEFORM("About.cpp", AboutDlg);
USEFORM("EmuMode.cpp", EmuModeDlg);
USEFORM("Value.cpp", NewValueDlg);
USEFORM("Entry.cpp", ProgramEntryDlg);
USEFORM("Data.cpp", DataBreakDlg);
USEFORM("AddData.cpp", AddDataBreakDlg);
USEFORM("Search.cpp", SearchDlg);
USEFORM("AddLog.cpp", AddLogDlg);
USEFORM("Log.cpp", LogDlg);
USEFORM("EditLog.cpp", EditLogDlg);
USEUNIT("asmintf.cpp");
USEUNIT("opcodee.cpp");
USEUNIT("opcode1.cpp");
USEUNIT("opcode2.cpp");
USEUNIT("opcode3.cpp");
USEUNIT("opcode4.cpp");
USEUNIT("opcode5.cpp");
USEUNIT("opcode6.cpp");
USEUNIT("opcode7.cpp");
USEUNIT("opcode8.cpp");
USEUNIT("opcode9.cpp");
USEUNIT("opcodeb.cpp");
USEUNIT("opcodec.cpp");
USEUNIT("opcoded.cpp");
USEUNIT("opcode0.cpp");
USEUNIT("cpufunc.cpp");
USEUNIT("Z80.cpp");
USEUNIT("rom.cpp");
USEUNIT("ti89.cpp");
USEUNIT("ti86.cpp");
USEUNIT("ti85.cpp");
USEUNIT("ti82.cpp");
USEUNIT("ti83.cpp");
USEUNIT("ti92.cpp");
USEUNIT("ti92plus.cpp");
USEASM("asmmem.asm");
USERES("Vti.res");
USEUNIT("ti83plus.cpp");
USEUNIT("ti73.cpp");
USEUNIT("jpenquan.cpp");
USEUNIT("jfif.cpp");
USEUNIT("jpdecomp.cpp");
USEUNIT("jpdedu.cpp");
USEUNIT("JPDEHUFF.cpp");
USEUNIT("jpdequan.cpp");
USEUNIT("jpeg.cpp");
USEUNIT("jpegdeco.cpp");
USEUNIT("jpegenco.cpp");
USEUNIT("Jpegfmt.cpp");
USEUNIT("jpencomp.cpp");
USEUNIT("jpendu.cpp");
USEUNIT("jpenhuff.cpp");
USEUNIT("bitimage.cpp");
USEUNIT("EmuThread.cpp");
USEFORM("SetupWizard.cpp", SetupWizardWnd);
USEFORM("ROMWizard.cpp", ROMWizardWnd);
USEFORM("Trap.cpp", TrapDlg);
USEFORM("WizardBmp.cpp", WizardBitmap);
USEFORM("VarWnd.cpp", VariableWnd);
USEFORM("LblWnd.cpp", LabelWnd);
USEUNIT("Sound.cpp");
//---------------------------------------------------------------------------
#include "Start.h"
#include "Debug.h"
#define NOTYPEREDEF
#include "cpudefs.h"
#include "linktype.h"
#include "dumpwnd.h"
#include "dump.h"
#include "calchw.h"
#include "About.h"
#include "emu.h"
#include "rom.h"
#include "SetupWizard.h"
#include "WizardBmp.h"

CalcHW* hw;
//---------------------------------------------------------------------------
extern void LoadState();
extern int calc,plusMod,debug,run,debugStartPC;
extern int romImageCount;
extern int lcdOnly;

int DumpROM();
extern int SendFile(int port);

extern bool restrict;

//extern int keymap[];
//extern char* alphakeystr[];
//---------------------------------------------------------------------------
HINSTANCE hInst;
HWND otherWnd;
HANDLE hVLinkFileMap,hMutex;
char vLinkFileName[32];

static Graphics::TBitmap *vti,*vti_a,*bkg,*dest;
static int bkgStored=0;
static int drawImage=0;

LRESULT CALLBACK VTIStartWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC dc;
    switch (msg)
    {
        case WM_PAINT:
            if (!drawImage) return 0;
            dc=BeginPaint(hWnd,&ps);
            TCanvas *canvas=new TCanvas;
            canvas->Handle=dc;
            if (!bkgStored)
            {
                BitBlt(bkg->Canvas->Handle,0,0,vti->Width,vti->Height,
                    dc,0,0,SRCCOPY);
                bkgStored=1;
            }
            for (int y=0;y<vti->Height;y++)
            {
                unsigned char *bgPtr=(unsigned char*)bkg->ScanLine[y];
                unsigned char *srcPtr=(unsigned char*)vti->ScanLine[y];
                unsigned char *srcAPtr=(unsigned char*)vti_a->ScanLine[y];
                unsigned char *destPtr=(unsigned char*)dest->ScanLine[y];
                for (int x=0;x<vti->Width;x++)
                {
                    int a=*(srcAPtr++);
                    *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                    *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                    *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                }
            }
            canvas->Draw(0,0,dest);
            delete canvas;
            EndPaint(hWnd,&ps);
            return 0;
    }
    return DefWindowProc(hWnd,msg,wParam,lParam);
}

void FadeIn(HWND hWnd)
{
    HDC dc=GetDC(hWnd);
    TCanvas *canvas=new TCanvas;
    canvas->Handle=dc;
    int lastTime=GetTickCount();
    for (int alphaMult=0;alphaMult<=256;)
    {
        if (!bkgStored)
        {
            BitBlt(bkg->Canvas->Handle,0,0,vti->Width,vti->Height,
                dc,0,0,SRCCOPY);
            bkgStored=1;
        }
        for (int y=0;y<vti->Height;y++)
        {
            unsigned char *bgPtr=(unsigned char*)bkg->ScanLine[y];
            unsigned char *srcPtr=(unsigned char*)vti->ScanLine[y];
            unsigned char *srcAPtr=(unsigned char*)vti_a->ScanLine[y];
            unsigned char *destPtr=(unsigned char*)dest->ScanLine[y];
            for (int x=0;x<vti->Width;x++)
            {
                int a=*(srcAPtr++);
                a=(a*alphaMult)>>8;
                *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
            }
        }
        canvas->Draw(0,0,dest);
        int curTime=GetTickCount();
        alphaMult+=(curTime-lastTime)/2;
        lastTime=curTime;
    }
    delete canvas;
    ReleaseDC(hWnd,dc);
    drawImage=1;
    InvalidateRect(hWnd,NULL,false);
    UpdateWindow(hWnd);
}

void FadeOut(HWND hWnd)
{
    drawImage=0;
    HDC dc=GetDC(hWnd);
    TCanvas *canvas=new TCanvas;
    canvas->Handle=dc;
    int lastTime=GetTickCount();
    for (int alphaMult=256;alphaMult>=0;)
    {
        if (!bkgStored)
        {
            BitBlt(bkg->Canvas->Handle,0,0,vti->Width,vti->Height,
                dc,0,0,SRCCOPY);
            bkgStored=1;
        }
        for (int y=0;y<vti->Height;y++)
        {
            unsigned char *bgPtr=(unsigned char*)bkg->ScanLine[y];
            unsigned char *srcPtr=(unsigned char*)vti->ScanLine[y];
            unsigned char *srcAPtr=(unsigned char*)vti_a->ScanLine[y];
            unsigned char *destPtr=(unsigned char*)dest->ScanLine[y];
            for (int x=0;x<vti->Width;x++)
            {
                int a=*(srcAPtr++);
                a=(a*alphaMult)>>8;
                *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
                *(destPtr++)=((*(bgPtr++))*(255-a)+(*(srcPtr++))*a)/256;
            }
        }
        canvas->Draw(0,0,dest);
        int curTime=GetTickCount();
        alphaMult-=(curTime-lastTime)/2;
        lastTime=curTime;
    }
    delete canvas;
    ReleaseDC(hWnd,dc);
}

WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR cmdLine, int)
{
    initPath[0]=0;
    getcwd(initPath,256);
    hInst=inst;
    for (int i=0;;i++)
    {
        sprintf(vLinkFileName,"Virtual Link %d",i);
        hVLinkFileMap=CreateFileMapping((HANDLE)-1,NULL,
            PAGE_READWRITE,0,sizeof(LinkBuffer),vLinkFileName);
        if (GetLastError()!=ERROR_ALREADY_EXISTS)
            break;
    }
    vSendBuf=(LinkBuffer*)MapViewOfFile(hVLinkFileMap,
        FILE_MAP_ALL_ACCESS,0,0,sizeof(LinkBuffer));
    hMutex=CreateMutex(NULL,true,"Virtual TI");
    if (GetLastError()==ERROR_ALREADY_EXISTS)
        otherWnd=FindWindow("TEmuWnd",NULL);
    try
    {
        int showAbout=1;
        Application->Initialize();
        Application->Title = "Virtual TI";
        WizardBitmap=new TWizardBitmap(NULL);
        if (!strncmp(cmdLine,"-setup",6))
        {
            SetupWizardWnd=new TSetupWizardWnd(NULL);
            if (SetupWizardWnd->ShowModal()!=1)
                exit(1);
            delete SetupWizardWnd;
            TRegistry *reg=new TRegistry;
            reg->RootKey=HKEY_LOCAL_MACHINE;
            reg->OpenKey("\\Software\\ACZ\\Virtual TI",true);
            reg->WriteString("LastEmuVersion",VERSION_STRING);
        }
        {
            TWizardBitmap *wb=new TWizardBitmap(NULL);
            vti=wb->vti1->Picture->Bitmap;
            vti_a=wb->vti1a->Picture->Bitmap;
            bkg=new Graphics::TBitmap;
            bkg->Width=vti->Width;
            bkg->Height=vti->Height;
            bkg->PixelFormat=pf24bit;
            dest=new Graphics::TBitmap;
            dest->Width=vti->Width;
            dest->Height=vti->Height;
            dest->PixelFormat=pf24bit;
            WNDCLASS wc;
            wc.style=0;
            wc.lpfnWndProc=VTIStartWndProc;
            wc.cbClsExtra=0;
            wc.cbWndExtra=0;
            wc.hInstance=hInst;
            wc.hIcon=NULL;
            wc.hCursor=NULL;
            wc.hbrBackground=NULL;
            wc.lpszMenuName=NULL;
            wc.lpszClassName="VTIStartWnd";
            RegisterClass(&wc);
            HWND hWnd=CreateWindowEx(0,//WS_EX_TRANSPARENT,
                "VTIStartWnd","",WS_POPUP|WS_VISIBLE,(GetSystemMetrics(
                SM_CXSCREEN)/2)-(vti->Width/2),(GetSystemMetrics(SM_CYSCREEN)/
                2)-(vti->Height/2),vti->Width,vti->Height,NULL,NULL,hInst,0);
            ShowWindow(hWnd,SW_SHOW);
            SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
            UpdateWindow(hWnd);
            FadeIn(hWnd);
//            StartWnd=new TStartWnd(NULL);
//            StartWnd->Show();
//            StartWnd->Update();
//            StartWnd->Message->Caption="Loading ROM image...";
//            StartWnd->Message->Update();
            TRegistry *reg=new TRegistry;
            reg->RootKey=HKEY_LOCAL_MACHINE;
            reg->OpenKey("\\Software\\ACZ\\Virtual TI",true);
            if (reg->ValueExists("LastEmuVersion"))
                showAbout=strcmp(reg->ReadString(
                    "LastEmuVersion").c_str(),VERSION_STRING);
            else
                showAbout=1;
            if (showAbout)
                reg->WriteString("LastEmuVersion",VERSION_STRING);
            int rn=AnalyzeROMs(showAbout);
            AnalyzeSkins();
            vti=wb->vti2->Picture->Bitmap;
            vti_a=wb->vti2a->Picture->Bitmap;
            InvalidateRect(hWnd,NULL,false);
            UpdateWindow(hWnd);
            if (reg->ValueExists("LastEmuROM"))
            {
                AnsiString str=reg->ReadString("LastEmuROM");
                int i;
                for (i=0;i<romImageCount;i++)
                {
                    if (!strcmp(str.c_str(),romImage[i].name))
                        rn=i;
                }
            }
            if (reg->ValueExists("RestrictSpeed"))
                restrict=reg->ReadInteger("RestrictSpeed");
            if (reg->ValueExists("LCDOnly"))
                lcdOnly=reg->ReadInteger("LCDOnly");
            if (reg->OpenKey("EmuScaleDefault",false))
            {
                for (int i=0;i<6;i++)
                {
                    char str[4];
                    sprintf(str,"%d",i);
                    if (reg->ValueExists(str))
                        scaleDefault[i]=reg->ReadInteger(str);
                }
                reg->OpenKey("\\Software\\ACZ\\Virtual TI",true);
            }
/*            if (reg->OpenKey("EmuKeyMap",false))
            {
                for (int i=k_q;i<=k_m;i++)
                {
                    int j;
                    for (j=0;keymap[j]!=-1;j+=2)
                    {
                        if (keymap[j+1]==i)
                        {
                            char str[4];
                            sprintf(str,"%s",alphakeystr[i-k_q]);
                            if (reg->ValueExists(str))
                                keymap[j]=reg->ReadInteger(str);
                            break;
                        }
                    }
                }
            }*/
            reg->CloseKey();
            delete reg;
            if ((rn==-1)||(showAbout))
            {
//                StartWnd->Hide();
                SetupWizardWnd=new TSetupWizardWnd(NULL);
                if (SetupWizardWnd->ShowModal()!=1)
                    exit(1);
                delete SetupWizardWnd;
                rn=AnalyzeROMs();
                if (rn==-1)
                    return 1;
            }
            EmuWnd=NULL;
            InitEmu(rn);
            CloseSkin(0);
            reg=new TRegistry;
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
                    LoadSkin(found);
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
//            StartWnd->Hide();
            FadeOut(hWnd);
            DestroyWindow(hWnd);
            delete wb;
            LoadState();
        }
        Application->CreateForm(__classid(TEmuWnd), &EmuWnd);
        Application->CreateForm(__classid(TDebugWnd), &DebugWnd);
        Application->CreateForm(__classid(TGoToDlg), &GoToDlg);
        Application->CreateForm(__classid(TAboutDlg), &AboutDlg);
        Application->CreateForm(__classid(TEmuModeDlg), &EmuModeDlg);
        Application->CreateForm(__classid(TNewValueDlg), &NewValueDlg);
        Application->CreateForm(__classid(TProgramEntryDlg), &ProgramEntryDlg);
        Application->CreateForm(__classid(TDataBreakDlg), &DataBreakDlg);
        Application->CreateForm(__classid(TAddDataBreakDlg), &AddDataBreakDlg);
        Application->CreateForm(__classid(TSearchDlg), &SearchDlg);
        Application->CreateForm(__classid(TAddLogDlg), &AddLogDlg);
        Application->CreateForm(__classid(TLogDlg), &LogDlg);
        Application->CreateForm(__classid(TEditLogDlg), &EditLogDlg);
        Application->CreateForm(__classid(TTrapDlg), &TrapDlg);
        Application->Run();
    }
    catch (Sysutils::Exception &exception)
    {
        Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------

