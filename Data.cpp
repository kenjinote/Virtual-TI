//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "Data.h"
#include "AddData.h"
#include "rom.h"
#include "Debug.h"

//---------------------------------------------------------------------
#pragma resource "*.dfm"
TDataBreakDlg *DataBreakDlg;

extern int debugDataBreakCount;
extern int debugDataBreakType[256];
extern int debugDataBreakLow[256];
extern int debugDataBreakHigh[256];
//---------------------------------------------------------------------
__fastcall TDataBreakDlg::TDataBreakDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}

void __fastcall TDataBreakDlg::UpdateList()
{
    char str[256];
    List->Clear();
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    for (int i=0;i<debugDataBreakCount;i++)
    {
        if (debugDataBreakType[i]==DATABREAK_READ)
        {
            if ((z80)&&(debugDataBreakLow[i]>=0x600000))
            {
                sprintf(str,"Read of I/O port %X - %X",
                    debugDataBreakLow[i]&0xff,
                    debugDataBreakHigh[i]&0xff);
            }
            else
            {
                sprintf(str,"Read of %X - %X",debugDataBreakLow[i],
                    debugDataBreakHigh[i]);
            }
            List->Items->Add(str);
        }
        else if (debugDataBreakType[i]==DATABREAK_WRITE)
        {
            if ((z80)&&(debugDataBreakLow[i]>=0x600000))
            {
                sprintf(str,"Write to I/O port %X - %X",
                    debugDataBreakLow[i]&0xff,
                    debugDataBreakHigh[i]&0xff);
            }
            else
            {
                sprintf(str,"Write to %X - %X",debugDataBreakLow[i],
                    debugDataBreakHigh[i]);
            }
            List->Items->Add(str);
        }
        else
        {
            if ((z80)&&(debugDataBreakLow[i]>=0x600000))
            {
                sprintf(str,"Read/write of I/O port %X - %X",
                    debugDataBreakLow[i]&0xff,
                    debugDataBreakHigh[i]&0xff);
            }
            else
            {
                sprintf(str,"Read/write of %X - %X",debugDataBreakLow[i],
                    debugDataBreakHigh[i]);
            }
            List->Items->Add(str);
        }
    }
}
//---------------------------------------------------------------------
void __fastcall TDataBreakDlg::AddBtnClick(TObject *Sender)
{
    if (AddDataBreakDlg->ShowModal()==1)
    {
        int i=debugDataBreakCount;
        if (AddDataBreakDlg->Read->Checked)
            debugDataBreakType[i]=DATABREAK_READ;
        else if (AddDataBreakDlg->Write->Checked)
            debugDataBreakType[i]=DATABREAK_WRITE;
        else
            debugDataBreakType[i]=DATABREAK_BOTH;
        if (AddDataBreakDlg->Single->Checked)
        {
            sscanf(AddDataBreakDlg->Addr->Text.c_str(),
                "%X",&debugDataBreakLow[i]);
            if (AddDataBreakDlg->Size->ItemIndex==0)
                debugDataBreakHigh[i]=debugDataBreakLow[i];
            else if (AddDataBreakDlg->Size->ItemIndex==1)
                debugDataBreakHigh[i]=debugDataBreakLow[i]+1;
            else
                debugDataBreakHigh[i]=debugDataBreakLow[i]+3;
            if (AddDataBreakDlg->IOPort->Checked)
            {
                debugDataBreakLow[i]=(debugDataBreakLow[i]&0xff)+
                    0x600000;
                debugDataBreakHigh[i]=(debugDataBreakHigh[i]&0xff)+
                    0x600000;
            }
        }
        else
        {
            sscanf(AddDataBreakDlg->LowAddr->Text.c_str(),
                "%X",&debugDataBreakLow[i]);
            sscanf(AddDataBreakDlg->HighAddr->Text.c_str(),
                "%X",&debugDataBreakHigh[i]);
            if (AddDataBreakDlg->IOPort->Checked)
            {
                debugDataBreakLow[i]=(debugDataBreakLow[i]&0xff)+
                    0x600000;
                debugDataBreakHigh[i]=(debugDataBreakHigh[i]&0xff)+
                    0x600000;
            }
        }
        debugDataBreakCount++;
    }
    ModalResult=0;
    UpdateList();
}
//---------------------------------------------------------------------------

void __fastcall TDataBreakDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TDataBreakDlg::Button1Click(TObject *Sender)
{
    if (List->ItemIndex==-1) return;
    for (int i=List->ItemIndex;i<(debugDataBreakCount-1);i++)
    {
        debugDataBreakType[i]=debugDataBreakType[i+1];
        debugDataBreakLow[i]=debugDataBreakLow[i+1];
        debugDataBreakHigh[i]=debugDataBreakHigh[i+1];
    }
    debugDataBreakCount--;
    UpdateList();
}
//---------------------------------------------------------------------------

