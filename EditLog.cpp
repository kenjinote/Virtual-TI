//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "EditLog.h"
#include "AddLog.h"
#define NOTYPEREDEF
#include "m68000.h"
#include "rom.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"

#define DATABREAK_READ 1
#define DATABREAK_WRITE 2
#define DATABREAK_BOTH 3

#define LOG_VALUE 1
#define LOG_BIT 2

//extern int readmem24(int a);

TEditLogDlg *EditLogDlg;
//---------------------------------------------------------------------
__fastcall TEditLogDlg::TEditLogDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
extern int debugLogEnable;
extern int debugLogCount;
extern int debugLogClass[256];
extern int debugLogType[256];
extern int debugLogLow[256];
extern int debugLogHigh[256];
extern int debugLogCheck[256];
extern int debugLogLast[256];
extern int debugLogState[256];
extern int debugLogAny[256];
void __fastcall TEditLogDlg::AddBtnClick(TObject *Sender)
{
    if (AddLogDlg->ShowModal()==1)
    {
        debugLogEnable=1;
        int i=debugLogCount;
        if (AddLogDlg->Page->ActivePage==AddLogDlg->ValuePage)
        {
            debugLogClass[i]=LOG_VALUE;
            if (AddLogDlg->Read->Checked)
                debugLogType[i]=DATABREAK_READ;
            else if (AddLogDlg->Write->Checked)
                debugLogType[i]=DATABREAK_WRITE;
            else
                debugLogType[i]=DATABREAK_BOTH;
            if (AddLogDlg->Single->Checked)
            {
                sscanf(AddLogDlg->Addr->Text.c_str(),"%x",
                    &debugLogLow[i]);
                if (AddLogDlg->Size->ItemIndex==0)
                    debugLogHigh[i]=debugLogLow[i];
                else if (AddLogDlg->Size->ItemIndex==0)
                    debugLogHigh[i]=debugLogLow[i]+1;
                else
                    debugLogHigh[i]=debugLogLow[i]+3;
                if (AddLogDlg->IOPort1->Checked)
                {
                    debugLogLow[i]=(debugLogLow[i]&0xff)+
                        0x600000;
                    debugLogHigh[i]=(debugLogHigh[i]&0xff)+
                        0x600000;
                }
            }
            else
            {
                sscanf(AddLogDlg->LowAddr->Text.c_str(),
                    "%x",&debugLogLow[i]);
                sscanf(AddLogDlg->HighAddr->Text.c_str(),
                    "%x",&debugLogHigh[i]);
                if (AddLogDlg->IOPort1->Checked)
                {
                    debugLogLow[i]=(debugLogLow[i]&0xff)+
                        0x600000;
                    debugLogHigh[i]=(debugLogHigh[i]&0xff)+
                        0x600000;
                }
            }
        }
        else
        {
            debugLogClass[i]=LOG_BIT;
            sscanf(AddLogDlg->BitAddr->Text.c_str(),"%x",
                &debugLogLow[i]);
            if (AddLogDlg->IOPort2->Checked)
            {
                debugLogLow[i]=(debugLogLow[i]&0xff)+
                    0x600000;
            }
            debugLogCheck[i]=0;
            if (AddLogDlg->Check7->Checked) debugLogCheck[i]|=0x80;
            if (AddLogDlg->Check6->Checked) debugLogCheck[i]|=0x40;
            if (AddLogDlg->Check5->Checked) debugLogCheck[i]|=0x20;
            if (AddLogDlg->Check4->Checked) debugLogCheck[i]|=0x10;
            if (AddLogDlg->Check3->Checked) debugLogCheck[i]|=0x8;
            if (AddLogDlg->Check2->Checked) debugLogCheck[i]|=0x4;
            if (AddLogDlg->Check1->Checked) debugLogCheck[i]|=0x2;
            if (AddLogDlg->Check0->Checked) debugLogCheck[i]|=0x1;
            debugLogLast[i]=cpu_readmem24(debugLogLow[i])&debugLogCheck[i];
            debugLogState[i]=0;
            if (AddLogDlg->State7->State==cbChecked) debugLogState[i]|=0x80;
            if (AddLogDlg->State6->State==cbChecked) debugLogState[i]|=0x40;
            if (AddLogDlg->State5->State==cbChecked) debugLogState[i]|=0x20;
            if (AddLogDlg->State4->State==cbChecked) debugLogState[i]|=0x10;
            if (AddLogDlg->State3->State==cbChecked) debugLogState[i]|=0x8;
            if (AddLogDlg->State2->State==cbChecked) debugLogState[i]|=0x4;
            if (AddLogDlg->State1->State==cbChecked) debugLogState[i]|=0x2;
            if (AddLogDlg->State0->State==cbChecked) debugLogState[i]|=0x1;
            debugLogAny[i]=0;
            if (AddLogDlg->State7->State==cbGrayed) debugLogAny[i]|=0x80;
            if (AddLogDlg->State6->State==cbGrayed) debugLogAny[i]|=0x40;
            if (AddLogDlg->State5->State==cbGrayed) debugLogAny[i]|=0x20;
            if (AddLogDlg->State4->State==cbGrayed) debugLogAny[i]|=0x10;
            if (AddLogDlg->State3->State==cbGrayed) debugLogAny[i]|=0x8;
            if (AddLogDlg->State2->State==cbGrayed) debugLogAny[i]|=0x4;
            if (AddLogDlg->State1->State==cbGrayed) debugLogAny[i]|=0x2;
            if (AddLogDlg->State0->State==cbGrayed) debugLogAny[i]|=0x1;
            if (!debugLogCheck[i]) return;
        }
        debugLogCount++;
        UpdateList();
    }
}
//---------------------------------------------------------------------------

