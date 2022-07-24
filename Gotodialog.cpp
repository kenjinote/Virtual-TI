//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "gotodialog.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TGoToDlg *GoToDlg;
//---------------------------------------------------------------------
__fastcall TGoToDlg::TGoToDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TGoToDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;
}
//---------------------------------------------------------------------------

void __fastcall TGoToDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;
}
//---------------------------------------------------------------------------


