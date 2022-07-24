//---------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "ROMWizard.h"
#include "rom82.h"
#include "rom83.h"
#include "rom83p.h"
#include "rom85u.h"
#include "rom85z.h"
#include "rom86.h"
#include "rom89.h"
#include "rom92f2.h"
#include "rom.h"
#include "WizardBmp.h"
//#include "Unit1.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TROMWizardWnd *ROMWizardWnd;

#define DUMP_STAGE -1
enum {S82_HEADER,S82_DATA,
      S83A_HEADER,S83A_DATA,
      S83PLUS_HEADER,S83PLUS_DATA,
      S85U_HEADER,S85U_DATA,
      S85Z_HEADER,S85Z_DATA,
      S86_HEADER,S86_DATA,S86_FINISH};
enum {S89_HEADER,S89_DATA,S89_FINISH,
      S92F2_HEADER,S92F2_DATA};

extern void clInitCom(int n,int timeout=0);
extern void clCloseCom();
extern void clSendCom(int ch);
extern int clGetCom(int &ch);
extern int clGetError();
extern void clResetError();
extern void clFlushInput();

static int WaitForAck()
{
    int i;
    clGetCom(i); clGetCom(i); clGetCom(i); clGetCom(i);
    return i;
}
//---------------------------------------------------------------------
__fastcall TROMWizardWnd::TROMWizardWnd(TComponent* AOwner)
	: TForm(AOwner)
{
    LinkTypePanel->Hide();
    FinishedPanel->Hide();
    StartPanel->Show();
    BackButton->Enabled=false;
    NextButton->Enabled=false;
    NextButton->Caption="Next >";
    activePanel=StartPanel;
    Application->OnIdle=OnIdle;
}

#define SEND89KEY(k) \
    clSendCom(9); clSendCom(0x87); \
    clSendCom(k&0xff); clSendCom((k>>8)&0xff); \
    WaitForAck();
