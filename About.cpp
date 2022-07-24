//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>

#include "About.h"
#include "rom.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TAboutDlg *AboutDlg;

//---------------------------------------------------------------------
__fastcall TAboutDlg::TAboutDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
extern int intRom92,plusMod,calc;
void __fastcall TAboutDlg::UpdateText()
{
    char str[256];

    sprintf(str,"ROM version: %s",romImage[currentROM].name);
    Calc->Caption=str;
    if (currentSkin==-1)
        Author->Caption="Skin Author: N/A";
    else if (skin[currentSkin].author[0])
        Author->Caption=AnsiString("Skin Author: ")+AnsiString(skin[currentSkin].author);
    else
        Author->Caption="Skin Author: Unknown";
    if (calc<=85)
        RAM->Caption="Emulated RAM: 32k";
    else if ((calc==86)||(calc==92))
        RAM->Caption="Emulated RAM: 128k";
    else
        RAM->Caption="Emulated RAM: 256k";
    if (romImage[currentROM].type&ROMFLAG_UPDATE)
        strcpy(str,"Main ROM Size: 2MB");
    else if (romImage[currentROM].size>=1048576)
        sprintf(str,"Main ROM Size: %dMB",romImage[currentROM].size>>20);
    else
        sprintf(str,"Main ROM Size: %dk",romImage[currentROM].size>>10);
    ROM->Caption=str;
    if (romImage[currentROM].type&ROMFLAG_FLASHROM)
        Flash->Caption="Flash ROM: Yes";
    else
        Flash->Caption="Flash ROM: No";
}

void __fastcall TAboutDlg::FormCreate(TObject *Sender)
{
    UpdateText();
}
//---------------------------------------------------------------------------