void __fastcall TDataBreakDlg::ListDblClick(TObject *Sender)
{
    if (List->ItemIndex==-1) return;
    int i=List->ItemIndex;
    char str[16];
    int z80=romImage[currentROM].type&ROMFLAG_Z80;
    if (debugDataBreakType[i]==DATABREAK_READ)
    {
        AddDataBreakDlg->Read->Checked=true;
        AddDataBreakDlg->Write->Checked=false;
        AddDataBreakDlg->Either->Checked=false;
    }
    else if (debugDataBreakType[i]==DATABREAK_WRITE)
    {
        AddDataBreakDlg->Read->Checked=false;
        AddDataBreakDlg->Write->Checked=true;
        AddDataBreakDlg->Either->Checked=false;
    }
    else
    {
        AddDataBreakDlg->Read->Checked=false;
        AddDataBreakDlg->Write->Checked=false;
        AddDataBreakDlg->Either->Checked=true;
    }
    if (debugDataBreakLow[i]==debugDataBreakHigh[i])
    {
        sprintf(str,"%X",(z80&&((debugDataBreakLow[i]>>20)==6))?
            (debugDataBreakLow[i]&0xff):debugDataBreakLow[i]);
        AddDataBreakDlg->Addr->Text=str;
        AddDataBreakDlg->Single->Checked=true;
        AddDataBreakDlg->Range->Checked=false;
        AddDataBreakDlg->IOPort->Checked=(z80&&((debugDataBreakLow[i]>>20)==6));
        AddDataBreakDlg->Size->ItemIndex=0;
        AddDataBreakDlg->LowAddr->Text="";
        AddDataBreakDlg->HighAddr->Text="";
    }
    else if ((debugDataBreakHigh[i]-debugDataBreakLow[i])==1)
    {
        sprintf(str,"%X",(z80&&((debugDataBreakLow[i]>>20)==6))?
            (debugDataBreakLow[i]&0xff):debugDataBreakLow[i]);
        AddDataBreakDlg->Addr->Text=str;
        AddDataBreakDlg->Single->Checked=true;
        AddDataBreakDlg->Range->Checked=false;
        AddDataBreakDlg->IOPort->Checked=(z80&&((debugDataBreakLow[i]>>20)==6));
        AddDataBreakDlg->Size->ItemIndex=1;
        AddDataBreakDlg->LowAddr->Text="";
        AddDataBreakDlg->HighAddr->Text="";
    }
    else if ((debugDataBreakHigh[i]-debugDataBreakLow[i])==3)
    {
        sprintf(str,"%X",(z80&&((debugDataBreakLow[i]>>20)==6))?
            (debugDataBreakLow[i]&0xff):debugDataBreakLow[i]);
        AddDataBreakDlg->Addr->Text=str;
        AddDataBreakDlg->Single->Checked=true;
        AddDataBreakDlg->Range->Checked=false;
        AddDataBreakDlg->IOPort->Checked=(z80&&((debugDataBreakLow[i]>>20)==6));
        AddDataBreakDlg->Size->ItemIndex=2;
        AddDataBreakDlg->LowAddr->Text="";
        AddDataBreakDlg->HighAddr->Text="";
    }
    else
    {
        sprintf(str,"%X",(z80&&((debugDataBreakLow[i]>>20)==6))?
            (debugDataBreakLow[i]&0xff):debugDataBreakLow[i]);
        AddDataBreakDlg->LowAddr->Text=str;
        sprintf(str,"%X",(z80&&((debugDataBreakLow[i]>>20)==6))?
            (debugDataBreakHigh[i]&0xff):debugDataBreakHigh[i]);
        AddDataBreakDlg->HighAddr->Text=str;
        AddDataBreakDlg->Addr->Text="";
        AddDataBreakDlg->Single->Checked=false;
        AddDataBreakDlg->Range->Checked=true;
        AddDataBreakDlg->IOPort->Checked=(z80&&((debugDataBreakLow[i]>>20)==6));
        AddDataBreakDlg->Size->ItemIndex=-1;
    }
    if (AddDataBreakDlg->ShowModal()==1)
    {
        if (AddDataBreakDlg->Read->Checked)
            debugDataBreakType[i]=DATABREAK_READ;
        else if (AddDataBreakDlg->Write->Checked)
            debugDataBreakType[i]=DATABREAK_WRITE;
        else
            debugDataBreakType[i]=DATABREAK_BOTH;
        if (AddDataBreakDlg->Single->Checked)
        {
            sscanf(AddDataBreakDlg->Addr->Text.c_str(),
                "%X",&debugDataBreakLow[i]);
            if (AddDataBreakDlg->Size->ItemIndex==0)
                debugDataBreakHigh[i]=debugDataBreakLow[i];
            else if (AddDataBreakDlg->Size->ItemIndex==1)
                debugDataBreakHigh[i]=debugDataBreakLow[i]+1;
            else
                debugDataBreakHigh[i]=debugDataBreakLow[i]+3;
            if (AddDataBreakDlg->IOPort->Checked)
            {
                debugDataBreakLow[i]=(debugDataBreakLow[i]&0xff)+
                    0x600000;
                debugDataBreakHigh[i]=(debugDataBreakHigh[i]&0xff)+
                    0x600000;
            }
        }
        else
        {
            sscanf(AddDataBreakDlg->LowAddr->Text.c_str(),
                "%X",&debugDataBreakLow[i]);
            sscanf(AddDataBreakDlg->HighAddr->Text.c_str(),
                "%X",&debugDataBreakHigh[i]);
            if (AddDataBreakDlg->IOPort->Checked)
            {
                debugDataBreakLow[i]=(debugDataBreakLow[i]&0xff)+
                    0x600000;
                debugDataBreakHigh[i]=(debugDataBreakHigh[i]&0xff)+
                    0x600000;
            }
        }
    }
    ModalResult=0;
    UpdateList();
}
//---------------------------------------------------------------------------

