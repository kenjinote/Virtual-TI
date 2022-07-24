//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Value.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TNewValueDlg *NewValueDlg;
//---------------------------------------------------------------------
__fastcall TNewValueDlg::TNewValueDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TNewValueDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TNewValueDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;    
}
//---------------------------------------------------------------------------