//---------------------------------------------------------------------
void __fastcall TROMWizardWnd::OnIdle(TObject *,bool& done)
{
    static int csum,first;
    done=false;
    if (!sending)
        Sleep(77);
    else
    {
        if (stage==DUMP_STAGE)
        {
            int i,c;
            static unsigned long startTime;
            unsigned long curTime,estTime;
            char str[64];
            for (i=pos&1023;i<1024;i++)
            {
                clGetCom(c);
                if (clGetError())
                {
                    clResetError();
                    if (!first)
                        clFlushInput();
                    break;
                }
                buf[pos++]=c;
                csum+=c;
                first=0;
            }
            if (i<1024)
            {
//                sprintf(str,"Block %d failed, data loss",blockStart>>10);
//                Form1->ListBox1->Items->Insert(0,str);
                if (!first)
                {
                    clFlushInput();
                    clSendCom(0);
                }
                csum=0;
                pos=blockStart;
                return;
            }
//            sprintf(str,"Block %d data finished",blockStart>>10);
//            Form1->ListBox1->Items->Insert(0,str);
            clGetCom(c); i=c<<8;
            clGetCom(c); i|=c;
            if (clGetError())
            {
//                sprintf(str,"Block %d failed, checksum not received",blockStart>>10);
//                Form1->ListBox1->Items->Insert(0,str);
                clResetError();
                clFlushInput();
                clSendCom(0);
                csum=0;
                pos=blockStart;
                return;
            }
            if ((csum&0xffff)!=(i&0xffff))
            {
//                sprintf(str,"Block %d failed, checksum error",blockStart>>10);
//                Form1->ListBox1->Items->Insert(0,str);
                clFlushInput();
                clSendCom(0);
                csum=0;
                pos=blockStart;
                return;
            }
            csum=0;
            Sleep(20);
            clSendCom(0xda);
//            sprintf(str,"Block %d success",blockStart>>10);
//            Form1->ListBox1->Items->Insert(0,str);
            Sleep(20);
            if (progress) progress->StepIt();
            if ((blockStart==0)&&(transferText))
                transferText->Caption="ROM transfer in progress.  Please wait...";
            if (blockStart==2048)
                startTime=GetTickCount();
            if (romSize==(1024*2048))
            {
                if (buf[5]==0x02)
                    strcpy(calcName,"89");
                else
                    strcpy(calcName,"92p");
            }
            blockStart+=1024;
            if ((blockStart>(3*1024))&&(timeText))
            {
                if (blockStart==4096)
                    timeText->Show();
                curTime=GetTickCount()-startTime;
                estTime=((romSize>>10)-(blockStart>>10))*(curTime/((blockStart>>10)-3))/60000;
                if (estTime<1)
                    sprintf(str,"Estimated Time Remaining:  Less than 1 minute");
                else
                    sprintf(str,"Estimated Time Remaining:  %d minute%s",estTime,(estTime==1)?"":"s");
                timeText->Caption=str;
            }
            if (pos!=blockStart)
                MessageBox(NULL,"Internal Error","ROM Dump",MB_OK);
            if (pos>=romSize)
            {
                sending=0;
                clCloseCom();
                activePanel->Hide();
                FinishedPanel->Show();
                activePanel=FinishedPanel;
                BackButton->Enabled=false;
                NextButton->Enabled=true;
                NextButton->Caption="Finish";
                char fn[256];
                for (int n=1;;n++)
                {
                    sprintf(fn,"%s%sti%s_%d.rom",
                        initPath,initPath[strlen(
                        initPath)-1]=='\\'?"":"\\",
                        calcName,n);
                    FILE *fp=fopen(fn,"rb");
                    if (!fp) break;
                    fclose(fp);
                }
                FILE *fp=fopen(fn,"wb");
                fwrite(buf,romSize,1,fp);
                delete[] buf;
                fclose(fp);
            }
            return;
        }
        if (activePanel==Send89Panel)
        {
            int size;
            int i,c;
            char str[64];
            if (TI92FargoII->Checked)
                size=(romDump92f2[0x4c]|(romDump92f2[0x4d]<<8))-0x5a;
            else
                size=(romDump89[0x4c]|(romDump89[0x4d]<<8))-0x5a;
            switch(stage)
            {
                case S89_HEADER:
                    clSendCom(0x89); clSendCom(6);
                    clSendCom(13); clSendCom(0);
                    clSendCom((size+2)&0xff);
                    clSendCom(((size+2)>>8)&0xff);
                    clSendCom(0); clSendCom(0);
                    csum=(size+2)&0xff;
                    csum+=((size+2)>>8)&0xff;
                    clSendCom(romDump89[0x48]);
                    csum+=romDump89[0x48];
                    clSendCom(7); clSendCom('d');
                    clSendCom('u'); clSendCom('m');
                    clSendCom('p'); clSendCom('r');
                    clSendCom('o'); clSendCom('m');
                    csum+=7+'d'+'u'+'m'+'p'+'r'+'o'+'m';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    WaitForAck(); WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send89Text2->Hide();
                        Get89TimeText->Hide();
                        Send89Progress->Position=0;
                        Send89Text1->Show();
                        Send89Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    clSendCom(0x89); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x89); clSendCom(0x15);
                    clSendCom((size+6)&0xff);
                    clSendCom(((size+6)>>8)&0xff);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0); clSendCom(0);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send89Text2->Hide();
                        Get89TimeText->Hide();
                        Send89Progress->Position=0;
                        Send89Text1->Show();
                        Send89Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x56; left=size+2;
                    Send89Progress->Max=left;
                    stage=S89_DATA;
                    break;
                case S89_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump89[pos]);
                        csum+=romDump89[pos++];
                        left--;
                        Send89Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        WaitForAck();
                        stage=S89_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send89Text2->Hide();
                        Get89TimeText->Hide();
                        Send89Progress->Position=0;
                        Send89Text1->Show();
                        Send89Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
                case S89_FINISH:
                    clSendCom(0x89); clSendCom(0x92);
                    clSendCom(0); clSendCom(0);
                    WaitForAck();
                    BackButton->Enabled=true;
                    Send89Progress->Position=0;
                    Send89Progress->Max=romSize>>10;
                    Send89Text2->Caption="Starting ROM transfer...";
                    Application->ProcessMessages();
                    SEND89KEY(0x107); SEND89KEY(0x107);
                    SEND89KEY('d'); SEND89KEY('u');
                    SEND89KEY('m'); SEND89KEY('p');
                    SEND89KEY('r'); SEND89KEY('o');
                    SEND89KEY('m'); SEND89KEY('(');
                    SEND89KEY(')'); SEND89KEY(13);
                    Sleep(1000);
                    pos=0; blockStart=0;
                    first=1;
                    buf=new unsigned char[2048*1024];
                    csum=0;
                    if (!clGetCom(c))
                    {
                        Send89Text2->Caption="The transfer did "
                            "not start.  Try running dumprom() "
                            "from the home screen or checking "
                            "the connection.";
                    }
                    else
                    {
                        buf[pos++]=c;
                        csum=c;
                    }
                    stage=DUMP_STAGE;
                    break;
                case S92F2_HEADER:
                    clSendCom(0x89); clSendCom(6);
                    clSendCom(13); clSendCom(0);
                    clSendCom((size+2)&0xff);
                    clSendCom(((size+2)>>8)&0xff);
                    clSendCom(0); clSendCom(0);
                    csum=(size+2)&0xff;
                    csum+=((size+2)>>8)&0xff;
                    clSendCom(romDump92f2[0x48]);
                    csum+=romDump92f2[0x48];
                    clSendCom(7); clSendCom('d');
                    clSendCom('u'); clSendCom('m');
                    clSendCom('p'); clSendCom('r');
                    clSendCom('o'); clSendCom('m');
                    csum+=7+'d'+'u'+'m'+'p'+'r'+'o'+'m';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    WaitForAck(); WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send89Text2->Hide();
                        Get89TimeText->Hide();
                        Send89Progress->Position=0;
                        Send89Text1->Show();
                        Send89Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    clSendCom(0x89); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x89); clSendCom(0x15);
                    clSendCom((size+6)&0xff);
                    clSendCom(((size+6)>>8)&0xff);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0); clSendCom(0);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send89Text2->Hide();
                        Get89TimeText->Hide();
                        Send89Progress->Position=0;
                        Send89Text1->Show();
                        Send89Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x56; left=size+2;
                    Send89Progress->Max=left;
                    stage=S92F2_DATA;
                    break;
                case S92F2_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump92f2[pos]);
                        csum+=romDump92f2[pos++];
                        left--;
                        Send89Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        WaitForAck();
                        stage=S89_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send89Text2->Hide();
                        Get89TimeText->Hide();
                        Send89Progress->Position=0;
                        Send89Text1->Show();
                        Send89Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
            }
        }
        if (activePanel==Send86Panel)
        {
            int size;
            int i,c;
            char str[64];
            if (TI85ZShell->Checked)
                size=(romDump85z[0x39]|(romDump85z[0x3a]<<8));
            else if (TI85Usgard->Checked)
                size=(romDump85u[0x39]|(romDump85u[0x3a]<<8));
            else if (TI82Ash->Checked)
                size=(romDump82[0x39]|(romDump82[0x3a]<<8));
            else if (TI83AShell->Checked)
                size=(romDump83[0x39]|(romDump83[0x3a]<<8));
            else if (TI83Plus->Checked)
                size=(romDump83p[0x39]|(romDump83p[0x3a]<<8));
            else
                size=(romDump86[0x39]|(romDump86[0x3a]<<8));
            switch(stage)
            {
                case S86_HEADER:
                    clSendCom(0x6); clSendCom(0xc9);
                    clSendCom(12); clSendCom(0);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    csum=(size)&0xff;
                    csum+=(size>>8)&0xff;
                    clSendCom(romDump86[0x3b]);
                    csum+=romDump86[0x3b];
                    clSendCom(7); clSendCom('d');
                    clSendCom('u'); clSendCom('m');
                    clSendCom('p'); clSendCom('r');
                    clSendCom('o'); clSendCom('m');
                    clSendCom(' ');
                    csum+=7+'d'+'u'+'m'+'p'+'r'+'o'+'m'+' ';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    Sleep(20);
                    WaitForAck(); WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    Sleep(20);
                    clSendCom(0x6); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x6); clSendCom(0x15);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x3b+romDump86[0x37]; left=size;
                    Send86Progress->Max=left;
                    stage=S86_DATA;
                    Sleep(20);
                    break;
                case S86_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump86[pos]);
                        csum+=romDump86[pos++];
                        left--;
                        Send86Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        Sleep(20);
                        WaitForAck();
                        clFlushInput();
                        Sleep(500);
                        stage=S86_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
                case S86_FINISH:
                    BackButton->Enabled=true;
                    Send86Progress->Position=0;
                    Send86Progress->Max=128;
                    if (TI85ZShell->Checked)
                    {
                        Send86Text2->Caption="Please run the \"dumprom\" "
                            "program from ZShell.";
                    }
                    else if (TI85Usgard->Checked)
                    {
                        Send86Text2->Caption="Please run the \"dumprom\" "
                            "program from Usgard.";
                    }
                    else if (TI82Ash->Checked)
                    {
                        Send86Text2->Caption="Please run the \"ROM Dump\" "
                            "program from Ash.";
                    }
                    else if (TI83AShell->Checked)
                    {
                        Send86Progress->Max=256;
                        Send86Text2->Caption="Please run the \"dumprom\" "
                            "program from AShell.";
                    }
                    else if (TI83Plus->Checked)
                    {
                        Send86Progress->Max=512;
                        Send86Text2->Caption="Please run the \"DUMPROM\" "
                            "program by executing Asm(prgmDUMPROM) "
                            "at the home screen.";
                    }
                    else
                    {
                        Send86Progress->Max=256;
                        Send86Text2->Caption="Please run the \"dumprom\" "
                            "program from a shell or run asm(dumprom) "
                            "at the home screen.";
                    }
                    Application->ProcessMessages();
                    pos=0; blockStart=0;
                    first=1;
                    buf=new unsigned char[512*1024];
                    csum=0;
                    stage=DUMP_STAGE;
                    break;
                case S85U_HEADER:
                    clSendCom(0x85); clSendCom(0x6);
                    clSendCom(11); clSendCom(0);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    csum=(size)&0xff;
                    csum+=(size>>8)&0xff;
                    clSendCom(romDump85u[0x3b]);
                    csum+=romDump85u[0x3b];
                    clSendCom(7); clSendCom('d');
                    clSendCom('u'); clSendCom('m');
                    clSendCom('p'); clSendCom('r');
                    clSendCom('o'); clSendCom('m');
                    csum+=7+'d'+'u'+'m'+'p'+'r'+'o'+'m';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    Sleep(20);
                    WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    WaitForAck();
                    for (i=0;clGetError();i++)
                    {
                        if (i>10)
                        {
                            sending=0;
                            MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                            Send86Text2->Hide();
                            Get86TimeText->Hide();
                            Send86Progress->Position=0;
                            Send86Text1->Show();
                            Send86Text2->Caption="The program is being transferred.  Please wait...";
                            NextButton->Enabled=true;
                            break;
                        }
                        else
                        {
                            clResetError();
                            WaitForAck();
                        }
                    }
                    if (clGetError()) break;
                    Sleep(20);
                    clSendCom(0x85); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x85); clSendCom(0x15);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x3b+romDump85u[0x37]; left=size;
                    Send86Progress->Max=left;
                    stage=S85U_DATA;
                    Sleep(20);
                    break;
                case S85U_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump85u[pos]);
                        csum+=romDump85u[pos++];
                        left--;
                        Send86Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        Sleep(20);
                        WaitForAck();
                        Sleep(20);
                        clSendCom(0x85); clSendCom(0x92);
                        clSendCom(0); clSendCom(0);
                        Sleep(20);
                        WaitForAck();
                        clFlushInput();
                        Sleep(500);
                        stage=S86_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
                case S85Z_HEADER:
                    clSendCom(0x85); clSendCom(0x6);
                    clSendCom(11); clSendCom(0);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    csum=(size)&0xff;
                    csum+=(size>>8)&0xff;
                    clSendCom(romDump85z[0x3b]);
                    csum+=romDump85z[0x3b];
                    clSendCom(7); clSendCom('d');
                    clSendCom('u'); clSendCom('m');
                    clSendCom('p'); clSendCom('r');
                    clSendCom('o'); clSendCom('m');
                    csum+=7+'d'+'u'+'m'+'p'+'r'+'o'+'m';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    Sleep(20);
                    WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    WaitForAck();
                    for (i=0;clGetError();i++)
                    {
                        if (i>10)
                        {
                            sending=0;
                            MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                            Send86Text2->Hide();
                            Get86TimeText->Hide();
                            Send86Progress->Position=0;
                            Send86Text1->Show();
                            Send86Text2->Caption="The program is being transferred.  Please wait...";
                            NextButton->Enabled=true;
                            break;
                        }
                        else
                        {
                            clResetError();
                            WaitForAck();
                        }
                    }
                    if (clGetError()) break;
                    Sleep(20);
                    clSendCom(0x85); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x85); clSendCom(0x15);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x3b+romDump85z[0x37]; left=size;
                    Send86Progress->Max=left;
                    stage=S85Z_DATA;
                    Sleep(20);
                    break;
                case S85Z_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump85z[pos]);
                        csum+=romDump85z[pos++];
                        left--;
                        Send86Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        Sleep(20);
                        WaitForAck();
                        Sleep(20);
                        clSendCom(0x85); clSendCom(0x92);
                        clSendCom(0); clSendCom(0);
                        Sleep(20);
                        WaitForAck();
                        clFlushInput();
                        Sleep(500);
                        stage=S86_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
                case S82_HEADER:
                    clSendCom(0x82); clSendCom(0x6);
                    clSendCom(11); clSendCom(0);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    csum=(size)&0xff;
                    csum+=(size>>8)&0xff;
                    clSendCom(romDump82[0x3b]);
                    csum+=romDump82[0x3b];
                    clSendCom('d'); clSendCom('u');
                    clSendCom('m'); clSendCom('p');
                    clSendCom('r'); clSendCom('o');
                    clSendCom('m'); clSendCom(0);
                    csum+='d'+'u'+'m'+'p'+'r'+'o'+'m';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    Sleep(20);
                    WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    WaitForAck();
                    for (i=0;clGetError();i++)
                    {
                        if (i>10)
                        {
                            sending=0;
                            MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                            Send86Text2->Hide();
                            Get86TimeText->Hide();
                            Send86Progress->Position=0;
                            Send86Text1->Show();
                            Send86Text2->Caption="The program is being transferred.  Please wait...";
                            NextButton->Enabled=true;
                            break;
                        }
                        else
                        {
                            clResetError();
                            WaitForAck();
                        }
                    }
                    if (clGetError()) break;
                    Sleep(20);
                    clSendCom(0x82); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x82); clSendCom(0x15);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x3b+romDump82[0x37]; left=size;
                    Send86Progress->Max=left;
                    stage=S82_DATA;
                    Sleep(20);
                    break;
                case S82_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump82[pos]);
                        csum+=romDump82[pos++];
                        left--;
                        Send86Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        Sleep(20);
                        WaitForAck();
                        Sleep(20);
                        clSendCom(0x82); clSendCom(0x92);
                        clSendCom(0); clSendCom(0);
                        Sleep(20);
                        WaitForAck();
                        clFlushInput();
                        Sleep(500);
                        stage=S86_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
                case S83A_HEADER:
                    clSendCom(0x3); clSendCom(0xc9);
                    clSendCom(11); clSendCom(0);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    csum=(size)&0xff;
                    csum+=(size>>8)&0xff;
                    clSendCom(romDump83[0x3b]);
                    csum+=romDump83[0x3b];
                    clSendCom('D'); clSendCom('U');
                    clSendCom('M'); clSendCom('P');
                    clSendCom('R'); clSendCom('O');
                    clSendCom('M'); clSendCom(0);
                    csum+='D'+'U'+'M'+'P'+'R'+'O'+'M';
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    Sleep(20);
                    WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    WaitForAck();
                    for (i=0;clGetError();i++)
                    {
                        if (i>10)
                        {
                            sending=0;
                            MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                            Send86Text2->Hide();
                            Get86TimeText->Hide();
                            Send86Progress->Position=0;
                            Send86Text1->Show();
                            Send86Text2->Caption="The program is being transferred.  Please wait...";
                            NextButton->Enabled=true;
                            break;
                        }
                        else
                        {
                            clResetError();
                            WaitForAck();
                        }
                    }
                    if (clGetError()) break;
                    Sleep(20);
                    clSendCom(0x3); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x3); clSendCom(0x15);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x3b+romDump83[0x37]; left=size;
                    Send86Progress->Max=left;
                    stage=S83A_DATA;
                    Sleep(20);
                    break;
                case S83A_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump83[pos]);
                        csum+=romDump83[pos++];
                        left--;
                        Send86Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        Sleep(20);
                        WaitForAck();
                        clFlushInput();
                        Sleep(500);
                        stage=S86_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
                case S83PLUS_HEADER:
                    clSendCom(0x23); clSendCom(0xc9);
                    clSendCom(13); clSendCom(0);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    csum=(size)&0xff;
                    csum+=(size>>8)&0xff;
                    clSendCom(romDump83p[0x3b]);
                    csum+=romDump83p[0x3b];
                    clSendCom('D'); clSendCom('U');
                    clSendCom('M'); clSendCom('P');
                    clSendCom('R'); clSendCom('O');
                    clSendCom('M'); clSendCom(0);
                    clSendCom(1); clSendCom(0);
                    csum+='D'+'U'+'M'+'P'+'R'+'O'+'M'+1;
                    clSendCom(csum&0xff);
                    clSendCom((csum>>8)&0xff);
                    Sleep(20);
                    WaitForAck();
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    WaitForAck();
                    for (i=0;clGetError();i++)
                    {
                        if (i>10)
                        {
                            sending=0;
                            MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                            Send86Text2->Hide();
                            Get86TimeText->Hide();
                            Send86Progress->Position=0;
                            Send86Text1->Show();
                            Send86Text2->Caption="The program is being transferred.  Please wait...";
                            NextButton->Enabled=true;
                            break;
                        }
                        else
                        {
                            clResetError();
                            WaitForAck();
                        }
                    }
                    if (clGetError()) break;
                    Sleep(20);
                    clSendCom(0x23); clSendCom(0x56);
                    clSendCom(0); clSendCom(0);
                    clSendCom(0x23); clSendCom(0x15);
                    clSendCom(size&0xff);
                    clSendCom((size>>8)&0xff);
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                        break;
                    }
                    csum=0;
                    pos=0x3b+romDump83p[0x37]; left=size;
                    Send86Progress->Max=left;
                    stage=S83PLUS_DATA;
                    Sleep(20);
                    break;
                case S83PLUS_DATA:
                    for (i=0;(i<128)&&(left);i++)
                    {
                        clSendCom(romDump83p[pos]);
                        csum+=romDump83p[pos++];
                        left--;
                        Send86Progress->StepIt();
                    }
                    if (!left)
                    {
                        clSendCom(csum&0xff);
                        clSendCom((csum>>8)&0xff);
                        Sleep(20);
                        WaitForAck();
                        clFlushInput();
                        Sleep(500);
                        stage=S86_FINISH;
                    }
                    if (clGetError())
                    {
                        clResetError();
                        sending=0;
                        MessageBox(NULL,"Could not send file.  Please check your connection.","Transmission Error",MB_OK);
                        Send86Text2->Hide();
                        Get86TimeText->Hide();
                        Send86Progress->Position=0;
                        Send86Text1->Show();
                        Send86Text2->Caption="The program is being transferred.  Please wait...";
                        NextButton->Enabled=true;
                    }
                    break;
            }
        }
    }
}

