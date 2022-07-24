//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "LinkType.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TLinkTypeDialog *LinkTypeDialog;
//---------------------------------------------------------------------
__fastcall TLinkTypeDialog::TLinkTypeDialog(TComponent* AOwner)
	: TForm(AOwner)
{
    OKBtn->Enabled=false;
    Calc->ItemIndex=-1;
}
//---------------------------------------------------------------------
void __fastcall TLinkTypeDialog::OKBtnClick(TObject *Sender)
{
    if (Calc->ItemIndex==-1)
    {
        ModalResult=0;
        return;
    }
    ModalResult=mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TLinkTypeDialog::CalcChange(TObject *Sender)
{
    OKBtn->Enabled=(Calc->ItemIndex!=-1);
    switch (Calc->ItemIndex)
    {
        case 0: case 1: case 2: case 4: case 5: case 6:
            Time->Caption="Estimated time for dump: 2 minutes";
            break;
        case 3: case 7:
            Time->Caption="Estimated time for dump: 4 minutes";
            break;
        case 8: case 12: case 13: case 14:
            Time->Caption="Estimated time for dump: 30 minutes";
            break;
        case 9: case 10: case 11:
            Time->Caption="Estimated time for dump: 15 minutes";
            break;
        default:
            Time->Caption="Estimated time for dump: Unknown";
            break;
    }
}
//---------------------------------------------------------------------------

