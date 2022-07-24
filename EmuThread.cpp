//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "EmuThread.h"
#include "CalcHW.h"
#include "rom.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall EmuThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
HANDLE emuEvent;

__fastcall EmuThread::EmuThread(bool CreateSuspended)
    : TThread(CreateSuspended)
{
    emuEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
}
//---------------------------------------------------------------------------
volatile extern int debug,debugStartPC,debugStepOver;
volatile int exitEmu=0;
volatile int emuRunning=0;
volatile extern bool restrict;
volatile extern bool pause,pauseReady,pauseExit;
volatile extern int percentChange,percent,showSpeed;

void __fastcall EmuThread::Execute()
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    unsigned long perfFreq=li.u.LowPart/50;
    //---- Place thread code here ----
    while (!exitEmu)
    {
        if (pause)
        {
            pauseReady=true;
            while (pause&&(!exitEmu))
                WaitForSingleObject(emuEvent,500);
            pauseExit=true;
        }
        if (((!debug)||(run))&&((!otherInDebug)||(debugStepOver)))
        {
            emuRunning=1;
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
            if (restrict&&(!soundEnable))
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
                        Sleep(10);
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
                    percentChange++;
                    percent=(ticks*100)/(9875*(curTime-lastTime)/1000);
                    lastTime=curTime;
                    ticks=0;
                }
            }
//            DoLink();
/*            if (Active)
            {
                if (((GetKeyState(VK_TAB)&0x8000)==0x8000)&&(genTab))
                {
                    TMessage msg;
                    msg.WParam=VK_TAB;
                    msg.LParam=0;
                    FormKeyDown(msg);
                    genTab=0;
                }
            }*/
        }
        else
        {
            emuRunning=0;
            Sleep(50);
        }
        if (emuLink==-1)
            Sleep(3);
    }
    exitEmu=false;
    CloseHandle(emuEvent);
}
//---------------------------------------------------------------------------