void __fastcall TROMWizardWnd::CancelButtonClick(TObject *Sender)
{
    if ((sending)&&(stage==DUMP_STAGE))
        clSendCom(0xcc);
    if (sending)
    {
        sending=0;
        clCloseCom();
    }
    ModalResult=2;
}
//---------------------------------------------------------------------------

void __fastcall TROMWizardWnd::NextButtonClick(TObject *Sender)
{
    if (activePanel==StartPanel)
    {
        StartPanel->Hide();
        LinkTypePanel->Show();
        activePanel=LinkTypePanel;
        BackButton->Enabled=true;
    }
    else if (activePanel==LinkTypePanel)
    {
//        Form1->Show();
        NextButton->Enabled=true;
        BackButton->Enabled=true;
        if (GraphLink1->Checked)
            linkType=1;
        else if (GraphLink2->Checked)
            linkType=2;
        else if (GraphLink3->Checked)
            linkType=3;
        else if (GraphLink4->Checked)
            linkType=4;
        else if (Serial1->Checked)
            linkType=5;
        else if (Serial2->Checked)
            linkType=6;
        else if (Serial3->Checked)
            linkType=7;
        else if (Serial4->Checked)
            linkType=8;
        else if (Parallel1->Checked)
            linkType=9;
        else if (Parallel2->Checked)
            linkType=10;
        else if (VirtualLink->Checked)
            linkType=-99;
        LinkTypePanel->Hide();
        if ((TI89->Checked)||(TI92Plus->Checked)||
            (TI92FargoII->Checked))
        {
            Send89Text2->Hide();
            Get89TimeText->Hide();
            Send89Text1->Show();
            Send89Progress->Min=0;
            Send89Progress->Step=1;
            Send89Progress->Position=0;
            Send89Panel->Show();
            Send89Text2->Caption="The program is being transferred.  Please wait...";
            activePanel=Send89Panel;
            sending=0;
        }
        else if (TI86->Checked)
        {
            Send86Text2->Hide();
            Get86TimeText->Hide();
            Send86Text1->Show();
            Send86Progress->Min=0;
            Send86Progress->Step=1;
            Send86Progress->Position=0;
            Send86Panel->Show();
            Send86Text1->Caption="The ROM Wizard must transfer the ROM dump program to your calculator.  Please connect your link cable to the calculator and make sure that the calculator is on and at the home screen.  Click Next when ready.";
            Send86Text2->Caption="The program is being transferred.  Please wait...";
            activePanel=Send86Panel;
            sending=0;
        }
        else if ((TI85ZShell->Checked)||(TI85Usgard->Checked))
        {
            Send86Text2->Hide();
            Get86TimeText->Hide();
            Send86Text1->Show();
            Send86Progress->Min=0;
            Send86Progress->Step=1;
            Send86Progress->Position=0;
            Send86Panel->Show();
            Send86Text1->Caption="The ROM Wizard must transfer the ROM dump program to your calculator.  Please connect your link cable to the calculator and put the calculator in receive mode.  Click Next when ready.";
            Send86Text2->Caption="The program is being transferred.  Please wait...";
            activePanel=Send86Panel;
            sending=0;
        }
        else if (TI82Ash->Checked)
        {
            Send86Text2->Hide();
            Get86TimeText->Hide();
            Send86Text1->Show();
            Send86Progress->Min=0;
            Send86Progress->Step=1;
            Send86Progress->Position=0;
            Send86Panel->Show();
            Send86Text1->Caption="The ROM Wizard must transfer the ROM dump program to your calculator.  Please connect your link cable to the calculator and put the calculator in receive mode.  Click Next when ready.";
            Send86Text2->Caption="The program is being transferred.  Please wait...";
            activePanel=Send86Panel;
            sending=0;
        }
        else if (TI83AShell->Checked)
        {
            Send86Text2->Hide();
            Get86TimeText->Hide();
            Send86Text1->Show();
            Send86Progress->Min=0;
            Send86Progress->Step=1;
            Send86Progress->Position=0;
            Send86Panel->Show();
            Send86Text1->Caption="The ROM Wizard must transfer the ROM dump program to your calculator.  Please connect your link cable to the calculator and make sure that the calculator is on and at the home screen.  Click Next when ready.";
            Send86Text2->Caption="The program is being transferred.  Please wait...";
            activePanel=Send86Panel;
            sending=0;
        }
        else if (TI83Plus->Checked)
        {
            Send86Text2->Hide();
            Get86TimeText->Hide();
            Send86Text1->Show();
            Send86Progress->Min=0;
            Send86Progress->Step=1;
            Send86Progress->Position=0;
            Send86Panel->Show();
            Send86Text1->Caption="The ROM Wizard must transfer the ROM dump program to your calculator.  Please connect your link cable to the calculator and make sure that the calculator is on and at the home screen.  Click Next when ready.";
            Send86Text2->Caption="The program is being transferred.  Please wait...";
            activePanel=Send86Panel;
            sending=0;
        }
        clInitCom(linkType,200);
    }
    else if (activePanel==Send89Panel)
    {
        Send89Text1->Hide();
        Send89Text2->Show();
        sending=1;
        if (TI92FargoII->Checked)
            stage=S92F2_HEADER;
        else
            stage=S89_HEADER;
        progress=Send89Progress;
        transferText=Send89Text2;
        timeText=Get89TimeText;
        romSize=1024*2048;
        if (TI92FargoII->Checked)
        {
            romSize=1024*1024;
            strcpy(calcName,"92");
        }
        NextButton->Enabled=false;
    }
    else if (activePanel==Send86Panel)
    {
        Send86Text1->Hide();
        Send86Text2->Show();
        sending=1;
        if ((TI85ZShell->Checked)||(TI85Usgard->Checked))
        {
            stage=TI85ZShell->Checked?S85Z_HEADER:S85U_HEADER;
            progress=Send86Progress;
            transferText=Send86Text2;
            timeText=Get86TimeText;
            romSize=1024*128;
            strcpy(calcName,"85");
        }
        else if (TI82Ash->Checked)
        {
            stage=S82_HEADER;
            progress=Send86Progress;
            transferText=Send86Text2;
            timeText=Get86TimeText;
            romSize=1024*128;
            strcpy(calcName,"82");
        }
        else if (TI83AShell->Checked)
        {
            stage=S83A_HEADER;
            progress=Send86Progress;
            transferText=Send86Text2;
            timeText=Get86TimeText;
            romSize=1024*256;
            strcpy(calcName,"83");
        }
        else if (TI83Plus->Checked)
        {
            stage=S83PLUS_HEADER;
            progress=Send86Progress;
            transferText=Send86Text2;
            timeText=Get86TimeText;
            romSize=1024*512;
            strcpy(calcName,"83p");
        }
        else
        {
            stage=S86_HEADER;
            progress=Send86Progress;
            transferText=Send86Text2;
            timeText=Get86TimeText;
            romSize=1024*256;
            strcpy(calcName,"86");
        }
        NextButton->Enabled=false;
    }
    else if (activePanel==FinishedPanel)
        ModalResult=1;
}
//---------------------------------------------------------------------------

