//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "AddLog.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TAddLogDlg *AddLogDlg;
//---------------------------------------------------------------------
__fastcall TAddLogDlg::TAddLogDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------


void __fastcall TAddLogDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TAddLogDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;
}
//---------------------------------------------------------------------------