void __fastcall TEditLogDlg::UpdateList()
{
    List->Clear();
    char str[256];
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    for (int i=0;i<debugLogCount;i++)
    {
        if (debugLogClass[i]==LOG_VALUE)
        {
            if ((z80)&&((debugLogLow[i]>>20)==6))
            {
                if (debugLogType[i]==DATABREAK_READ)
                    sprintf(str,"Read of I/O port %X - %X",debugLogLow[i]&0xff,debugLogHigh[i]&0xff);
                else if (debugLogType[i]==DATABREAK_WRITE)
                    sprintf(str,"Write to I/O port %X - %X",debugLogLow[i]&0xff,debugLogHigh[i]&0xff);
                else
                    sprintf(str,"Read/write of I/O port %X - %X",debugLogLow[i]&0xff,debugLogHigh[i]&0xff);
            }
            else
            {
                if (debugLogType[i]==DATABREAK_READ)
                    sprintf(str,"Read of %X - %X",debugLogLow[i],debugLogHigh[i]);
                else if (debugLogType[i]==DATABREAK_WRITE)
                    sprintf(str,"Write to %X - %X",debugLogLow[i],debugLogHigh[i]);
                else
                    sprintf(str,"Read/write of %X - %X",debugLogLow[i],debugLogHigh[i]);
            }
            List->Items->Add(str);
        }
        else
        {
            char set[32],clear[32],change[32],tmp[2];
            int j,b;
            set[0]=0; clear[0]=0; change[0]=0; tmp[1]=0;
            for (j=0,b=1;j<8;j++,b<<=1)
            {
                if (debugLogCheck[i]&b)
                {
                    if (debugLogAny[i]&b)
                    {
                        if (change[0])
                            strcat(change,",");
                        tmp[0]='0'+j; strcat(change,tmp);
                    }
                    else
                    {
                        if (debugLogState[i]&b)
                        {
                            if (set[0])
                                strcat(set,",");
                            tmp[0]='0'+j; strcat(set,tmp);
                        }
                        else
                        {
                            if (clear[0])
                                strcat(clear,",");
                            tmp[0]='0'+j; strcat(clear,tmp);
                        }
                    }
                }
            }
            str[0]=0;
            if (set[0])
            {
                strcat(str,"Set bit ");
                strcat(str,set);
            }
            if (clear[0])
            {
                if (str[0])
                    strcat(str,", ");
                strcat(str,"Clear bit ");
                strcat(str,clear);
            }
            if (change[0])
            {
                if (str[0])
                    strcat(str,", ");
                strcat(str,"Change bit ");
                strcat(str,change);
            }
            strcat(str," in ");
            if ((z80)&&((debugLogLow[i]>>20)==6))
                sprintf(set,"I/O port %X",debugLogLow[i]&0xff);
            else
                sprintf(set,"%X",debugLogLow[i]);
            strcat(str,set);
            List->Items->Add(str);
        }
    }
}

