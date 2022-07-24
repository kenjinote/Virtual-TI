//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Search.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TSearchDlg *SearchDlg;
//---------------------------------------------------------------------
__fastcall TSearchDlg::TSearchDlg(TComponent* AOwner)
	: TForm(AOwner)
{
    Size->ItemIndex=0;
}
//---------------------------------------------------------------------
void __fastcall TSearchDlg::OKBtnClick(TObject *Sender)
{
    ModalResult=1;    
}
//---------------------------------------------------------------------------

void __fastcall TSearchDlg::CancelBtnClick(TObject *Sender)
{
    ModalResult=2;    
}
//---------------------------------------------------------------------------