void __fastcall TROMWizardWnd::BackButtonClick(TObject *Sender)
{
    if (activePanel==LinkTypePanel)
    {
        LinkTypePanel->Hide();
        StartPanel->Show();
        activePanel=StartPanel;
        BackButton->Enabled=false;
    }
    else if (activePanel==Send89Panel)
    {
        if (Send89Text1->Visible)
        {
            Send89Panel->Hide();
            LinkTypePanel->Show();
            activePanel=LinkTypePanel;
            clCloseCom();
        }
        else
        {
            if ((sending)&&(stage==DUMP_STAGE))
            {
                clFlushInput();
                clSendCom(0xcc);
            }
            sending=0;
            Send89Text2->Hide();
            Get89TimeText->Hide();
            Send89Progress->Position=0;
            Send89Text1->Show();
            Send89Text2->Caption="The program is being transferred.  Please wait...";
            NextButton->Enabled=true;
        }
    }
    else if (activePanel==Send86Panel)
    {
        if (Send86Text1->Visible)
        {
            Send86Panel->Hide();
            LinkTypePanel->Show();
            activePanel=LinkTypePanel;
        }
        else
        {
            if ((sending)&&(stage==DUMP_STAGE))
            {
                clFlushInput();
                clSendCom(0xcc);
            }
            sending=0;
            Send86Text2->Hide();
            Get86TimeText->Hide();
            Send86Progress->Position=0;
            Send86Text1->Show();
            Send86Text2->Caption="The program is being transferred.  Please wait...";
            NextButton->Enabled=true;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TROMWizardWnd::FormDestroy(TObject *Sender)
{
    Application->OnIdle=NULL;
}
//---------------------------------------------------------------------------

void __fastcall TROMWizardWnd::CalcClick(TObject *Sender)
{
    NextButton->Enabled=true;
}
//---------------------------------------------------------------------------

void __fastcall TROMWizardWnd::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    static int vHit=0;
    if ((Key=='L')&&(vHit))
        VirtualLink->Show();
    else if (Key=='V')
        vHit=1;
    else
        vHit=0;
}
//---------------------------------------------------------------------------

void __fastcall TROMWizardWnd::FormCreate(TObject *Sender)
{
    Image1->Picture=WizardBitmap->Bmp->Picture;
}
//---------------------------------------------------------------------------