void __fastcall TEditLogDlg::ListDblClick(TObject *Sender)
{
    if (List->ItemIndex==-1) return;
    int i=List->ItemIndex;
    char str[32];
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    if (debugLogClass[i]==LOG_VALUE)
    {
        AddLogDlg->Page->ActivePage=AddLogDlg->ValuePage;
        if (debugLogType[i]==DATABREAK_READ)
        {
            AddLogDlg->Read->Checked=true;
            AddLogDlg->Write->Checked=false;
            AddLogDlg->Either->Checked=false;
        }
        else if (debugLogType[i]==DATABREAK_WRITE)
        {
            AddLogDlg->Read->Checked=false;
            AddLogDlg->Write->Checked=true;
            AddLogDlg->Either->Checked=false;
        }
        else
        {
            AddLogDlg->Read->Checked=false;
            AddLogDlg->Write->Checked=false;
            AddLogDlg->Either->Checked=true;
        }
        if ((debugLogHigh[i]-debugLogLow[i])==0)
        {
            AddLogDlg->Single->Checked=true;
            AddLogDlg->Range->Checked=false;
            AddLogDlg->Size->ItemIndex=0;
            sprintf(str,"%X",(z80&&((debugLogLow[i]>>20)==6))?
                (debugLogLow[i]&0xff):debugLogLow[i]);
            AddLogDlg->Addr->Text=str;
        }
        else if ((debugLogHigh[i]-debugLogLow[i])==1)
        {
            AddLogDlg->Single->Checked=true;
            AddLogDlg->Range->Checked=false;
            AddLogDlg->Size->ItemIndex=1;
            sprintf(str,"%X",(z80&&((debugLogLow[i]>>20)==6))?
                (debugLogLow[i]&0xff):debugLogLow[i]);
            AddLogDlg->IOPort1->Checked=(z80&&((debugLogLow[i]>>20)==6));
            AddLogDlg->Addr->Text=str;
        }
        if ((debugLogHigh[i]-debugLogLow[i])==3)
        {
            AddLogDlg->Single->Checked=true;
            AddLogDlg->Range->Checked=false;
            AddLogDlg->Size->ItemIndex=2;
            sprintf(str,"%X",(z80&&((debugLogLow[i]>>20)==6))?
                (debugLogLow[i]&0xff):debugLogLow[i]);
            AddLogDlg->IOPort1->Checked=(z80&&((debugLogLow[i]>>20)==6));
            AddLogDlg->Addr->Text=str;
        }
        else
        {
            AddLogDlg->Single->Checked=false;
            AddLogDlg->Range->Checked=true;
            sprintf(str,"%X",(z80&&((debugLogLow[i]>>20)==6))?
                (debugLogLow[i]&0xff):debugLogLow[i]);
            AddLogDlg->LowAddr->Text=str;
            sprintf(str,"%X",(z80&&((debugLogLow[i]>>20)==6))?
                (debugLogHigh[i]&0xff):debugLogHigh[i]);
            AddLogDlg->IOPort1->Checked=(z80&&((debugLogLow[i]>>20)==6));
            AddLogDlg->HighAddr->Text=str;
        }
    }
    else
    {
        AddLogDlg->Page->ActivePage=AddLogDlg->BitPage;
        sprintf(str,"%X",(z80&&((debugLogLow[i]>>20)==6))?
            (debugLogLow[i]&0xff):debugLogLow[i]);
        AddLogDlg->IOPort2->Checked=(z80&&((debugLogLow[i]>>20)==6));
        AddLogDlg->BitAddr->Text=str;
        AddLogDlg->Check7->Checked=debugLogCheck[i]&0x80?true:false;
        AddLogDlg->Check6->Checked=debugLogCheck[i]&0x40?true:false;
        AddLogDlg->Check5->Checked=debugLogCheck[i]&0x20?true:false;
        AddLogDlg->Check4->Checked=debugLogCheck[i]&0x10?true:false;
        AddLogDlg->Check3->Checked=debugLogCheck[i]&0x8?true:false;
        AddLogDlg->Check2->Checked=debugLogCheck[i]&0x4?true:false;
        AddLogDlg->Check1->Checked=debugLogCheck[i]&0x2?true:false;
        AddLogDlg->Check0->Checked=debugLogCheck[i]&0x1?true:false;
        AddLogDlg->State7->State=debugLogAny[i]&0x80?cbGrayed:((debugLogState[i]&0x80)?cbChecked:cbUnchecked);
        AddLogDlg->State6->State=debugLogAny[i]&0x40?cbGrayed:((debugLogState[i]&0x40)?cbChecked:cbUnchecked);
        AddLogDlg->State5->State=debugLogAny[i]&0x20?cbGrayed:((debugLogState[i]&0x20)?cbChecked:cbUnchecked);
        AddLogDlg->State4->State=debugLogAny[i]&0x10?cbGrayed:((debugLogState[i]&0x10)?cbChecked:cbUnchecked);
        AddLogDlg->State3->State=debugLogAny[i]&0x8?cbGrayed:((debugLogState[i]&0x8)?cbChecked:cbUnchecked);
        AddLogDlg->State2->State=debugLogAny[i]&0x4?cbGrayed:((debugLogState[i]&0x4)?cbChecked:cbUnchecked);
        AddLogDlg->State1->State=debugLogAny[i]&0x2?cbGrayed:((debugLogState[i]&0x2)?cbChecked:cbUnchecked);
        AddLogDlg->State0->State=debugLogAny[i]&0x1?cbGrayed:((debugLogState[i]&0x1)?cbChecked:cbUnchecked);
    }
    if (AddLogDlg->ShowModal()==1)
    {
        debugLogEnable=1;
        for (;i<(debugLogCount-1);i++)
        {
            debugLogClass[i]=debugLogClass[i+1];
            debugLogType[i]=debugLogType[i+1];
            debugLogLow[i]=debugLogLow[i+1];
            debugLogHigh[i]=debugLogHigh[i+1];
            debugLogCheck[i]=debugLogCheck[i+1];
            debugLogState[i]=debugLogState[i+1];
            debugLogAny[i]=debugLogAny[i+1];
        }
        debugLogCount--;
        i=debugLogCount;
        if (AddLogDlg->Page->ActivePage==AddLogDlg->ValuePage)
        {
            debugLogClass[i]=LOG_VALUE;
            if (AddLogDlg->Read->Checked)
                debugLogType[i]=DATABREAK_READ;
            else if (AddLogDlg->Write->Checked)
                debugLogType[i]=DATABREAK_WRITE;
            else
                debugLogType[i]=DATABREAK_BOTH;
            if (AddLogDlg->Single->Checked)
            {
                sscanf(AddLogDlg->Addr->Text.c_str(),"%x",
                    &debugLogLow[i]);
                if (AddLogDlg->Size->ItemIndex==0)
                    debugLogHigh[i]=debugLogLow[i];
                else if (AddLogDlg->Size->ItemIndex==0)
                    debugLogHigh[i]=debugLogLow[i]+1;
                else
                    debugLogHigh[i]=debugLogLow[i]+3;
                if (AddLogDlg->IOPort1->Checked)
                {
                    debugLogLow[i]=(debugLogLow[i]&0xff)+
                        0x600000;
                    debugLogHigh[i]=(debugLogHigh[i]&0xff)+
                        0x600000;
                }
            }
            else
            {
                sscanf(AddLogDlg->LowAddr->Text.c_str(),
                    "%x",&debugLogLow[i]);
                sscanf(AddLogDlg->HighAddr->Text.c_str(),
                    "%x",&debugLogHigh[i]);
                if (AddLogDlg->IOPort1->Checked)
                {
                    debugLogLow[i]=(debugLogLow[i]&0xff)+
                        0x600000;
                    debugLogHigh[i]=(debugLogHigh[i]&0xff)+
                        0x600000;
                }
            }
        }
        else
        {
            debugLogClass[i]=LOG_BIT;
            sscanf(AddLogDlg->BitAddr->Text.c_str(),"%x",
                &debugLogLow[i]);
            if (AddLogDlg->IOPort2->Checked)
            {
                debugLogLow[i]=(debugLogLow[i]&0xff)+
                    0x600000;
            }
            debugLogCheck[i]=0;
            if (AddLogDlg->Check7->Checked) debugLogCheck[i]|=0x80;
            if (AddLogDlg->Check6->Checked) debugLogCheck[i]|=0x40;
            if (AddLogDlg->Check5->Checked) debugLogCheck[i]|=0x20;
            if (AddLogDlg->Check4->Checked) debugLogCheck[i]|=0x10;
            if (AddLogDlg->Check3->Checked) debugLogCheck[i]|=0x8;
            if (AddLogDlg->Check2->Checked) debugLogCheck[i]|=0x4;
            if (AddLogDlg->Check1->Checked) debugLogCheck[i]|=0x2;
            if (AddLogDlg->Check0->Checked) debugLogCheck[i]|=0x1;
            debugLogLast[i]=cpu_readmem24(debugLogLow[i])&debugLogCheck[i];
            debugLogState[i]=0;
            if (AddLogDlg->State7->State==cbChecked) debugLogState[i]|=0x80;
            if (AddLogDlg->State6->State==cbChecked) debugLogState[i]|=0x40;
            if (AddLogDlg->State5->State==cbChecked) debugLogState[i]|=0x20;
            if (AddLogDlg->State4->State==cbChecked) debugLogState[i]|=0x10;
            if (AddLogDlg->State3->State==cbChecked) debugLogState[i]|=0x8;
            if (AddLogDlg->State2->State==cbChecked) debugLogState[i]|=0x4;
            if (AddLogDlg->State1->State==cbChecked) debugLogState[i]|=0x2;
            if (AddLogDlg->State0->State==cbChecked) debugLogState[i]|=0x1;
            debugLogAny[i]=0;
            if (AddLogDlg->State7->State==cbGrayed) debugLogAny[i]|=0x80;
            if (AddLogDlg->State6->State==cbGrayed) debugLogAny[i]|=0x40;
            if (AddLogDlg->State5->State==cbGrayed) debugLogAny[i]|=0x20;
            if (AddLogDlg->State4->State==cbGrayed) debugLogAny[i]|=0x10;
            if (AddLogDlg->State3->State==cbGrayed) debugLogAny[i]|=0x8;
            if (AddLogDlg->State2->State==cbGrayed) debugLogAny[i]|=0x4;
            if (AddLogDlg->State1->State==cbGrayed) debugLogAny[i]|=0x2;
            if (AddLogDlg->State0->State==cbGrayed) debugLogAny[i]|=0x1;
            if (!debugLogCheck[i]) return;
        }
        debugLogCount++;
        UpdateList();
    }
}
//---------------------------------------------------------------------------

void __fastcall TEditLogDlg::DelBtnClick(TObject *Sender)
{
    if (List->ItemIndex==-1) return;
    int i=List->ItemIndex;
    for (;i<(debugLogCount-1);i++)
    {
        debugLogClass[i]=debugLogClass[i+1];
        debugLogType[i]=debugLogType[i+1];
        debugLogLow[i]=debugLogLow[i+1];
        debugLogHigh[i]=debugLogHigh[i+1];
        debugLogCheck[i]=debugLogCheck[i+1];
        debugLogState[i]=debugLogState[i+1];
        debugLogAny[i]=debugLogAny[i+1];
    }
    debugLogCount--;
    if (!debugLogCount)
        debugLogEnable=0;
    UpdateList();
}
//---------------------------------------------------------------------------

