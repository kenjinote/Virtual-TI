//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "EmuMode.h"
#include "rom.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TEmuModeDlg *EmuModeDlg;
extern int rom89Present,rom92Present,rom92PlusPresent;
extern int calc,plusMod,intRom92;
extern int romImageCount;
//---------------------------------------------------------------------
__fastcall TEmuModeDlg::TEmuModeDlg(TComponent* AOwner)
	: TForm(AOwner)
{
    romID=new int[romImageCount];
    int c82=0,c83=0,c83p=0,c85=0,c86=0,c89=0,c92=0,c92p=0;
    int curRomTab=0;
    for (int i=0;i<romImageCount;i++)
    {
        if (romImage[i].type&ROMFLAG_TI82) { c82=1; if (currentROM==i) curRomTab=82; }
        else if (romImage[i].type&ROMFLAG_TI83)
        {
            if (romImage[i].type&ROMFLAG_FLASHROM) { c83p=1; if (currentROM==i) curRomTab=84; }
            else { c83=1; if (currentROM==i) curRomTab=83; }
        }
        else if (romImage[i].type&ROMFLAG_TI85) { c85=1; if (currentROM==i) curRomTab=85; }
        else if (romImage[i].type&ROMFLAG_TI86) { c86=1; if (currentROM==i) curRomTab=86; }
        else if (romImage[i].type&ROMFLAG_TI89) { c89=1; if (currentROM==i) curRomTab=89; }
        else
        {
            if (romImage[i].type&ROMFLAG_FLASHROM) { c92p=1; if (currentROM==i) curRomTab=93; }
            else { c92=1; if (currentROM==i) curRomTab=92; }
        }
    }
    int selCalc;
    if (c82) { Tab->Tabs->Add("TI-82"); if (curRomTab==82) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c83) { Tab->Tabs->Add("TI-83"); if (curRomTab==83) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c83p) { Tab->Tabs->Add("TI-83+"); if (curRomTab==84) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c85) { Tab->Tabs->Add("TI-85"); if (curRomTab==85) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c86) { Tab->Tabs->Add("TI-86"); if (curRomTab==86) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c89) { Tab->Tabs->Add("TI-89"); if (curRomTab==89) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c92) { Tab->Tabs->Add("TI-92"); if (curRomTab==92) Tab->TabIndex=Tab->Tabs->Count-1; }
    if (c92p) { Tab->Tabs->Add("TI-92+"); if (curRomTab==93) Tab->TabIndex=Tab->Tabs->Count-1; }
    FillList();
}
//---------------------------------------------------------------------

void TEmuModeDlg::FillList()
{
    Calc->Clear();
    if (Tab->TabIndex==-1) return;
    if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-82"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if (romImage[i].type&ROMFLAG_TI82)
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-83"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if ((romImage[i].type&ROMFLAG_TI83)&&
                (!(romImage[i].type&ROMFLAG_FLASHROM)))
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-83+"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if ((romImage[i].type&ROMFLAG_TI83)&&
                (romImage[i].type&ROMFLAG_FLASHROM))
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-85"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if (romImage[i].type&ROMFLAG_TI85)
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-86"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if (romImage[i].type&ROMFLAG_TI86)
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-89"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if (romImage[i].type&ROMFLAG_TI89)
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-92"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if (!(romImage[i].type&(ROMFLAG_TI82|ROMFLAG_TI83|
                ROMFLAG_TI85|ROMFLAG_TI86|ROMFLAG_TI89))&&
                (!(romImage[i].type&ROMFLAG_FLASHROM)))
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
    else if (Tab->Tabs->Strings[Tab->TabIndex]==AnsiString("TI-92+"))
    {
        for (int i=0,j=0;i<romImageCount;i++)
        {
            if (!(romImage[i].type&(ROMFLAG_TI82|ROMFLAG_TI83|
                ROMFLAG_TI85|ROMFLAG_TI86|ROMFLAG_TI89))&&
                (romImage[i].type&ROMFLAG_FLASHROM))
            {
                if (currentROM==i)
                    Calc->ItemIndex=Calc->Items->Add(romImage[i].name);
                else
                    Calc->Items->Add(romImage[i].name);
                romID[j++]=i;
            }
        }
    }
}

void __fastcall TEmuModeDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;
}
//---------------------------------------------------------------------------

void __fastcall TEmuModeDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;    
}
//---------------------------------------------------------------------------

void __fastcall TEmuModeDlg::TabChange(TObject *Sender)
{
    FillList();    
}
//---------------------------------------------------------------------------

