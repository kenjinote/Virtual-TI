//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Dump.h"
#include "DumpWnd.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall DumpThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
__fastcall DumpThread::DumpThread(int port,bool CreateSuspended)
    : TThread(CreateSuspended)
{
    comPort=port;
}

int dumpProgress;
void __fastcall DumpThread::UpdateProgress()
{
    DumpWindow->ProgressBar1->Position=dumpProgress;
}

char dumpLabel[64];
void __fastcall DumpThread::UpdateLabel()
{
    DumpWindow->Label1->Caption=dumpLabel;
}

void __fastcall DumpThread::CloseWindow()
{
    DumpWindow->Hide();
}
//---------------------------------------------------------------------------
extern void clInitCom(int n,int timeout=0);
extern void clCloseCom();
extern void clSendCom(int ch);
extern int clGetCom(int &ch);
extern int clGetError();
extern void clResetError();
extern int dumpCancel;
extern char shellPath[256];
int dumpComplete,dumpSuccess;
void __fastcall DumpThread::Execute()
{
    //---- Place thread code here ----
    clInitCom(comPort,1000);
    int first=1;
    dumpProgress=0;
    char *buf=new char[0x200000];
    int i;
    for (i=0;i<0x200000;i+=1024,dumpProgress++)
    {
        Synchronize(UpdateProgress);
        for (;;)
        {
            int j,ch;
            unsigned short csum,calcCSum;
            for (j=0,csum=0;j<1024;)
            {
                int ch;
                clGetCom(ch);
                if (clGetError())
                {
                    clResetError();
                    break;
                }
                buf[i+j]=ch;
                csum+=(unsigned short)((unsigned char)ch);
                j++;
                first=0;
            }
            if (j<1024)
            {
                if (dumpCancel)
                    break;
                if (!first)
                    clSendCom(0);
                continue;
            }
            clGetCom(ch); calcCSum=(ch&0xff)<<8;
            clGetCom(ch); calcCSum|=ch&0xff;
            if (dumpCancel)
                break;
            if (clGetError())
            {
                clResetError();
                clSendCom(0);
                continue;
            }
            if (csum!=calcCSum)
            {
                clSendCom(0);
                continue;
            }
            clSendCom(0xda);
            if (i==0)
            {
                if (buf[5]==0x20)
                {
                    strcpy(dumpLabel,"Receiving ROM from TI-89...");
                    Synchronize(UpdateLabel);
                }
                else
                {
                    strcpy(dumpLabel,"Receiving ROM from TI-92+...");
                    Synchronize(UpdateLabel);
                }
            }
            break;
        }
        if (dumpCancel)
        {
            clSendCom(0xcc);
            break;
        }
    }
    clCloseCom();
    if (i>=0x200000)
    {
        char fn[256];
        for (int n=1;;n++)
        {
            sprintf(fn,"ti%s_%d.rom",(buf[5]==0x20)?"89":"92p",n);
            FILE *fp=fopen(fn,"rb");
            if (!fp) break;
            fclose(fp);
        }
        FILE *fp=fopen(fn,"wb");
        fwrite(buf,1024,2048,fp);
        fclose(fp);
        Synchronize(CloseWindow);
        dumpSuccess=1;
    }
    dumpComplete=1;
}
//---------------------------------------------------------------------------
